
# 📚 가변 인수(Variadic Arguments) in C & Modern C++

## 1. 전통적인 C 스타일 가변 인수 (`<cstdarg>`)

C 스타일 가변 인수는 `stdarg.h` 또는 `<cstdarg>` 헤더를 이용해 구현하며,  
`va_list`와 관련 매크로(`va_start`, `va_arg`, `va_end`)를 사용합니다.

```cpp
#include <iostream>
#include <cstdarg>
#include <cassert>

void func(int _count, ...) {
    assert(_count >= 0);
    va_list list;
    va_start(list, _count);

    for (int i = 0; i != _count; ++i) {
        std::cout << va_arg(list, int) << std::endl;
    }

    va_end(list);
}

int main() {
    func(5, 1, 2, 3, 4, 5);
    return 0;
}
```

### ⚠️ 단점
- **타입 안정성(type safety)** 없음 → 매개변수 타입을 잘못 주면 런타임 오류
- 전달된 인자의 개수, 타입 검증 불가능
- 디버깅 어려움
- 오버로드, 템플릿 기반 함수와 혼합 시 제약

---

## 2. Modern C++ 스타일 — Variadic Templates

C++11에서 도입된 **가변 템플릿(Variadic Templates)**은  
타입 안정성과 컴파일 타임 인자 해석이 가능해 기존 `va_list`의 단점을 극복합니다.

```cpp
#include <iostream>

void printAll() {
    // 재귀 종료 조건 (인자가 없을 때)
}

template <typename T, typename... Args>
void printAll(T first, Args... args) {
    std::cout << first << std::endl;
    printAll(args...); // 재귀적으로 나머지 인자 처리
}

int main() {
    printAll(1, 2, 3.5, "Hello", 'A');
    return 0;
}
```

### 📝 장점
- **타입 안정성** 확보 (컴파일 타임에 타입 체크)
- 다양한 타입의 인자를 섞어 전달 가능
- `std::forward`를 통한 perfect forwarding 지원
- `sizeof...(Args)`로 인자 개수 계산 가능

---

## 3. Fold Expressions (C++17)

C++17에서는 가변 인자를 순차적으로 처리하는 **Fold Expression**이 도입되었습니다.  
재귀 호출 없이 단일 표현식으로 처리할 수 있습니다.

```cpp
#include <iostream>

template <typename... Args>
void printAll(Args... args) {
    ((std::cout << args << std::endl), ...); // 왼쪽 fold
}

int main() {
    printAll(1, 2, 3.5, "Hello", 'A');
    return 0;
}
```

- `((expr), ...)` → 왼쪽 fold
- `(..., expr)` → 오른쪽 fold
- `(... + args)` → 인자 합산 등의 연산에 활용 가능

---

## 4. 예: 가변 인자 합계 함수 (Modern C++17)

```cpp
#include <iostream>

template <typename... Args>
auto sum(Args... args) {
    return (args + ...); // fold expression (C++17)
}

int main() {
    std::cout << sum(1, 2, 3, 4, 5) << std::endl; // 15
}
```

---

## 📌 정리

| 방식 | 도입 시기 | 타입 안정성 | 장점 | 단점 |
|------|----------|-------------|------|------|
| C 스타일 (`va_list`) | C89 | ❌ | 간단, 오래된 코드와 호환성 | 타입 검증 불가, 안전성 낮음 |
| Variadic Templates | C++11 | ✅ | 타입 안전, 다양한 타입 지원, 컴파일 타임 처리 | 문법이 처음엔 복잡 |
| Fold Expressions | C++17 | ✅ | 재귀 없이 간결, 성능 개선 | C++17 이상 필요 |

---

💡 **권장**:  
- Modern C++에서는 **variadic templates + fold expressions** 사용  
- 기존 C API와의 호환성이 필요한 경우에만 `va_list` 사용  
