# C++ / Rust Decorator

## ✅ C++에서 데코레이터처럼 동작하는 구조
C++은 메타 언어가 없지만, 함수 포인터, 람다, 템플릿, Functor를 활용해서
함수를 감싸는 구조를 만들 수 있음.

## 🔧 예시: 실행 시간 측정 데코레이터
```cpp
#include <iostream>
#include <chrono>
#include <functional>

template<typename Func, typename... Args>
auto perf_clock(Func func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "[Time] " << elapsed.count() << "s\n";
    return result;
}

// 원래 함수
int sum_func(int a, int b) {
    return a + b;
}

int main() {
    int result = perf_clock(sum_func, 10, 20);
    std::cout << "Result: " << result << std::endl;
}
```

- perf_clock()은 데코레이터처럼 함수를 감싸서 실행 시간 측정
- std::function이나 람다로 더 일반화 가능

---

