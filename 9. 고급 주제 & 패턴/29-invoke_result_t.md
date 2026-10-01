## 📘 std::invoke_result_t란?
- 정의:
  - C++17에서 도입된 타입 트레이트(type trait)입니다.  
  - 특정 함수(또는 호출 가능한 객체)를 주어진 인자 타입으로 호출했을 때 반환되는 타입을 컴파일 시점에 추론해 줍니다.
- 형식:
```cpp
std::invoke_result_t<F, Args...>
```
- 여기서
  - F = 함수 타입 또는 호출 가능한 객체(callable)
  - Args... = 함수에 전달할 인자 타입들
- 동작:
  - `std::invoke_result<F, Args...>::type` 의 축약형(alias template)이 바로 `std::invoke_result_t<F, Args...>` 입니다.

### 📌 예시
```cpp
#include <type_traits>
#include <string>
#include <iostream>

int add(int a, int b) { return a + b; }

struct Greeter {
    std::string operator()(const std::string& name) {
        return "Hello " + name;
    }
};
```
```cpp
int main() {
    // 함수 add(int,int)의 반환 타입 추론
    using AddResult = std::invoke_result_t<decltype(add), int, int>;
    static_assert(std::is_same_v<AddResult, int>);

    // 함수 객체 Greeter의 operator()(std::string)의 반환 타입 추론
    using GreetResult = std::invoke_result_t<Greeter, std::string>;
    static_assert(std::is_same_v<GreetResult, std::string>);

    std::cout << "AddResult is int\n";
    std::cout << "GreetResult is std::string\n";
}
```

#### 🔹 출력:
```
AddResult is int
GreetResult is std::string
```


### 📌 왜 쓰는가?
- 범용 코드 작성:
  - 템플릿에서 함수 반환 타입을 미리 알 수 없을 때, std::invoke_result_t로 안전하게 추론합니다.
- Memoize 구현에서 사용:
```cpp
Result = std::invoke_result_t<Func&, Args...>;
```
- Func를 Args...로 호출했을 때 반환되는 타입을 자동으로 Result로 지정합니다.
- 덕분에 Func가 int를 반환하든, std::string을 반환하든, Memoize가 자동으로 맞춰집니다.

### 📌 정리하면:
- std::invoke_result_t는 **이 함수(또는 호출 객체)를 이런 인자로 호출했을 때 반환되는 타입** 을 컴파일러가 알려주는 도구입니다.
---
