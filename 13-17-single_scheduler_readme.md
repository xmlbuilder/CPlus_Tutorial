# ⏰ Single‑File C++ Scheduler (every / in / interval / cron / at)

간단한 **단일 파일 스케줄러**입니다. 외부 의존성 없이 `every`, `in`, `interval`, `cron`, `at` 를 지원하며,
내부에 경량 **ThreadPool**을 포함합니다. 프로젝트에 파일 하나만 넣고 빌드하면 바로 사용할 수 있습니다.

---

## ✨ 특징
- `every(duration, fn, args...)` : 주기적으로 실행 (작업 **시작 간격** 보장)
- `in(duration|time_point, fn, args...)` : 지정 **지연/시각**에 1회 실행
- `interval(duration, fn, args...)` : **작업 종료 후** 간격을 기다렸다가 재실행 (중복 실행 방지)
- `cron("m h dom mon dow", fn)` : 5필드 cron 표현식 지원 (`*` 또는 정수)
- `at("YYYY-mm-dd HH:MM:SS" | "YYYY/mm/dd HH:MM:SS" | "HH:MM:SS")` : 문자열 시각
  - 시간만 주면 이미 지난 경우 **내일** 실행, 날짜 포함이면 이미 지났어도 **즉시** 실행
- 내부 경량 **ThreadPool** (고정 워커, 작업 예외 격리)

---

## 🔧 빌드
```bash
g++ -std=c++17 -O2 single_scheduler.cpp -lpthread -o scheduler_demo
./scheduler_demo
```

---

## 📦 단일 파일 소스 (`single_scheduler.cpp`)

