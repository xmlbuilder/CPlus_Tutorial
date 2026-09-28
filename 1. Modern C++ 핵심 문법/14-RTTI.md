## 📘 RTTI(Runtime Type Information) & (Proto) Reflection

### 📌 목표
- typeid, std::type_info, dynamic_cast를 정확히 이해하고 안전하게 쓰기
- std::type_index, std::any, std::variant로 런타임 타입 처리 패턴 익히기
- **진짜 리플렉션** 이 표준화되지 않았다는 현실과, 대신 활용 가능한 기법 정리
- 다형성과 클래스 계층 이해를 돕는 실용 예제 제공

### 📌 1) RTTI란?
- RTTI (Run-Time Type Information) 는 실행 중 객체의 실제(dynamic) 타입 정보를 알기 위한 메커니즘.
- 대표 기능:
    - typeid(expr) → std::type_info 반환
    - dynamic_cast<T*>(ptr) / dynamic_cast<T&>(ref) → 안전한 다운캐스팅(실패 시 nullptr 또는 std::bad_cast)
    - 동작 전제: 다형성(polymorphic) 타입, 즉 가상 함수(보통 가상 소멸자) 를 가진 베이스여야 함
    - 컴파일러 옵션에 RTTI 비활성화가 있을 수 있습니다.
    - GCC/Clang: -fno-rtti를 켜면 RTTI 기능이 빠집니다.
    - MSVC: 기본 활성화.

### 📌 2) typeid의 규칙 요약

- typeid(T) (T가 타입) → 컴파일타임 타입 정보
- typeid(expr) (expr이 폴리모픽 타입의 glvalue이고, 참조/역참조를 통해 접근한 경우) → 동적 타입
- 폴리모픽이 아니라면 → 정적 타입만 반환

```cpp
struct Base { virtual ~Base() = default; };
struct Derived : Base {};

Base* p = new Derived;
typeid(p)        // => Base*  (포인터의 정적 타입)
typeid(*p)       // => Derived (폴리모픽 + 역참조 → 동적 타입)
```

### 📌 3) dynamic_cast 안전 다운캐스팅
- 포인터 변환: 실패 시 nullptr
- 참조 변환: 실패 시 std::bad_cast 예외

```cpp
Derived* d = dynamic_cast<Derived*>(p); // p: Base*
if (!d) { /* 실패 처리 */ }
```

- static_cast는 컴파일타임만 보고 변환하여 런타임 안전성 없음.
- 다운캐스팅에 static_cast 사용은 주의.

### 📌 4) C++20 “리플렉션” 현황 (요약)

- 2025년 현재 공식 표준 리플렉션은 없음.

- 대안:
    - 컴파일러별 확장/라이브러리: magic_get, nameof, Boost.PFR(Plain Function Reflection: POD 구조체를 튜플처럼 다루기) 등
    - 메타프로그래밍/컨셉/타입트레이트: std::is_base_of_v<>, std::is_same_v<>, std::is_polymorphic_v<> 등
    - 런타임 컨테이너: std::any, std::variant + 방문자



### 📌 5) 예제 코드 모음

#### 📄 Case 01 rtti_typeid
```cpp
#include <iostream>
#include <typeinfo>

struct Base { virtual ~Base() = default; };
struct Derived : Base {};
struct NonPoly {};

int main() {
    Base* bp = new Derived;
    Derived d;
    NonPoly np;

    std::cout << "typeid(bp).name():    " << typeid(bp).name() << "\n";   // Base*
    std::cout << "typeid(*bp).name():   " << typeid(*bp).name() << "\n";  // Derived (동적)
    std::cout << "typeid(d).name():     " << typeid(d).name() << "\n";    // Derived
    std::cout << "typeid(np).name():    " << typeid(np).name() << "\n";   // NonPoly (정적)

    delete bp;
}
```

#### 📄 Case 02 dynamic_cast
```cpp
#include <iostream>
#include <typeinfo>
#include <stdexcept>

struct Base { virtual ~Base() = default; };
struct Dog  : Base {};
struct Cat  : Base {};

void feedDog(Base& b) {
    try {
        Dog& d = dynamic_cast<Dog&>(b); // 실패 시 bad_cast
        std::cout << "Feeding dog!\n";
    } catch (const std::bad_cast&) {
        std::cout << "Not a dog. Skip.\n";
    }
}

int main() {
    Dog dog; Cat cat;
    feedDog(dog); // OK
    feedDog(cat); // Not a dog
}
```

