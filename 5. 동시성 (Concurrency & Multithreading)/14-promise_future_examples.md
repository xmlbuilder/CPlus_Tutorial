## 📘 `std::promise` & `std::future` — 스레드 결과 반환

### 📌 개요
- C++11에서는 `<future>` 헤더에 **`std::promise`** 와 **`std::future`** 가 추가되어  
멀티스레드 환경에서 **스레드 실행 결과를 안전하게 반환** 받을 수 있습니다.

- **`std::promise<T>`** : 값을 설정(set)하는 역할
- **`std::future<T>`** : 값을 가져오는(get) 역할
- `promise` → `get_future()` → `future`
- `future.get()` 은 결과값을 얻을 때까지 블록됨


### 📌 기본 예제 — `promise`와 `future`

```cpp
#include <iostream>
#include <thread>
#include <future>

void ThreadFunc(std::promise<int>& retVal) {
    retVal.set_value(1);
}

int main() {
    std::promise<int> p;               // 반환 타입이 int인 promise 생성
    auto f = p.get_future();            // future 생성

    std::thread th1(ThreadFunc, std::ref(p));
    th1.join();

    std::cout << f.get() << std::endl;  // 스레드 반환값 출력 (1)
}
```

#### 🔹 동작 설명
- 1. `promise<int>` → 반환값 타입 `int`
- 2. `get_future()` 로 `future<int>` 생성
- 3. 스레드에서 `set_value()` 호출 → 값 전달
- 4. `future.get()` 호출 → 값 수신 (스레드 종료 후)

### 📌 `std::async`를 이용한 간단한 구현

```cpp
#include <thread>
#include <future>
#include <iostream>
#include <chrono>

int ThreadFunc() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 1;
}

int main() {
    std::future<int> f = std::async(ThreadFunc);

    // get() 호출 전까지 다른 코드 실행 가능

    std::cout << f.get() << std::endl; // 3초 대기 후 1 출력
}
```

- `std::async`는 `thread + promise/future`를 합친 고수준 API
- `get()` 호출 시 결과가 준비될 때까지 대기


### 📌 `promise<void>` — 완료 여부 플래그로 사용

```cpp
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void worker(std::promise<void>* p) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    p->set_value(); // 완료 신호
}

int main() {
    std::promise<void> p;
    std::future<void> data = p.get_future();

    std::thread t(worker, &p);

    while (true) {
        std::future_status status = data.wait_for(std::chrono::seconds(1));
        if (status == std::future_status::timeout) {
            std::cerr << ">"; // 대기 중 표시
        } else if (status == std::future_status::ready) {
            break; // 완료
        }
    }

    t.join();
}
```

- `promise<void>`는 값을 전달하지 않고 **작업 완료 여부** 만 전달
- `wait_for()`를 사용해 **폴링(polling) 방식** 대기 가능


### 📌 값 반환 예제 (`promise<int>`)

```cpp
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void worker(std::promise<int>* p) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    p->set_value(10); // 결과 값 전달
}

int main() {
    std::promise<int> p;
    std::future<int> data = p.get_future();

    std::thread t(worker, &p);

    while (true) {
        std::future_status status = data.wait_for(std::chrono::seconds(1));
        if (status == std::future_status::timeout) {
            std::cerr << ">"; // 대기 표시
        } else if (status == std::future_status::ready) {
            break;
        }
    }

    t.join();
    std::cout << data.get() << std::endl; // 10 출력
}
```

### 📌 핵심 요약

| 요소 | 역할 |
|------|------|
| `std::promise<T>` | 값을 저장(set_value)하는 쪽 |
| `std::future<T>`  | 값을 읽는(get) 쪽 |
| `get_future()`    | promise에서 future 생성 |
| `set_value()`     | 값을 전달 |
| `get()`           | 값을 가져옴 (대기 포함) |
| `wait_for()`      | 특정 시간만 대기 후 상태 반환 |
| `std::async()`    | thread + promise/future 통합형 |



### 📌 동작 흐름 다이어그램

```md
Promise/Producer                        Future/Consumer
----------------                        ----------------
    │  create promise<T>                      │
    │───────────────────────────────────────▶│ get_future()
    │                                         │
    │  set_value(value)                       │
    │───────────────────────────────────────▶│ get() → waits until ready
    │                                         │
    │ (thread finishes)                       │
```
---
