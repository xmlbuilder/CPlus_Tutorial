## 📘 Lambda Closure와 이동 캡처

### 📌 핵심 개념
- C++ Lambda는 **함수와 캡처한 상태를 하나의 객체(Closure)** 로 보관할 수 있다.
- `[data = std::move(data)]`는 **초기화 캡처(init-capture)** 문법으로, 지역 변수 `data`의 `std::vector`를 Lambda 내부 멤버로 이동한다.
- 따라서 큰 배열을 복사하지 않고 함수 종료 후에도 Lambda가 데이터를 소유할 수 있다.

### 📌 사용 예제

```cpp
#include <iostream>
#include <vector>
#include <utility>
#include <cstddef>

auto createHandler() {
    std::vector<int> data(1000000, 10);

    return [data = std::move(data)](size_t index) -> int {
        return data.at(index);
    };
}

int main() {
    auto fn = createHandler();

    std::cout << fn(0) << "\n";
    std::cout << fn(100) << "\n";
    std::cout << fn(999999) << "\n";
}
```

#### 🔹 출력

```text
10
10
10
```

### 📌 문법 설명

```cpp
[data = std::move(data)](size_t index) -> int {
    return data.at(index);
}
```

| 구문 | 의미 |
|---|---|
| `[data = std::move(data)]` | 외부 `data`를 이동하여 Closure 내부의 `data`를 초기화 |
| `(size_t index)` | 호출 시 전달받는 인덱스 |
| `-> int` | Lambda 반환형 (`int`), 이 예제에서는 생략 가능 |
| `data.at(index)` | 보관한 벡터의 요소 반환. 범위 밖이면 `std::out_of_range` 예외 발생 |

### 📌 실행 흐름과 수명

1. `createHandler()`에서 정수 100만 개를 담은 `vector`를 생성한다.
2. `std::move(data)`로 벡터를 Lambda의 캡처 멤버에 **이동 생성** 한다. 일반적인 `std::vector` 이동 생성에서는 요소 배열을 복사하지 않는다.
3. 함수가 종료되면 원래 지역 변수는 소멸하지만, 데이터는 반환된 Closure 객체가 소유한다.
4. `fn(index)` 호출은 저장된 데이터의 해당 요소만 읽는다. 벡터 전체를 복사하지 않는다.
5. `fn`이 소멸하면 Closure에 보관된 벡터도 소멸한다.

### 📌 값 캡처와 비교

```cpp
[data]() { /* ... */ }                  // vector 복사
[data = std::move(data)]() { /* ... */ } // vector 이동
[&data]() { /* ... */ }                 // 참조만 저장: 지역 변수 소멸 후 사용 금지
```

### 📌 주의사항

- `std::move` 자체가 이동을 수행하는 것은 아니다. 이동 가능한 값으로 변환하며, 실제 이동은 `vector`의 이동 생성자가 수행한다.
- 이 예제의 Lambda는 데이터를 **소유** 하므로, 원래 지역 변수의 수명에 의존하지 않는다.
- Lambda 객체를 **복사** 하면 캡처된 `vector`도 복사된다. 불필요한 복사는 피한다.
- 이 방식은 데이터를 한 번 준비한 후 여러 차례 조회하거나 계산하는 콜백에 적합하다. 단순히 벡터를 한 번 반환하려는 목적이라면 일반 함수가 더 간단하다.

- 요약: `auto fn = createHandler();`에서 `fn`은 단순 함수 포인터가 아니라, 벡터 데이터를 소유한 **Closure 객체** 다.

### 초기화 갭쳐
- C++ Lambda도 캡처한 데이터를 Closure 객체가 직접 소유하도록 만들면 JavaScript Closure처럼 사용할 수 있습니다.
```cpp
auto createCounter() {
    int count = 0;

    return [count]() mutable {
        return ++count;
    };
}

int main() {
    auto counter = createCounter();

    std::cout << counter() << "\n"; // 1
    std::cout << counter() << "\n"; // 2
    std::cout << counter() << "\n"; // 3
}
```
```javascript
function createCounter() {
    let count = 0;

    return () => ++count;
}

const counter = createCounter();

console.log(counter()); // 1
console.log(counter()); // 2
console.log(counter()); // 3
```
---

