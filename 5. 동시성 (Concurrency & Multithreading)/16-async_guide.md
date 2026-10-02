## 📘`std::async` — 비동기 실행과 결과 수집

### 📌 개요
- `std::async`는 C++11부터 도입된 **비동기 함수 실행 도구** 로,  
내부적으로 `std::thread` + `std::promise` + `std::future`를 결합한 고수준 API입니다.

- 반환값은 **`std::future<T>`** 로 받습니다.
- **Thread Pool** 또는 구현체 내부 스케줄러를 활용할 수 있습니다.
- 예외/값 반환을 안전하게 처리합니다.
- 실행 정책(`std::launch`)을 통해 즉시 실행할지, 지연 실행할지 결정할 수 있습니다.

### 📌 기본 사용 예제

```cpp
#include <iostream>
#include <future>

void for_print(char c) {
    for (int i = 0; i < 5; i++)
        printf("%c - Thread : %d\n", c, i);
}

int main() {
    std::future<void> a = std::async(std::launch::async, for_print, 'a');     // 즉시 실행
    std::future<void> b = std::async(std::launch::deferred, for_print, 'b'); // get()/wait() 시 실행
    std::future<void> c = std::async(for_print, 'c');                        // 정책 생략 (구현체 결정)

    b.get(); // b 실행
}
```

#### 🔹 출력 예시
```
b - Thread : 0
...
a - Thread : 4
```


### 📌 실행 정책 (`std::launch`)
| 정책 | 설명 |
|------|------|
| `std::launch::async` | 즉시 새로운 스레드에서 실행 |
| `std::launch::deferred` | 지연 실행 — `get()` 또는 `wait()` 호출 시 실행 (호출 스레드에서 실행) |
| 생략 | 구현체가 async 또는 deferred 중 선택 |



### 📌 예외 처리

```cpp
#include <iostream>
#include <future>

void for_print(char c) {
    for (int i = 0; i < 10; i++)
        printf("%c - Thread : %d\n", c, i);
}

int main() {
    auto c = std::async(for_print, 'c');
    std::future<int> d; // 비어있는 future

    try {
        c.get(); // 정상
        d.get(); // 예외 발생 (no state)
    } catch (const std::future_error& e) {
        std::cout << "Caught a future_error with code \"" << e.code()
                  << "\"\nMessage: \"" << e.what() << "\"\n";
    }
}
```

#### 🔹 출력:
```
Caught a future_error with code "future:4"
Message: "no state"
```

---

### 📌 `wait_for()`로 상태 확인

```cpp
#include <iostream>
#include <future>
#include <chrono>

const int number = 444444443;
bool is_prime(int x) {
    for (int i = 2; i < x; ++i) if (x % i == 0) return false;
    return true;
}

int main() {
    std::future<bool> fut = std::async(is_prime, number);
    std::cout << "checking, please wait";

    std::chrono::milliseconds span(100);
    while (fut.wait_for(span) != std::future_status::ready) {
        std::cout << '.'; std::cout.flush();
    }

    bool x = fut.get();
    std::cout << "\n" << number << " " << (x ? "is" : "is not") << " prime.\n";
}
```

---

### 📌 병렬 합계 계산 예제

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <algorithm>

template <typename It>
int parallel_sum(It start, It end) {
    int len = end - start;
    if (len <= 1) return 0;

    It mid = start + 200;
    auto handle = std::async(parallel_sum<It>, mid, end);

    int sum = 0;
    std::for_each(start, mid, [&](int n) { sum += n; });

    return sum + handle.get();
}

int main() {
    std::vector<int> v(1000);
    std::iota(v.begin(), v.end(), 1); // 1~1000

    std::cout << "sum : " << parallel_sum(v.begin(), v.end()) << "\n";
}
```

- 데이터를 200개 단위로 분할해 여러 스레드에서 병렬 합계 계산
- `handle.get()`으로 부분합을 합산

---

### 📌 핵심 요약
- `std::async`는 `thread`보다 안전하고 간편한 비동기 실행 도구
- `future.get()`로 값/예외를 수거 (1회만 호출 가능)
- 실행 정책으로 즉시/지연 실행 제어 가능
- `wait_for()`/`wait_until()`로 비차단 상태 확인 가능
- 병렬 알고리즘 구현 시 재귀적으로 사용할 수 있음


---