```cpp
// single_scheduler.cpp
// Minimal single-file scheduler: every / in / interval / cron / at
// - 내부 경량 ThreadPool 포함
// - Cron 5필드 지원: m h dom mon dow   (* 또는 정수)
// - "at" 포맷: "%Y-%m-%d %H:%M:%S", "%Y/%m/%d %H:%M:%S", "%H:%M:%S"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace mini {

// ---------- Interruptible sleep ----------
class InterruptibleSleep {
public:
  void sleep() {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [&]{ return interrupted_.exchange(false); });
  }
  void sleep_until(std::chrono::system_clock::time_point tp) {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait_until(lk, tp, [&]{ return interrupted_.exchange(false); });
  }
  void interrupt() {
    {
      std::lock_guard<std::mutex> lk(m_);
      interrupted_.store(true);
    }
    cv_.notify_all();
  }
private:
  std::mutex m_;
  std::condition_variable cv_;
  std::atomic<bool> interrupted_{false};
}

// ---------- Tiny ThreadPool (fixed size) ----------
class ThreadPool {
public:
  explicit ThreadPool(std::size_t n) { start(n ? n : 1); }
  ~ThreadPool() { stop(); }

  void enqueue(std::function<void()> f) {
    {
      std::lock_guard<std::mutex> lk(m_);
      if (stopping_) throw std::runtime_error("ThreadPool stopping");
      q_.push(std::move(f));
    }
    cv_.notify_one();
  }

  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F&, Args&...>> {
    using R = std::invoke_result_t<F&, Args&...>;
    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto fut = task->get_future();
    enqueue([task]{ (*task)(); });
    return fut;
  }

private:
  void start(std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
      workers_.emplace_back([this]{
        for (;;) {
          std::function<void()> job;
          {
            std::unique_lock<std::mutex> lk(m_);
            cv_.wait(lk, [&]{ return stopping_ || !q_.empty(); });
            if (stopping_ && q_.empty()) return;
            job = std::move(q_.front()); q_.pop();
          }
          try { job(); } catch (...) { /* swallow or log */ }
        }
      });
    }
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lk(m_);
      stopping_ = true;
    }
    cv_.notify_all();
    for (auto& t : workers_) if (t.joinable()) t.join();
  }

  std::mutex m_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> q_;
  std::vector<std::thread> workers_;
  bool stopping_ = false;
};

// ---------- Cron (5 fields) ----------
class BadCron : public std::runtime_error {
public:
  explicit BadCron(const std::string& m) : std::runtime_error(m) {}
};

inline void add_tm(std::tm& tm, std::chrono::system_clock::duration d) {
  using Clock = std::chrono::system_clock;
  auto tp = Clock::from_time_t(std::mktime(&tm));
  auto tp2 = tp + d;
  auto tt = Clock::to_time_t(tp2);
  tm = *std::localtime(&tt);
}

struct Cron {
  // minute hour day month dow
  int minute = -1, hour = -1, day = -1, month = -1, dow = -1;

  explicit Cron(const std::string& expr) {
    std::istringstream iss(expr);
    std::vector<std::string> tok; std::string t;
    while (iss >> t) tok.push_back(t);
    if (tok.size() != 5) throw BadCron("cron must be 5 fields: " + expr);
    parse(tok[0], 0, 59, minute);
    parse(tok[1], 0, 23, hour);
    parse(tok[2], 1, 31, day);
    parse(tok[3], 1, 12, month, true);     // tm_mon: 0~11
    parse(tok[4], 0, 6,  dow);
  }

  static void parse(const std::string& s, int lo, int hi, int& out, bool adjust=false) {
    if (s == "*") { out = -1; return; }
    int v{};
    try { v = std::stoi(s); } catch (...) { throw BadCron("bad token: " + s); }
    if (v < lo || v > hi) throw BadCron("out of range: " + s);
    out = adjust ? (v-1) : v;
  }

  std::chrono::system_clock::time_point next(std::chrono::system_clock::time_point from
                                             = std::chrono::system_clock::now()) const {
    using Clock = std::chrono::system_clock;
    auto tt = Clock::to_time_t(from);
    std::tm next_tm = *std::localtime(&tt);
    next_tm.tm_sec = 0;
    add_tm(next_tm, std::chrono::minutes(1)); // 최소 1분 뒤

    for (;;) {
      if (month != -1 && next_tm.tm_mon != month) {
        if (next_tm.tm_mon + 1 > 11) { next_tm.tm_mon = 0; next_tm.tm_year++; }
        else next_tm.tm_mon++;
        next_tm.tm_mday = 1; next_tm.tm_hour = 0; next_tm.tm_min = 0; continue;
      }
      if (day != -1 && next_tm.tm_mday != day) { add_tm(next_tm, std::chrono::hours(24)); next_tm.tm_hour = 0; next_tm.tm_min = 0; continue; }
      if (dow != -1 && next_tm.tm_wday != dow) { add_tm(next_tm, std::chrono::hours(24)); next_tm.tm_hour = 0; next_tm.tm_min = 0; continue; }
      if (hour != -1 && next_tm.tm_hour != hour) { add_tm(next_tm, std::chrono::hours(1)); next_tm.tm_min = 0; continue; }
      if (minute != -1 && next_tm.tm_min != minute) { add_tm(next_tm, std::chrono::minutes(1)); continue; }
      break;
    }
    next_tm.tm_isdst = -1;
    return Clock::from_time_t(std::mktime(&next_tm));
  }
};

// ---------- Scheduler ----------
class Scheduler {
public:
  using Clock = std::chrono::system_clock;

  explicit Scheduler(unsigned max_parallel = 4)
      : done_(false), pool_(max_parallel ? max_parallel : 1) {
    // 타이밍 관리 스레드 1개
    mgr_ = std::thread([this]{
      while (!done_.load()) {
        if (tasks_.empty()) {
          sleeper_.sleep();
        } else {
          auto next_tp = tasks_.begin()->first;
          sleeper_.sleep_until(next_tp);
        }
        manage();
      }
    });
  }

  ~Scheduler() {
    done_.store(true);
    sleeper_.interrupt();
    if (mgr_.joinable()) mgr_.join();
  }

  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  // in: 특정 시간/지연 뒤 1회 실행
  template <class F, class... Args>
  void in(Clock::time_point tp, F&& f, Args&&... args) {
    add(tp, std::make_shared<InTask>(bind(std::forward<F>(f), std::forward<Args>(args)...)));
  }
  template <class F, class... Args>
  void in(Clock::duration d, F&& f, Args&&... args) {
    in(Clock::now() + d, std::forward<F>(f), std::forward<Args>(args)...);
  }

  // every: 주기적으로 실행(다음 실행 시각을 미리 스케줄)
  template <class F, class... Args>
  void every(Clock::duration d, F&& f, Args&&... args) {
    auto t = std::make_shared<EveryTask>(d, bind(std::forward<F>(f), std::forward<Args>(args)...), false);
    add(t->next_time(), t);
  }

  // interval: 작업이 끝난 후 간격을 기다린 뒤 다시 실행(중복 실행 안 됨)
  template <class F, class... Args>
  void interval(Clock::duration d, F&& f, Args&&... args) {
    auto t = std::make_shared<EveryTask>(d, bind(std::forward<F>(f), std::forward<Args>(args)...), true);
    add(Clock::now(), t);
  }

  // cron: 5필드 cron 표현
  template <class F, class... Args>
  void cron(const std::string& expr, F&& f, Args&&... args) {
    auto t = std::make_shared<CronTask>(expr, bind(std::forward<F>(f), std::forward<Args>(args)...));
    add(t->next_time(), t);
  }

  // at: 문자열 시각 (지나간 시간: 날짜 없으면 내일, 날짜 있으면 즉시)
  template <class F, class... Args>
  void at(const std::string& when, F&& f, Args&&... args) {
    auto tp = parse_time(when);
    in(tp, std::forward<F>(f), std::forward<Args>(args)...);
  }

private:
  using Fn = std::function<void()>;

  struct Task {
    explicit Task(Fn g, bool recur=false, bool interval=false)
        : f(std::move(g)), recur(recur), interval(interval) {}
    virtual ~Task() = default;
    virtual Clock::time_point next_time() const = 0;
    Fn f; bool recur; bool interval;
  };

  struct InTask : Task {
    explicit InTask(Fn g) : Task(std::move(g)) {}
    Clock::time_point next_time() const override { return Clock::time_point{}; } // unused
  };

  struct EveryTask : Task {
    EveryTask(Clock::duration d, Fn g, bool interval)
        : Task(std::move(g), true, interval), dur(d) {}
    Clock::time_point next_time() const override { return Clock::now() + dur; }
    Clock::duration dur;
  };

  struct CronTask : Task {
    CronTask(const std::string& e, Fn g) : Task(std::move(g), true, false), cron(e) {}
    Clock::time_point next_time() const override { return cron.next(); }
    Cron cron;
  };

  template <class F, class... Args>
  static Fn bind(F&& f, Args&&... args) {
    return std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  void add(Clock::time_point tp, std::shared_ptr<Task> t) {
    {
      std::lock_guard<std::mutex> lk(mx_);
      tasks_.emplace(tp, std::move(t));
    }
    sleeper_.interrupt();
  }

  static Clock::time_point parse_time(const std::string& s) {
    using Clock = std::chrono::system_clock;
    auto now_tt = Clock::to_time_t(Clock::now());
    std::tm tm = *std::localtime(&now_tt);

    auto try_fmt = [&](const char* fmt)->bool{
      std::istringstream iss(s);
      iss >> std::get_time(&tm, fmt);
      return !iss.fail();
    };

    if (try_fmt("%H:%M:%S")) {
      auto tp = Clock::from_time_t(std::mktime(&tm));
      if (Clock::now() >= tp) tp += std::chrono::hours(24);
      return tp;
    }
    if (try_fmt("%Y-%m-%d %H:%M:%S")) return Clock::from_time_t(std::mktime(&tm));
    if (try_fmt("%Y/%m/%d %H:%M:%S")) return Clock::from_time_t(std::mktime(&tm));
    throw std::runtime_error("Cannot parse time string: " + s);
  }

  void manage() {
    std::multimap<Clock::time_point, std::shared_ptr<Task>> recur;
    {
      std::lock_guard<std::mutex> lk(mx_);
      auto end = tasks_.upper_bound(Clock::now());
      if (end == tasks_.begin()) return;

      for (auto it = tasks_.begin(); it != end; ++it) {
        auto& t = it->second;
        if (t->interval) {
          auto task_ptr = t; // capture
          pool_.enqueue([this, task_ptr]{
            task_ptr->f();
            add(task_ptr->next_time(), task_ptr);
          });
        } else {
          pool_.enqueue([task=t]{ task->f(); });
          if (t->recur) recur.emplace(t->next_time(), t);
        }
      }
      tasks_.erase(tasks_.begin(), end);
    }
    for (auto& kv : recur) add(kv.first, kv.second);
  }

  std::atomic<bool> done_;
  ThreadPool pool_;
  std::thread mgr_;
  InterruptibleSleep sleeper_;
  std::mutex mx_;
  std::multimap<Clock::time_point, std::shared_ptr<Task>> tasks_;
};

} // namespace mini

// ---------------------- Demo ----------------------
void message(const std::string& s) { std::cout << s << std::endl; }

int main() {
  using namespace std::chrono;

  unsigned max_n_threads = 12;
  mini::Scheduler s(max_n_threads);

  s.every(seconds(1), message, "every second");

  s.in(minutes(1), []{ std::cout << "in one minute" << std::endl; });

  s.interval(seconds(1), []{
    std::cout << "right away, then ~every 6s (5s work + 1s interval)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
  });

  s.cron("* * * * *", []{ std::cout << "top of every minute" << std::endl; });

  s.at("2099-12-31 23:59:55", []{ std::cout << "at a specific time." << std::endl; });

  s.cron("5 0 * * *", []{ std::cout << "every day 5 minutes after midnight" << std::endl; });

  std::this_thread::sleep_for(std::chrono::minutes(10));
}
```

---

## 🧭 API 요약

| 메서드 | 의미 |
|---|---|
| `every(duration, f, args...)` | 지정 주기마다 **새 실행 스케줄** (작업 시간이 길어도 시작 간격 고정) |
| `interval(duration, f, args...)` | 작업 **종료 후** 간격을 기다렸다가 재실행 (중복 실행 방지) |
| `in(duration|time_point, f, args...)` | 일정 지연/시각에 **1회 실행** |
| `cron("m h dom mon dow", f)` | cron 표현식으로 반복 실행 |
| `at("YYYY-mm-dd HH:MM:SS" \| "YYYY/mm/dd HH:MM:SS" \| "HH:MM:SS")` | 문자열 시각 실행 |

> `cron` 필드 순서: 분(0–59) 시(0–23) 일(1–31) 월(1–12) 요일(0–6, 일요일=0). `*` 허용.

---

## 📌 주의사항
- 본 구현은 **단일 파일** 데모이며, 운영 환경에서는 **Bounded Queue, 취소 토큰, 메트릭** 등을 추가하는 것을 권장합니다.
- 타임존/서머타임(DST) 변경 시각에는 표준 `tm/mktime` 동작에 따릅니다.

---

행복한 스케줄링! ⏱️
