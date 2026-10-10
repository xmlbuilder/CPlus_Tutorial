## 📘 C++17 템플릿 타입 제한 (Java Generic Bound 비교)

### 📌 개요
- Java의 `class ClassArray<T extends BaseClass>`처럼 C++17에서도 템플릿 인자에 조건을 부여할 수 있다.
- C++17에서는 `<type_traits>`의 타입 특성과 `static_assert`를 조합하는 방법이 간단하다.

```cpp
#include <type_traits>

template <typename T>
class ClassArray {
    static_assert(std::is_base_of_v<BaseClass, T>,
                  "T must inherit from BaseClass");
};
```

- `std::is_base_of_v<Base, T>`는 `Base` 자체도 허용하며, 비공개 상속도 참으로 판단할 수 있다.
- 실제 공개 상속으로 변환할 수 있는지를 제한하려면 `std::is_convertible_v<T*, Base*>`도 검토한다.

### 📌 상속 이외의 주요 타입 제한

| 요구 조건 | C++17 검사 | 예시 |
|---|---|---|
| 정수형만 | `std::is_integral_v<T>` | `int`, `bool`, `char` 허용 |
| 부동소수점형만 | `std::is_floating_point_v<T>` | `float`, `double` 허용 |
| 포인터형만 | `std::is_pointer_v<T>` | `int*` 허용, `nullptr_t` 불허 |
| 정확히 같은 타입 | `std::is_same_v<T, U>` | `T`가 `std::string`일 때만 허용 |
| 지정 타입으로 암시적 변환 가능 | `std::is_convertible_v<T, U>` | `int` → `double` 허용 |

- `std::is_integral`에는 `bool`과 문자형도 포함된다.
- `std::is_pointer`는 일반 객체/함수 포인터를 검사하며 멤버 포인터나 스마트 포인터는 포함하지 않는다.
- `std::is_same`은 `const` 및 참조 여부까지 비교한다. `std::is_convertible`은 암시적 변환 가능성을 검사하며 명시적(`explicit`) 변환은 허용하지 않는다.

### 📌 클래스에 적용하는 예제

```cpp
#include <type_traits>

template <typename T>
class IntegerArray {
    static_assert(std::is_integral_v<T>, "T must be integral");
};

template <typename T>
class FloatArray {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");
};

template <typename T>
class PointerArray {
    static_assert(std::is_pointer_v<T>, "T must be a pointer");
};

template <typename T>
class DoubleOnly {
    static_assert(std::is_same_v<T, double>, "T must be double");
};

template <typename T>
class ConvertibleToDouble {
    static_assert(std::is_convertible_v<T, double>,
                  "T must be implicitly convertible to double");
};
```

### 📌 `enable_if`와 C++20 Concepts

- C++17에서는 `std::enable_if_t`로 오버로드 후보 자체를 제한할 수도 있다.
- 클래스 템플릿에서 단순히 잘못된 타입을 금지하려면 `static_assert`가 읽기 쉽다.

```cpp
// C++17
#include <type_traits>
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
class IntegralOnly {};
```

- C++20에서는 `requires` 또는 Concepts로 선언부에 제약을 표현할 수 있다.

```cpp
// C++20 예시 (현재 C++17 프로젝트에는 직접 적용하지 않음)
#include <concepts>
template <typename T>
    requires std::integral<T>
class IntegralOnly20 {};
```

### 📌 실행 테스트

- **상속 관계 이외의 5개 타입 제한을 모두 검사** 한다. 각 제한을 실제 템플릿 클래스에 적용하고, `static_assert`로 허용/거부되는 타입을 확인한다.
- 잘못된 템플릿 인스턴스화는 의도적인 컴파일 오류이므로 주석으로 예시를 제공한다.

> 참고: 타입 특성 검사는 **컴파일 시 타입 조건**을 검사할 뿐, 객체의 수명이나 포인터 유효성을 보장하지 않는다.

