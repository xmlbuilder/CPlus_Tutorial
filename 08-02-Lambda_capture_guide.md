# 🧠 C++ 람다 캡처 정리 (Value / Reference / Init-Capture / Generic / 수명 이슈)

C++ 람다는 **외부 스코프 변수**를 *캡처(capture)* 해서 본문에서 사용할 수 있습니다.  
여기서는 **값 캡처**, **참조 캡처**, **초기화 캡처(init-capture)**, **제네릭 람다**, 그리고 **수명(Lifetime) 함정**까지 한 번에 정리합니다.

---

## 1) 값 캡처 (by value)

외부 변수를 **복사**해서 람다 안에서 사용합니다.  
람다 생성 시점의 값이 저장되며 이후 외부 값이 바뀌어도 람다 내부 값은 **변하지 않습니다**.

```cpp
void lambda_value_capture(){
    int value = 1;
    auto copy_value = [value]{
        return value;
    };
    value = 100;
    auto stored_value = copy_value();
    std::cout << "Stored value " << stored_value << std::endl; // Stored value 1
}
```

> 값 캡처는 기본적으로 **읽기 전용**입니다. 내부에서 수정하려면 `mutable`을 사용하세요. (외부 원본은 변하지 않음)

---

## 2) 참조 캡처 (by reference)

외부 변수를 **참조**로 잡습니다.  
람다 실행 시점의 값(외부 변수의 현재 값)을 읽습니다.

```cpp
void lambda_reference_capture(){
    int value = 1;
    auto ref_value = [&value]{
        return value;
    };
    value = 100;
    auto stored_value = ref_value();
    std::cout << "Stored value " << stored_value << std::endl; // Stored value 100
}
```

> **주의**: 참조 캡처는 캡처한 변수의 **수명**이 람다보다 짧으면 **댕글링 참조**가 됩니다.

---

## 3) 초기화 캡처 (init-capture / C++14~)

람다 캡처 리스트에서 **새 변수 선언 및 초기화**가 가능합니다.  
특히 `std::unique_ptr` 같은 **이동 전용** 자원을 안전하게 넘길 때 유용합니다.

```cpp
void lambda_expression_capture() {
    auto important = std::make_unique<int>(1);
    auto add = [v1 = 1, v2 = std::move(important)](int x, int y) -> int {
        return x + y + v1 + (*v2);
    };
    std::cout << add(3,4) << std::endl;        // 9
    std::cout << important.get() << std::endl; // 0 (moved)
}
```

포인트:
- `v1 = 1` 처럼 **값을 직접 캡처** 가능
- `v2 = std::move(important)` 로 **소유권 이동** 가능
- 이동 후 원본 포인터는 `nullptr`

---

## 4) 제네릭 람다 (C++14~)

매개변수 타입을 `auto`로 받아 **템플릿 함수처럼** 동작하는 람다입니다.  
아래 예제는 값/참조 **캡처 방식에 따른 차이**를 보여줍니다.

```cpp
void lambda_generic(){
    int value = 3;

    auto generic  = [=](auto x, auto y){ return x + y + value; };
    auto generic2 = [&](auto x, auto y){ return x + y + value; };

    value = 100;

    std::cout << generic(1, 2)      << std::endl;   // 6   (value=3 복사됨)
    std::cout << generic2(1.1, 2.2) << std::endl;   // 103.3 (value 참조)
}
```

---

## 5) 수명(Lifetime) 함정: **참조 캡처를 반환하지 마세요**

다음 코드는 로컬 변수 `a`를 **참조**로 캡처한 람다를 **반환**합니다.  
`retFunc`가 끝나면 `a`는 소멸하므로, 반환된 람다의 참조는 **댕글링**이 됩니다.

```cpp
#include <functional>
#include <iostream>

std::function<int(int)> retFunc () {
    int a = 300;
    return [&a](int b) {
        std::cout << "a value = " << a << std::endl;
        return a + b;
    };
}

int main() {
    std::function<int(int)> ret = retFunc();
    std::cout << ret(10) << std::endl; // UB(비정의 동작): 운 좋게 300처럼 보일 수도, 깨질 수도
}
```

### 안전한 대안

#### ✔ 값으로 복사 캡처
```cpp
std::function<int(int)> retFuncSafe () {
    int a = 300;
    return [a](int b) {                 // ← a를 값으로 캡처
        std::cout << "a value = " << a << std::endl;
        return a + b;
    };
}
```

#### ✔ 소유권 있는 객체를 캡처 (`std::shared_ptr` 등)
```cpp
std::function<int(int)> retFuncShared () {
    auto pa = std::make_shared<int>(300);
    return [pa](int b) {                // shared_ptr 복사 → 참조 카운트 증가
        std::cout << "a value = " << *pa << std::endl;
        return *pa + b;
    };
}
```

---

## 6) 요약 / 베스트 프랙티스

- **값 캡처**: 기본적으로 안전. 내부 수정이 필요하면 `mutable` 사용(외부엔 영향 없음).
- **참조 캡처**: 성능은 좋지만 **수명**을 보장할 수 있는 범위에서만 사용.
- **반환하는 람다**에는 **값 캡처**나 **소유권 객체 캡처**를 사용해 댕글링 방지.
- **초기화 캡처**로 `unique_ptr` 이동, 계산된 값 보관 등 활용.
- **제네릭 람다**로 템플릿처럼 유연하게 처리.
- 표준 알고리즘과 함께 쓰면 가독성과 생산성이 크게 올라감.

---

### 전체 예제 모음

```cpp
#include <iostream>
#include <functional>
#include <memory>

void lambda_value_capture();
void lambda_reference_capture();
void lambda_expression_capture();
void lambda_generic();

std::function<int(int)> retFuncSafe ();
std::function<int(int)> retFuncShared ();

int main() {
    lambda_value_capture();
    lambda_reference_capture();
    lambda_expression_capture();
    lambda_generic();

    auto f1 = retFuncSafe();
    std::cout << f1(10) << std::endl; // 310

    auto f2 = retFuncShared();
    std::cout << f2(10) << std::endl; // 310
}
```
