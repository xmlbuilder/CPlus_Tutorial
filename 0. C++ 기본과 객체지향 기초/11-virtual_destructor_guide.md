# 🧹 C++ 가상 소멸자(virtual destructor) 가이드

## 핵심 요약
- **기반(Base) 클래스 포인터/참조로 파생(Derived) 객체를 삭제**할 가능성이 있으면, **기반 클래스의 소멸자는 반드시 `virtual`** 이어야 합니다.
- 그렇지 않으면 `delete basePtr;`는 **정의되지 않은 동작(UB)** 이며, 보통 **파생 소멸자가 호출되지 않아 리소스 누수**가 발생합니다.

---

## 문제가 되는 코드 (UB 예시)

```cpp
#include <iostream>
struct Base {
    ~Base() { std::cout << "Base dtor\n"; } // ❌ virtual 아님
};
struct Derived : Base {
    ~Derived() { std::cout << "Derived dtor\n"; }
};

int main() {
    Base* p = new Derived;
    delete p;                 // UB: 보통 Base dtor만 호출 → 누수
}
```
전형적 출력:
```
Base dtor
```

---

## 올바른 패턴

```cpp
#include <iostream>
struct Base {
    virtual ~Base() { std::cout << "Base dtor\n"; }  // ✅ 반드시 virtual
};
struct Derived : Base {
    ~Derived() override { std::cout << "Derived dtor\n"; } // ✅ override 권장
};

int main() {
    Base* p = new Derived();
    delete p;  // Derived → Base 순서로 정상 호출
}
```
출력:
```
Derived dtor
Base dtor
```

---

## 언제 `virtual ~Base()`가 필요한가?
- 그 클래스가 **다형(polymorphic) 기반**으로 쓰일 가능성이 있거나(멤버 함수 중 하나라도 `virtual`이면 보통 포함),
- **기반 포인터/참조로 파생 객체를 가리켜**,
- 그 포인터로 **`delete`** 할 수 있다면  
→ **무조건 가상 소멸자**를 선언하세요.

> 반대로 **상속 금지 타입**이라면 `final`로 명시하고 가상 소멸자를 두지 않아도 됩니다.
> ```cpp
> struct NonVirtual final { ~NonVirtual() = default; };
> ```

---

## 스마트 포인터와의 관계

```cpp
#include <memory>
std::unique_ptr<Base> p = std::make_unique<Derived>();
// unique_ptr/shared_ptr의 기본 deleter는 'delete base*' 호출
// → Base의 소멸자가 virtual이 아니면 동일한 문제 발생
```
스마트 포인터를 사용해도 **Base 소멸자는 virtual이어야 안전**합니다.

---

## 추가 팁

- **호출 순서**: 파생 → 기반(자식부터 부모 순)으로 소멸자가 호출됩니다.
- **비용**: 가상 소멸자는 vptr 등 약간의 비용이 있지만 대부분 미미합니다.
- **순수 가상 소멸자**도 가능:
  ```cpp
  struct IFace {
      virtual ~IFace() = 0;  // 추상 클래스
  };
  inline IFace::~IFace() {}   // 반드시 정의 필요(링크 에러 방지)
  ```
- **예외**: 소멸자는 기본적으로 `noexcept(true)`가 바람직합니다. 예외가 퍼지면 `std::terminate`.

---

## 체크리스트
- [x] 기반 클래스에 다른 `virtual` 멤버가 있다 → `virtual ~Base() = default;`
- [x] 기반 포인터/참조로 파생을 가리켜 삭제한다 → `virtual ~Base()`
- [x] 파생 소멸자에는 `override`를 붙여 실수 방지
- [x] 상속 금지 타입은 `final`로 표시하고 가상 소멸자 생략

---

## 사용자의 코드 정리 (정답 버전)

```cpp
#include <iostream>
using namespace std;

class Base {
public:
    virtual ~Base() {
        cout << "Base Destructor called\n";
    }
};

class Derived : public Base {
public:
    ~Derived() override {
        cout << "Derived Destructor called\n";
    }
};

int main() {
    Base* p = new Derived();
    delete p;
    return 0;
}
// 출력:
// Derived Destructor called
// Base Destructor called
```