```cpp
#include "test_template_type_constraints.h"


#include <type_traits>
#include <string>
#include <iostream>

// C++17: 상속 관계 외의 5가지 타입 제약

template <typename T>
struct IntegralOnly {
    static_assert(std::is_integral_v<T>, "T must be integral");
    using value_type = T;
};

template <typename T>
struct FloatingOnly {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");
    using value_type = T;
};

template <typename T>
struct PointerOnly {
    static_assert(std::is_pointer_v<T>, "T must be a pointer");
    using value_type = T;
};

template <typename T>
struct StringOnly {
    static_assert(std::is_same_v<T, std::string>, "T must be std::string");
    using value_type = T;
};

template <typename T>
struct ConvertibleToDouble {
    static_assert(std::is_convertible_v<T, double>,
                  "T must be implicitly convertible to double");
    using value_type = T;
};

struct ImplicitDouble {
    operator double() const { return 1.0; }
};

struct ExplicitDouble {
    explicit operator double() const { return 1.0; }
};

static_assert(std::is_same_v<typename IntegralOnly<int>::value_type, int>);
static_assert(std::is_same_v<typename IntegralOnly<bool>::value_type, bool>);
static_assert(!std::is_integral_v<double>);
static_assert(!std::is_integral_v<std::string>);

static_assert(std::is_same_v<typename FloatingOnly<double>::value_type, double>);
static_assert(std::is_same_v<typename FloatingOnly<float>::value_type, float>);
static_assert(!std::is_floating_point_v<int>);

static_assert(std::is_same_v<typename PointerOnly<int*>::value_type, int*>);
static_assert(std::is_same_v<typename PointerOnly<void(*)()>::value_type, void(*)()>);
static_assert(!std::is_pointer_v<int>);
static_assert(!std::is_pointer_v<std::nullptr_t>);

static_assert(std::is_same_v<typename StringOnly<std::string>::value_type, std::string>);
static_assert(!std::is_same_v<const std::string, std::string>);
static_assert(!std::is_same_v<const char*, std::string>);

static_assert(std::is_same_v<typename ConvertibleToDouble<int>::value_type, int>);
static_assert(std::is_same_v<typename ConvertibleToDouble<ImplicitDouble>::value_type, ImplicitDouble>);
static_assert(std::is_convertible_v<float, double>);
static_assert(!std::is_convertible_v<ExplicitDouble, double>);
static_assert(!std::is_convertible_v<std::string, double>);


int main() {
    IntegralOnly<int> a;
    FloatingOnly<double> b;
    PointerOnly<int*> c;
    StringOnly<std::string> d;
    ConvertibleToDouble<ImplicitDouble> e;
    (void)a; (void)b; (void)c; (void)d; (void)e;

    std::cout << "[PASS] integral\n"
              << "[PASS] floating_point\n"
              << "[PASS] pointer\n"
              << "[PASS] same\n"
              << "[PASS] convertible\n"
              << "RESULT: PASS (5 categories)\n";


    return 0;
}
```

