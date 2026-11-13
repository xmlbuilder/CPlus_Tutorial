## 📘 `operator bool()` 이해하기

### 📌 1. 무엇을 하는 함수인가?

- `operator bool()`은 클래스의 객체를 `bool`로 변환할 때 사용할 규칙을 정의하는 특별한 멤버 함수입니다.
- C++에서는 이러한 함수를 **변환 함수(conversion function)** 라고 합니다.
- 어떤 상태를 `true` 또는 `false`로 해석할지는 함수 본문이 결정합니다.

### 📌 2. 일반 멤버 함수와 비교

- 상태를 확인하는 일반 함수는 다음처럼 작성할 수 있습니다.

```cpp
bool hasFunction() const noexcept {
    return static_cast<bool>(fn_);
}
```
- 사용자는 이름을 직접 호출합니다.

```cpp
if (task.hasFunction()) {
    // 호출 대상이 있음
}
```

- 같은 판단을 변환 함수로 제공하면 다음과 같습니다.

```cpp
explicit operator bool() const noexcept {
    return static_cast<bool>(fn_);
}
```

- 이제 조건문에서 객체를 직접 사용할 수 있습니다.

```cpp
if (task) {
    // 호출 대상이 있음
}
```

- 개념적으로 `task.operator bool()`의 반환값을 조건으로 사용하는 것입니다.
- 이 멤버 함수를 직접 호출하는 것도 가능하지만 보통은 `if (task)`라고 씁니다.

### 📌 3. 선언의 각 부분

```cpp
explicit operator bool() const noexcept
```

| 구성 | 의미 |
|---|---|
| `operator bool` | 객체를 `bool`로 변환하는 함수 |
| `()` | 인자를 받지 않음 |
| `const` | 이 함수에서 객체의 일반 데이터 멤버를 변경하지 않음 |
| `noexcept` | 예외를 외부로 내보내지 않는 함수임을 선언 |
| `explicit` | 일반적인 암시적 변환을 제한 |

- 변환 함수에는 앞에 반환 타입을 쓰지 않습니다.
- 변환 대상 타입이 이미 이름에 포함되어 있습니다.

```cpp
bool hasFunction() const;        // 일반 함수
explicit operator bool() const; // 변환 함수
// bool operator bool() const;  // 잘못된 선언
```

- `noexcept` 함수에서 예외가 빠져나가면 `std::terminate()`가 호출됩니다.

### 📌 4. true / false의 기준은 직접 정한다

```cpp
class Number {
    int value_;
public:
    explicit Number(int value) : value_(value) {}

    explicit operator bool() const noexcept {
        return value_ > 0;
    }
};

Number a{10};
Number b{0};
Number c{-5};

if (a) { /* 실행됨 */ }
if (b) { /* 실행되지 않음 */ }
if (c) { /* 실행되지 않음 */ }
```

- 세 객체는 모두 존재하지만 이 클래스는 양수일 때만 `true`를 반환하도록 정의했습니다.

### 📌 5. 이번 function.hpp 에서의 구현

- 공통 기반 클래스 `Storage`에는 다음 코드가 있습니다.

```cpp
std::function<Signature> fn_;

explicit operator bool() const noexcept {
    return bool(fn_);
}
```
- `bool(fn_)`는 `std::function`이 제공하는 변환 함수를 사용합니다.
  - 호출 대상이 저장되어 있으면 `true`
  - 호출 대상이 비어 있으면 `false`


```cpp
#include "function.hpp"
using namespace functional;

Function<int, int> empty{};
Function<int, int> twice = [](int x) { return x * 2; };

bool a = static_cast<bool>(empty); // false
bool b = static_cast<bool>(twice); // true
```

- 객체 생성과 호출 대상 등록은 별개입니다.
- `empty`라는 래퍼 객체는 존재하지만 내부에 실행할 함수가 없습니다.

```text
static_cast<bool>(empty)
  → Storage의 operator bool()
  → fn_의 std::function::operator bool()
  → 호출 대상이 없으므로 false
```
```cpp
Function<int, int> zero = [](int) { return 0; };

bool exists = static_cast<bool>(zero); // true: 호출 대상이 있음
int result = zero.apply(10);           // 0: 실제 실행 결과
```
- 빈 래퍼의 기본 호출 메서드를 실행하면 `std::bad_function_call` 예외가 발생합니다.

### 📌 6. explicit의 의미

- `explicit`은 의도하지 않은 자동 변환을 제한합니다.

```cpp
Function<int, int> f = [](int x) { return x * 2; };

// bool a = f;                  // 오류: 복사 초기화의 암시적 변환
bool b = static_cast<bool>(f);   // 가능: 명시적 변환
bool c(f);                      // 가능: 직접 초기화
bool d{f};                      // 가능: 직접 리스트 초기화
```