#### 📄 Case 03 type_index_map
```cpp
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <memory>

struct Base { virtual ~Base() = default; };
struct A : Base {};
struct B : Base {};

using Handler = void(*)(Base&);

void handleA(Base& b) { std::cout << "Handle A\n"; }
void handleB(Base& b) { std::cout << "Handle B\n"; }

int main() {
    std::unordered_map<std::type_index, Handler> dispatch {
        { typeid(A), &handleA },
        { typeid(B), &handleB }
    };

    A a; B b;
    Base& r1 = a; Base& r2 = b;

    dispatch.at(typeid(r1))(r1);
    dispatch.at(typeid(r2))(r2);
}
```

#### 📄 Case 04 any_variant
```cpp
#include <any>
#include <variant>
#include <iostream>
#include <string>

void printAny(const std::any& v) {
    if (v.type() == typeid(int))         std::cout << "int: " << std::any_cast<int>(v) << "\n";
    else if (v.type() == typeid(double)) std::cout << "double: " << std::any_cast<double>(v) << "\n";
    else if (v.type() == typeid(std::string)) std::cout << "string: " << std::any_cast<std::string>(v) << "\n";
    else std::cout << "unknown type\n";
}

int main() {
    std::any a = 42;
    printAny(a);
    a = std::string("hello");
    printAny(a);

    std::variant<int, double, std::string> v = 3.14;
    std::visit([](auto&& x){ std::cout << "variant holds: " << x << "\n"; }, v);
    v = std::string("world");
    std::visit([](auto&& x){ std::cout << "variant holds: " << x << "\n"; }, v);
}
```

#### 📄 Case 05 manual_tagged_variant
```cpp
#include <iostream>
#include <memory>
#include <string>

// RTTI를 쓰지 않고 '태그 + 방문자'로 처리하는 패턴
struct Shape {
    enum class Kind { Circle, Rect } kind;
    explicit Shape(Kind k): kind(k) {}
    virtual ~Shape() = default; // 다형성 사용해도 되고, 안 써도 됨
};

struct Circle : Shape { double r; Circle(double r): Shape(Kind::Circle), r(r) {} };
struct Rect   : Shape { double w,h; Rect(double w,double h): Shape(Kind::Rect), w(w), h(h) {} };

double area(const Shape& s) {
    switch (s.kind) {
    case Shape::Kind::Circle: return 3.141592653589793 * static_cast<const Circle&>(s).r * static_cast<const Circle&>(s).r;
    case Shape::Kind::Rect:   return static_cast<const Rect&>(s).w * static_cast<const Rect&>(s).h;
    }
    return 0.0;
}

int main() {
    Circle c{2.0};
    Rect   r{3.0, 4.0};
    std::cout << "A(circle) = " << area(c) << "\n";
    std::cout << "A(rect)   = " << area(r) << "\n";
}
```

### 📌 6) 언제 무엇을 쓰나?
- 다형 계층에서 안전 다운캐스팅 필요 → dynamic_cast
- 런타임 타입 기반 디스패치 → typeid + std::type_index 맵, 또는 std::variant + std::visit
- 정말 다양한 타입 수용(플러그인/스크립팅 등) → std::any (단, 사용 지점에서 캐스팅 필요)
- 성능 민감이고 타입이 제한적 → std::variant가 보통 더 빠르고 안전

### 📌 7) 흔한 함정 & 권장사항
- 베이스 클래스에는 반드시 가상 소멸자를 두자 (다형 삭제 안전성).
- dynamic_cast 남발은 설계 신호일 수 있음 → 방문자 패턴, 가상 함수 오버라이드로 해결 가능한지 먼저 검토.
- std::any는 남발 시 “원시형”으로 회귀 → API 경계에서 최소화하고, 내부에서는 구체 타입/variant를 선호.
- RTTI 비활성 빌드를 고려해야 하는 프로젝트(임베디드 등)라면, 태그드 유니온 / variant / 가상함수로 대체 설계.

---
