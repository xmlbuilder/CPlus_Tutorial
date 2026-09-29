## 📘 Thread Basics (C++11~)

- 스레드는 여러 작업을 동시에 실행하여 **응답성** 과 **성능** 을 높이기 위한 기본 도구입니다.  
- C++11부터 `<thread>` 표준 라이브러리로 포터블하게 스레드를 다룰 수 있습니다.



### 📌 1) 최소 예제: 생성(spawn) + 합류(join)

```cpp
#include <iostream>
#include <thread>

int main() {
    std::thread t([]{
        for (int i = 0; i < 5; ++i) {
            std::cout << "Worker: " << i << "\n";
        }
    });

    for (int i = 0; i < 3; ++i) {
        std::cout << "Main  : " << i << "\n";
    }

    t.join(); // 스레드 종료 대기 (필수)
}
```
- `join()` 또는 `detach()`를 **반드시** 호출해야 합니다. 안 하면 `std::terminate()` 발생.

---

### 📌 2) 캡처와 수명

- 지역 변수를 스레드에 넘길 땐 **값 캡처** 또는 **값 전달** 이 안전합니다.

```cpp
void thread_test1() {
    int a = 10;
    std::thread t([value = a] {        // 값 캡처
        std::cout << "hello world.\n";
        std::cout << "a value : " << value << "\n";
    });
    t.join();
}
```

> 참조 캡처(`[&]`)는 스레드가 더 오래 살면 **댕글링 참조** 위험이 있으므로 지양.


### 📌 3) 인자 전달(복사/이동/참조)

```cpp
#include <thread>
#include <string>
#include <utility>
#include <iostream>

void work(std::string s) { std::cout << s << "\n"; }

int main() {
    std::string msg = "hello";
    std::thread t1(work, msg);                 // 복사
    std::thread t2(work, std::move(msg));      // 이동 (msg는 이후 비어질 수 있음)

    auto append_excl = [](std::string& s){ s += "!"; };
    std::string s = "hi";
    std::thread t3(append_excl, std::ref(s));  // 참조 전달은 std::ref 로 명시
    t1.join(); t2.join(); t3.join();
}
```

---

### 📌 4) `detach()`는 신중히

```cpp
std::thread t([]{ /* 백그라운드 작업 */ });
t.detach(); // 메인과 분리
```
- 종료 시점/자원 관리가 어려워 **일반적으로 권장되지 않음**. 가능하면 `join()` 사용.



### 📌 5) RAII: Thread Guard

```cpp
#include <thread>

struct thread_guard {
    std::thread t;
    explicit thread_guard(std::thread&& th) : t(std::move(th)) {}
    ~thread_guard() { if (t.joinable()) t.join(); } // 스코프 종료 시 자동 join
    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

int main(){
    thread_guard g(std::thread([]{ /* 작업 */ }));
}
```

-

### 📌 6) 공유 데이터 동기화: `std::mutex`

```cpp
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

int main() {
    std::mutex m;
    int counter = 0;
    std::vector<std::thread> ts;

    for (int i = 0; i < 4; ++i) {
        ts.emplace_back([&] {
            for (int k = 0; k < 10000; ++k) {
                std::lock_guard<std::mutex> lk(m);
                ++counter; // 임계 구역
            }
        });
    }
    for (auto& t : ts) t.join();
    std::cout << counter << "\n"; // 40000
}
```
- 공유 데이터 변경 시 **반드시** 적절한 동기화(뮤텍스, 원자 변수 등)를 사용.

---

### 📌 7) 유틸리티

```cpp
auto n = std::thread::hardware_concurrency(); // 사용 가능한 CPU 힌트(0일 수도)
std::this_thread::sleep_for(std::chrono::milliseconds(10));
std::this_thread::yield(); // 스케줄러에 양보
```

---

### 📌 8) 흔한 실수 체크리스트

- `join()`/`detach()` 미호출 → **terminate**
- 로컬 변수 참조 캡처 → 수명 문제
- 데이터 레이스(동기화 누락)
- `detach()` 남용으로 종료/자원 관리 불가
- 스레드 내부 예외 미처리 → `std::terminate()`

---

