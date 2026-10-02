## 📘 Mutex & `osyncstream` 예제 

- 간단한 예제로 **뮤텍스(`std::mutex`)** 와 **동기화 출력(`std::osyncstream`)** 을 사용해  
멀티스레드 환경에서 공유 변수(`balance`)를 안전하게 수정하는 방법을 보여줍니다.


### 📌 핵심 아이디어
- `balance`는 **공유 자원** 이므로 동시에 접근하면 **race condition** 이 됩니다.
- 임계 구역을 `std::mutex`로 보호하여 **동시에 하나의 스레드만** 수정하도록 보장합니다.
- 출력은 시작 로그만 `std::osyncstream`으로 감싸 **출력 꼬임(interleaving)** 을 최소화합니다.


### 📌 예제 코드 (RAII 적용판)

- 수동 `lock()/unlock()` 대신 **RAII** (`std::lock_guard`)를 사용해서 예외 상황에서도 자동 해제가 보장됩니다.  


```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <syncstream>

std::mutex m;
int balance = 100;

void ThreadFunc() {
    // osyncstream: 이 줄의 출력은 한 덩어리로 안전하게 출력
    std::osyncstream(std::cout) << "Model Test Started" << std::endl;

    // 임계구역: 공유 데이터 수정은 뮤텍스로 보호
    std::lock_guard<std::mutex> lk(m);
    std::cout << std::this_thread::get_id() << std::endl;
    std::cout << "Thread Started" << std::endl;
    balance -= 80; // 공유 데이터 수정
    std::cout << "Thread Ended" << std::endl;

    // lk 소멸 시 자동 unlock
    std::cout << "Model Test Ended" << std::endl;
}

int main() {
    std::thread t1(ThreadFunc);
    std::thread t2(ThreadFunc);
    t1.join();
    t2.join();

    std::cout << balance << std::endl; // 100 - 80 - 80 = -60
    return 0;
}
```

### 📌 빌드
```bash
g++ -std=c++20 -O2 -pthread mutex_demo.cpp -o mutex_demo
./mutex_demo
```

> `std::osyncstream`은 C++20 기능입니다. (GCC 10+/Clang 12+/MSVC 19.28+ 권장)

---

### 📌 예상 출력 (예)
- 스레드 ID와 줄 순서는 환경에 따라 다를 수 있지만, **최종 `balance`는 항상 -60** 이어야 합니다.

```
Model Test Started
Model Test Started
35356
Thread Started
Thread Ended
Model Test Ended
36020
Thread Started
Thread Ended
Model Test Ended
-60
```

#### 🔹 왜 -60인가요?  
- 초기값 100에서 두 스레드가 각각 80씩 차감: `100 - 80 - 80 = -60`.  
- 뮤텍스가 임계구역을 직렬화하므로 순서는 달라도 **결과는 결정적** 입니다.

---

### 📌 흔한 실수와 개선점
- **수동 `lock()`/`unlock()`**: 예외 발생 시 `unlock()`이 건너뛰어 **데드락** 위험 → `std::lock_guard`/`std::unique_lock` 사용 권장.
- **출력 꼬임**: 여러 스레드가 동시에 `std::cout` 사용 시 줄이 섞일 수 있음 → `std::osyncstream` 또는 출력도 뮤텍스로 보호.



### 📌 참고: 대안 설계
- 단순 카운터/합산처럼 **원자 연산** 만 필요하면 `std::atomic<int>` + `fetch_sub(80)`로도 가능.  
  복합 연산(다중 변수, 조건부 갱신 등)은 **뮤텍스가 더 적합** 합니다.

---

