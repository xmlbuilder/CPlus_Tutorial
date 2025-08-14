# 🧵 C++ Thread Pool (Java 스타일 영감) — README

간단하지만 실전에 투입 가능한 C++ Thread Pool 구현입니다.  
Java의 Executor/ThreadPool에서 아이디어를 가져와 **작업 큐 + 워커 스레드 + 안전한 종료 + future 기반 결과 수집**을 제공합니다.

---

## ✨ 특징

- ✅ **고정 크기 워커 스레드** (생성 시 지정)
- ✅ **작업 큐**로 비동기 작업 스케줄링
- ✅ `enqueueJob()` (void 작업) + `submit()` (반환값/예외 수거 `std::future`)
- ✅ **예외 안전**: `submit()`은 예외를 future로 전파
- ✅ **안전한 종료**: `Drain`(드레인) / `Discard`(즉시중단) 정책
- ✅ 데이터 레이스 제거 (`std::atomic<bool>` + 락)
- 🧩 확장 포인트: 우선순위 큐, bounded queue, cancel(token), metrics 등

---

## 📦 인터페이스 요약

```cpp
namespace ThreadPool {

enum class StopMode { Drain, Discard };

class ThreadPool {
public:
  explicit ThreadPool(std::size_t num_threads, StopMode mode = StopMode::Drain);

  // void 작업 등록 (예외는 내부에서 캡처/로깅; 전파되지 않음)
  void enqueueJob(std::function<void()> job);

  // 결과/예외를 future로 수거
  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F&, Args&...>>;

  // 명시적 종료 (옵션). 소멸자에서도 호출됨.
  void stop(StopMode mode = StopMode::Drain);

  ~ThreadPool();
};
} // namespace ThreadPool
```

---

## 🧪 사용 예시

### 1) fire-and-forget (반환값 없음)

```cpp
#include <chrono>
#include <cstdio>
#include <thread>
#include "thread_pool.hpp"

void work(int t, int id) {
  std::printf("%d start\n", id);
  std::this_thread::sleep_for(std::chrono::seconds(t));
  std::printf("%d end after %ds\n", id, t);
}

int main() {
  ThreadPool::ThreadPool pool(3); // 워커 3개
  for (int i = 0; i < 10; ++i)
    pool.enqueueJob([i]{ work(i % 3 + 1, i); });
}
```

### 2) 결과/예외 수거 (`submit` + `future`)

```cpp
#include <iostream>
#include <stdexcept>
#include "thread_pool.hpp"

int main() {
  ThreadPool::ThreadPool pool(4);

  auto f1 = pool.submit([]{ return 21 + 21; });
  auto f2 = pool.submit([]{ throw std::runtime_error("boom"); return 0; });

  std::cout << "sum = " << f1.get() << "\n"; // 42

  try {
    (void)f2.get(); // 예외 재던져짐
  } catch (const std::exception& e) {
    std::cerr << "caught: " << e.what() << "\n";
  }
}
```

---

## 🧱 구현 (단일 헤더 예시)

> 파일명: `thread_pool.hpp`

```cpp
#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace ThreadPool {

enum class StopMode { Drain, Discard };

class ThreadPool {
public:
  explicit ThreadPool(std::size_t num_threads, StopMode mode = StopMode::Drain)
      : stop_mode_(mode) {
    if (num_threads == 0) num_threads = 1;
    workers_.reserve(num_threads);
    for (std::size_t i = 0; i < num_threads; ++i) {
      workers_.emplace_back([this] { workerLoop(); });
    }
  }

  void enqueueJob(std::function<void()> job) {
    if (is_stopping_.load(std::memory_order_acquire))
      throw std::runtime_error("ThreadPool is stopping/stopped");
    {
      std::lock_guard<std::mutex> lk(m_);
      jobs_.emplace(std::move(job));
    }
    cv_.notify_one();
  }

  template <class F, class... Args>
  auto submit(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F&, Args&...>> {
    using R = std::invoke_result_t<F&, Args&...>;
    if (is_stopping_.load(std::memory_order_acquire))
      throw std::runtime_error("ThreadPool is stopping/stopped");

    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<R> fut = task->get_future();

    {
      std::lock_guard<std::mutex> lk(m_);
      jobs_.emplace([task]{
        try { (*task)(); }
        catch (...) { /* packaged_task가 future로 set_exception */ }
      });
    }
    cv_.notify_one();
    return fut;
  }

  void stop(StopMode mode = StopMode::Drain) {
    {
      std::lock_guard<std::mutex> lk(m_);
      stop_mode_ = mode;
      is_stopping_.store(true, std::memory_order_release);
      if (stop_mode_ == StopMode::Discard) {
        while (!jobs_.empty()) jobs_.pop();
      }
    }
    cv_.notify_all();
    for (auto& t : workers_) if (t.joinable()) t.join();
    workers_.clear();
  }

  ~ThreadPool() { stop(stop_mode_); }

private:
  void workerLoop() {
    for (;;) {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this]{
          return is_stopping_.load(std::memory_order_acquire) || !jobs_.empty();
        });

        if (is_stopping_.load(std::memory_order_acquire)) {
          if (stop_mode_ == StopMode::Discard) return;
          if (jobs_.empty()) return; // Drain: 남은 작업 없으면 종료
        }

        job = std::move(jobs_.front());
        jobs_.pop();
      }
      // 예외는 submit 경로에선 future로 전파됨. enqueueJob 경로는 여기서 삼킨다.
      try { job(); }
      catch (...) { /* TODO: 로깅/계측 훅 */ }
    }
  }

private:
  std::mutex m_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> jobs_;
  std::vector<std::thread> workers_;
  std::atomic<bool> is_stopping_{false};
  StopMode stop_mode_{StopMode::Drain};
};

} // namespace ThreadPool
```

---

## 🧭 설계 노트

- **Drain vs Discard**: 종료 정책 선택 가능 (운영 시 매우 유용)
- **예외 처리**: `submit()` 경로는 `packaged_task`가 자동으로 `set_exception` → `future.get()`에서 재던짐
- **스푸리어스 웨이크** 대응: `wait`의 조건자 사용
- **데이터 레이스 제거**: 종료 플래그는 `atomic` + 락 경계 내 접근

---

## 🔌 확장 아이디어

- **Bounded Queue + Backpressure**: 큐가 가득 차면 대기/타임아웃/거부
- **취소 지원(C++20)**: `std::stop_token` 전달 및 협력적 취소
- **메트릭**: 큐 길이/대기시간/작업시간/실패 건수 기록
- **우선순위 큐**: 레이턴시 민감 작업을 빠르게 실행
- **스레드 이름/핀닝**: 디버깅 및 성능 최적화

---

## ✅ 테스트 체크리스트

- 다중 프로듀서/컨슈머 동시 제출
- 종료 중 경쟁: `stop(Drain/Discard)` + 제출 경합
- 예외 던지는 작업 다량 제출
- 대량 제출/중단 반복 시 리소스 누수 검사(ASan/Valgrind)
- 처리량·지연 측정(프로파일링)
---





