## 📘 동기화 확장 — Mutual Exclusion & Condition Synchronization

### 📌 개요
- 멀티프로세스 또는 멀티스레드 환경에서 공유 자원을 사용할 때 **race condition(경쟁 상태)** 이 발생할 수 있습니다.  
- 이를 해결하기 위해 **동기화(synchronization)** 가 필요합니다.

- 동기화 전략은 크게 두 가지로 나뉩니다:
   - 1. **상호 배제 (Mutual Exclusion)**  
      - 동시에 하나의 스레드만 공유 자원에 접근 가능하게 함
      - `std::mutex`, `std::lock_guard`, `std::unique_lock` 등을 사용
   - 2. **조건 동기화 (Condition Synchronization)**  
      - 특정 조건이 충족될 때까지 스레드를 대기시키고, 조건이 충족되면 실행
      - `std::condition_variable`을 사용


### 📌 예제 — `std::condition_variable` 확장판

- 이 예제는 **생산자(Producer)** 와 **소비자(Consumer)** 패턴을 사용하여  
조건 변수(`std::condition_variable`)로 스레드 간 동기화를 구현한 코드입니다.

```cpp
#include <queue>
#include <chrono>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <atomic>

void conditional_variable_test1() {
    std::queue<int> produced_names;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> notified = {false};

    auto producer = [&]() {
        for (int i = 0; ; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::unique_lock<std::mutex> lock(mtx);
            std::cout << "producing " << i << std::endl;
            produced_names.push(i);
            notified = true;
            cv.notify_all(); // 모든 대기 스레드 깨움
            lock.unlock();
        }
    };

    auto consumer = [&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);

            while (!notified) { // 조건이 충족될 때까지 대기
                cv.wait(lock);
            }

            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            lock.lock();
            if (!produced_names.empty()) {
                std::cout << "consuming " << produced_names.front() << std::endl;
                produced_names.pop();
            }
            notified = false;
            lock.unlock();
        }
    };

    std::thread p(producer);
    std::thread cs[2];
    for (int i = 0; i < 2; ++i) {
        cs[i] = std::thread(consumer);
    }
    p.join();
    for (int i = 0; i < 2; ++i) {
        cs[i].join();
    }
}

int main() {
    conditional_variable_test1();
}
```

---

#### 🔹 실행 예시
```
producing 0
producing 1
consuming 0
consuming 1
producing 2
producing 3
consuming 2
consuming 3
producing 4
producing 5
consuming 4
consuming 5
producing 6
producing 7
producing 8
consuming 6
consuming 7
producing 9
producing 10
producing 11
...
```

### 📌 동작 원리
- **생산자 스레드**
  - 일정 시간 간격으로 새로운 데이터를 생성하여 큐(`produced_names`)에 삽입
  - `cv.notify_all()`로 대기 중인 모든 소비자 스레드를 깨움
- **소비자 스레드**
  - `notified` 플래그가 `true`가 될 때까지 `cv.wait(lock)`으로 대기
  - 알림을 받으면 큐에서 데이터를 꺼내 처리
  - 처리 후 `notified = false`로 상태 초기화

### 📌 핵심 요약

| 동기화 방식       | 설명 | 사용 예시 |
|-------------------|------|----------|
| 상호 배제         | 공유 자원에 동시 접근 방지 | `std::mutex`, `lock_guard` |
| 조건 동기화       | 특정 조건 충족 시 스레드 실행 | `std::condition_variable` |
| notify_one()      | 대기 중인 스레드 하나 깨움 | 소비자 1명 깨우기 |
| notify_all()      | 모든 대기 스레드 깨움 | 소비자 여러 명 깨우기 |


### 📌 동작 흐름 다이어그램
- Producer - Consumer with Condition Variable
```md
Producer                       Condition Variable                   Consumer(s)
---------                      -------------------                  ------------
    │                                   │                                 │
    │ produce item                      │                                 │
    ├─────────────────────────────────▶│                                 │
    │                                   │                                 │
    │          notify_all()             │                                 │
    ├─────────────────────────────────▶│                                 │
    │                                   │           wait()                │
    │                                   ├───────────────────────────────▶│
    │                                   │                                 │
    │                                   │        consume item             │
    │                                   │◀───────────────────────────────┤
    │                                   │                                 │
    │ (loop)                            │             (loop)              │
```
---