```cpp

#include "test_template_type_constraints_values.h"
#include <type_traits>
#include <string>
#include <iostream>
#include <utility>

// 템플릿 인수 T에 대한 제약. Set(T)는 T로 암시적 변환 가능한 값을 받는다.
template <typename T>
class IntegralOnly {
    static_assert(std::is_integral_v<T>, "T must be integral");
public:
    void Set(T value) { value_ = value; }
    T Get() const { return value_; }
private:
    T value_{};
};

template <typename T>
class FloatingOnly {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");
public:
    void Set(T value) { value_ = value; }
    T Get() const { return value_; }
private:
    T value_{};
};

template <typename T>
class PointerOnly {
    static_assert(std::is_pointer_v<T>, "T must be a pointer");
public:
    void Set(T value) { value_ = value; }
    T Get() const { return value_; }
private:
    T value_{};
};

template <typename T>
class StringOnly {
    static_assert(std::is_same_v<T, std::string>, "T must be std::string");
public:
    void Set(T value) { value_ = std::move(value); }
    const T& Get() const { return value_; }
private:
    T value_{};
};

struct ImplicitDouble {
    double value;
    operator double() const { return value; }
};
struct ExplicitDouble {
    double value;
    explicit operator double() const { return value; }
};

template <typename T>
class ConvertibleToDouble {
    static_assert(std::is_convertible_v<T, double>,
                  "T must be implicitly convertible to double");
public:
    void Set(T value) { value_ = std::move(value); }
    double AsDouble() const { return value_; }
private:
    T value_{};
};

int main() {
    IntegralOnly<int> integral;
    integral.Set(42);
    std::cout << "integral: " << integral.Get() << '\n';

    FloatingOnly<double> floating;
    floating.Set(3.14);
    std::cout << "floating: " << floating.Get() << '\n';

    floating.Set(3.15f);
    std::cout << "floating: " << floating.Get() << '\n';

    int number = 10;
    PointerOnly<int*> pointer;
    pointer.Set(&number);
    std::cout << "pointer: " << *pointer.Get() << '\n';

    StringOnly<std::string> str;
    str.Set(std::string("hello"));
    std::cout << "same: " << str.Get() << '\n';

    ConvertibleToDouble<ImplicitDouble> convertible;
    convertible.Set(ImplicitDouble{2.5});
    std::cout << "convertible: " << convertible.AsDouble() << '\n';


    // 주의: T에 대한 제약과 Set() 인수 제약은 서로 다르다.
    // 예를 들어 아래 두 코드는 컴파일된다 (암시적 변환 발생).
    integral.Set(3.9);         // double -> int, 값은 3 (소수 부분 손실)
    floating.Set(7);           // int -> double
    str.Set("literal");       // const char[] -> std::string
    std::cout << "converted integral: " << integral.Get() << '\n';
    std::cout << "converted floating: " << floating.Get() << '\n';
    std::cout << "converted string: " << str.Get() << '\n';


    // 하나씩 주석을 풀어 확인: 템플릿 T 자체가 제한에 위배됨
    //IntegralOnly<double> invalidIntegral;
    // FloatingOnly<int> invalidFloating;
    // PointerOnly<int> invalidPointer;
    // StringOnly<const char*> invalidString;
    // ConvertibleToDouble<ExplicitDouble> invalidConvertible;
    // (void)invalidIntegral; (void)invalidFloating; (void)invalidPointer;
    // (void)invalidString; (void)invalidConvertible;

    // T는 유효하지만 Set()에 전달하는 값이 T로 변환 불가능함
    //integral.Set(std::string("abc"));
    //floating.Set(std::string("abc"));
    //pointer.Set(123); // nullptr은 가능하지만 일반 정수 123은 불가능
    //str.Set(123);
    //convertible.Set(ExplicitDouble{1.0});


    return 0;
}

```
---

### 📌 상속 제한: Java `T extends BaseClass`

```cpp
#include <type_traits>

template <typename T>
class ClassArray {
    static_assert(std::is_base_of_v<BaseClass, T>,
                  "T must inherit from BaseClass");
};
```
- `std::is_base_of_v<Base, T>`는 `Base` 자체도 허용하며, 비공개 상속도 참으로 판단할 수 있다.
- 실제 공개 상속으로 변환할 수 있는지를 제한하려면 `std::is_convertible_v<T*, Base*>`도 검토한다.
- Java의 `T extends BaseClass`와 유사하게 C++17에서는 `std::is_base_of_v<BaseClass, T>`로 검사한다.
- 아래 예제는 **실제 객체를 `Set()`에 전달**하는 방식이다.