- 하지만 조건을 판단하는 문맥에서는 `explicit operator bool()`도 사용할 수 있습니다.
- 이를 **문맥적 bool 변환(contextual conversion to bool)** 이라고 합니다.

```cpp
if (f) { /* 호출 대상이 있음 */ }
if (!f) { /* 호출 대상이 없음 */ }

bool both = f && f;
bool either = f || f;
```
- 위 `!`, `&&`, `||`는 이번 래퍼에서 별도로 오버로드하지 않은 내장 연산자입니다.
- 내장 `&&`와 `||`는 단락 평가합니다.
- `while (f)`도 문법적으로 가능하지만 반복문 안에서 `f`를 빈 상태로 바꾸거나 탈출하지 않으면 계속 반복할 수 있습니다.

### 📌 7. check(f)와 check(bool(f))의 차이

```cpp
void check(bool condition);
```

- 일반 함수 인자 전달은 조건문의 문맥적 bool 변환에 해당하지 않습니다.

```cpp
// check(f);                    // 오류: explicit 변환을 자동 적용하지 않음
check(bool(f));                 // 가능: 명시적 변환
check(static_cast<bool>(f));    // 가능: 명시적 변환
```

- 그래서 테스트에서는 다음 두 줄을 사용했습니다.

```cpp
check(bool(f));
check(!bool(Function<int, int>{}));
```

- 첫 번째 줄은 `f`에 호출 대상이 있는지 검사합니다. 
- 두 번째 줄은 기본 생성한 빈 래퍼가 `false`로 변환되는지 검사합니다.
- 빈 래퍼는 `false`이고, 앞의 `!` 때문에 최종 검사값은 `true`가 됩니다.

### 📌 8. bool(f);를 단독으로 쓰면 왜 오류가 나는가?

- 블록 안에서 다음 문장은 변환 표현식처럼 보이지만 선언으로도 해석할 수 있습니다.

```cpp
bool(f);
```
- C++은 이처럼 선언과 표현식으로 모두 해석될 수 있는 문장을 선언으로 해석합니다. 따라서 다음과 같은 변수 선언으로 처리됩니다.

```cpp
bool f;
```

- 같은 스코프에 이미 `Function<int, int> f`가 있다면 충돌하여 컴파일 오류가 발생합니다.
- 해당 이름이 없다면 새 `bool` 변수를 선언할 수도 있으므로, `bool(f);`가 항상 문법 오류인 것은 아닙니다.
- 반면 다음 코드는 함수 인자 자리에 표현식이 필요하므로 명시적 변환으로 해석됩니다.

```cpp
check(bool(f));
```

- 형 변환 의도를 분명하게 쓰려면 다음을 권장합니다.

```cpp
bool hasFunction = static_cast<bool>(f);
check(hasFunction);
```

- 변환 결과를 사용하지 않는 단독 표현식도 다음처럼 명확하게 쓸 수 있습니다.

```cpp
static_cast<bool>(f); // 변환 후 결과를 버림; 실용적인 상태 검사는 아님
```

### 📌 9. 포인터의 bool 변환과 비교

| 대상 | bool 판단 방식 |
|---|---|
| 원시 포인터 `p` | 널 포인터이면 false, 아니면 true |
| 이번 래퍼 `f` | 클래스의 `operator bool()` 호출 |
| 내부 `std::function` | 호출 대상이 없으면 false, 있으면 true |

```cpp
Function<int, int> f{};
Function<int, int>* p = &f;

bool pointerExists = static_cast<bool>(p); // true: p는 널이 아님
bool targetExists = static_cast<bool>(f);  // false: 내부 호출 대상은 비어 있음
```

- 포인터 `p`가 유효한 객체를 가리키는 것과 래퍼 `f`에 호출 대상이 있는 것은 서로 다른 상태입니다.
- 또한 원시 포인터가 널이 아니라고 해서 대상 객체가 아직 살아 있다는 보장은 없습니다.

### 📌 10. 사용 기준

- 조건문에서는 `if (f)` 또는 `if (!f)`를 사용하면 됩니다.
- bool 변수에 저장하거나 `check(bool)` 같은 일반 함수에 전달할 때는 `static_cast<bool>(f)`를 사용하면 의도가 명확합니다.
- 이번 구현에서 `operator bool()`은 **호출 대상의 유무**를 나타냅니다.
- 캡처한 포인터의 대상이 살아 있는지, 실행이 성공할지, 실행 결과가 참일지는 보장하지 않습니다. 캡처 대상의 수명 관리는 호출자 책임입니다.

---


