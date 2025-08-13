# noexcept in C++

## 📌 noexcept란?
`noexcept`는 **이 함수가 예외를 던지지 않는다**는 것을 컴파일러와 호출자에게 명시하는 키워드입니다.  
- C++11에서 도입, `throw()`의 대체.
- 성능 최적화와 코드의 예외 안전성 계약(Contract) 명시.
- `noexcept(true)`는 `noexcept`와 동일.
- `noexcept(false)`는 예외를 던질 수 있다는 의미.

---

## 🔍 noexcept와 throw()의 차이

| 구분 | `throw()` (C++98/03) | `noexcept` (C++11~) |
|------|----------------------|----------------------|
| 문법 | `void func() throw();` | `void func() noexcept;` |
| 의미 | 함수가 예외를 던지지 않음 | 함수가 예외를 던지지 않음 |
| 표준 상태 | **C++11부터 deprecated**, C++17에서 제거 | 현재 표준 |
| 런타임 동작 | 예외 발생 시 `std::unexpected()` 호출 후 종료 | 예외 발생 시 `std::terminate()` 호출 후 종료 |
| 유연성 | 예외 타입을 나열 가능 → `void func() throw(int, std::string);` | 타입 나열 불가, 단순 불린 값(true/false)만 가능 |
| 최적화 | 제한적 | 컨테이너 최적화, 이동 연산 최적화에 적극 활용됨 |

---

## ⚠️ noexcept 함수에서 예외가 발생하면?
- 예외가 외부로 전파되면 **`std::terminate()`**가 호출되어 프로그램이 강제 종료됩니다.
- 따라서 내부에서 예외가 발생할 가능성이 있다면 반드시 **`try-catch`로 처리**해야 함.

---

## 🔹 안전한 예시
```cpp
#include <iostream>
#include <stdexcept>

void func() noexcept {
    try {
        throw std::runtime_error("error inside noexcept");
    } catch (const std::exception& e) {
        std::cerr << "[func] Exception handled: " << e.what() << "\n";
    }
}

int main() {
    func(); // 안전하게 실행됨
    std::cout << "Program continues...\n";
}
```
**출력**
```
[func] Exception handled: error inside noexcept
Program continues...
```

---

## 🔹 위험한 예시
```cpp
#include <stdexcept>

void func() noexcept {
    throw std::runtime_error("error inside noexcept");
}

int main() {
    func(); // terminate 발생
}
```
**출력**
```
terminate called after throwing an instance of 'std::runtime_error'
```

---

## 📊 noexcept 사용 가이드

| 상황 | 사용 권장 여부 | 이유 |
|------|---------------|------|
| 단순 연산, 메모리 접근, 소멸자 | ✅ | 예외가 발생할 여지가 거의 없음 |
| move 생성자/이동 연산자 | ✅ | 컨테이너 재배치 최적화에 도움 |
| 예외 발생 가능성이 없는 래퍼 함수 | ✅ | 코드 명시성 & 성능 향상 |
| 예외 발생 가능성이 있는 함수 | ⚠️ | 반드시 내부에서 `try-catch` 처리 필요 |
| 외부 라이브러리 호출 포함 | ⚠️ | 호출 대상이 예외를 던질 수 있으면 위험 |
| 디버깅 중 예외 추적 필요 | ❌ | `noexcept`가 있으면 디버깅이 어려워질 수 있음 |

---

## 💡 결론
- `noexcept`는 `throw()`의 현대적 대체품이며, **성능 최적화**와 **예외 안전성 계약**에 도움이 됨.
- 예외 발생 가능성이 있는 경우 반드시 내부에서 `try-catch`로 처리.
- `throw()`는 더 이상 사용하지 않으며, C++17에서 완전히 제거됨.