```cpp
#include <type_traits>

struct BaseClass {
    virtual ~BaseClass() = default;
    virtual const char* Name() const { return "BaseClass"; }
};
struct DerivedClass : BaseClass {
    const char* Name() const override { return "DerivedClass"; }
};
struct OtherClass {};

template <typename T>
class DerivedOnly {
    static_assert(std::is_base_of_v<BaseClass, T>,
                  "T must inherit from BaseClass");
public:
    void Set(const T& value) { value_ = value; }
    const T& Get() const { return value_; }
private:
    T value_{};
};

void TestInheritance() {
    DerivedOnly<BaseClass> base;
    base.Set(BaseClass{});                 // OK

    DerivedOnly<DerivedClass> derived;
    derived.Set(DerivedClass{});          // OK

    // DerivedOnly<OtherClass> invalid;  // 컴파일 오류: 상속 조건 위반
    // derived.Set(OtherClass{});         // 컴파일 오류: 값의 타입 불일치
}
```
- `std::is_base_of_v`는 비공개 상속도 참으로 판단할 수 있으므로 **공개 상속으로 변환 가능** 해야 한다면 `std::is_convertible_v<T*, BaseClass*>`를 추가로 확인한다.
- 위 예제는 객체를 **값으로 저장** 하므로 `DerivedOnly<BaseClass>`에 파생 객체를 전달하면 객체 슬라이싱이 발생할 수 있다. 다형성이 필요한 저장소는 별도의 포인터/참조 소유권 설계가 필요하다.

### 📌 T 자체가 포인터인 경우
- 이때는 std::is_base_of_v<BaseClass, T>를 그대로 사용할 수 없습니다.
- T가 BaseClass* 라면 포인터 타입이기 때문입니다.
- 이 경우 std::remove_pointer_t<T>를 사용합니다.
```cpp
template <typename T>
class ClassArray
{
    static_assert(
        std::is_pointer_v<T>,
        "T must be a pointer"
    );

    static_assert(
        std::is_base_of_v<
            BaseClass,
            std::remove_pointer_t<T>
        >,
        "T must point to BaseClass or derived class"
    );

public:
    void Add(T obj)
    {
        m_items.push_back(obj);
    }

private:
    std::vector<T> m_items;
};
```

### 테스트 코드
```cpp

// Java: class ClassArray<T extends BaseClass> 와 유사한 상속 제한
struct BaseClass {
    virtual ~BaseClass() = default;
    virtual const char* Name() const { return "BaseClass"; }
};
struct DerivedClass : BaseClass {
    const char* Name() const override { return "DerivedClass"; }
};
struct OtherClass {};

template <typename T>
class DerivedOnly {
    static_assert(std::is_base_of_v<BaseClass, T>,
                  "T must inherit from BaseClass");
public:
    void Set(const T& value) { value_ = value; }
    const T& Get() const { return value_; }
private:
    T value_{};
};


template <typename T>
class PointerArray
{
    static_assert(
        std::is_pointer_v<T>,
        "T must be a pointer"
    );

    static_assert(
        std::is_base_of_v<
            BaseClass,
            std::remove_pointer_t<T>
        >,
        "T must point to BaseClass or derived class"
    );

public:
    void Add(T obj)
    {
        m_items.push_back(obj);
    }

private:
    std::vector<T> m_items;
};


int main()
{
    // 상속 제한: BaseClass와 DerivedClass는 허용된다.
    DerivedOnly<BaseClass> base;
    base.Set(BaseClass{});
    std::cout << "base: " << base.Get().Name() << '\n';

    DerivedOnly<DerivedClass> derived;
    derived.Set(DerivedClass{});
    std::cout << "derived: " << derived.Get().Name() << '\n';


    //DerivedOnly<OtherClass> invalidInheritance;
    //derived.Set(OtherClass{});


  
    PointerArray<BaseClass*> a;   // OK
    PointerArray<DerivedClass*> b;    // OK

    //PointerArray<OtherClass*> c;

    return 0;
}
```


---

