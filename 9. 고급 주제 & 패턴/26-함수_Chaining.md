# 함수 Chaining

## 1. Variadic Template로 여러 함수 합성
= 여러 개의 함수를 받아서 순차적으로 적용하는 방식입니다.
```cpp
#include <iostream>
#include <functional>

// 기본 compose: 두 함수
template <typename F, typename G>
auto compose(F f, G g) {
    return [=](auto x) { return g(f(x)); };
}
```
```cpp
// Variadic compose: 여러 함수
template <typename F, typename... Fs>
auto compose(F f, Fs... fs) {
    return compose(f, compose(fs...));
}
```
```cpp
int main() {
    auto f = compose(
        [](int x){ return x + 5; },
        [](int x){ return x * 4; },
        [](int x){ return x - 2; }
    );

    int result = f(100); // (((100 + 5) * 4) - 2) = 418
    std::cout << result << "\n";
}
```
- 👉 compose를 재귀적으로 정의해서, 여러 함수를 체인처럼 연결할 수 있습니다.

## 2. std::function과 std::accumulate 활용
- 함수들을 컨테이너에 담아 순차적으로 적용하는 방법입니다.
```
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
```
```cpp
int main() {
    std::vector<std::function<int(int)>> funcs = {
        [](int x){ return x + 5; },
        [](int x){ return x * 4; },
        [](int x){ return x - 2; }
    };

    int start = 100;
    int result = std::accumulate(
        funcs.begin(), funcs.end(), start,
        [](int acc, auto& f){ return f(acc); }
    );

    std::cout << result << "\n"; // 418
}
```

- 👉 이 방식은 동적으로 함수 체인을 구성할 때 유용합니다.
- 예를 들어 CAD 파이프라인에서 여러 변환을 순서대로 적용할 때 적합합니다.

## 3. Pipe Operator 스타일 (커스텀 구현)
- C++20부터는 operator|를 오버로드해서 함수 체이닝을 파이프처럼 표현할 수도 있습니다.
```cpp
#include <iostream>
template <typename F, typename T>
auto operator|(T x, F f) {
    return f(x);
}
```
```cpp
int main() {
    auto result = 100
        | [](int x){ return x + 5; }
        | [](int x){ return x * 4; }
        | [](int x){ return x - 2; };

    std::cout << result << "\n"; // 418
}
```
- 👉 이 방식은 함수형 언어 스타일로 직관적인 체이닝을 제공합니다.

## 📌 정리
- Variadic Template → 정적, 컴파일 타임 체이닝 (가장 범용적).
- 컨테이너 + accumulate → 동적, 런타임 체이닝 (실행 중에 파이프라인 구성 가능).
- Pipe Operator → 문법적으로 깔끔한 체이닝 표현.

---
  
