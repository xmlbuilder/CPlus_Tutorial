## 📘 Virtual 함수 초기화 문제

- 이 문제는 초기화 순서 문제입니다.  
- 특히 기초 클래스 생성자에서 가상 함수(virtual function)를 호출할 때 하위 클래스의 필드가 아직 초기화되지 않아  
예상치 못한 동작이 발생할 수 있음.  
- Rust는 상속이 없고 초기화가 명시적이라 이런 문제가 발생하지 않지만, C++에서는 반드시 회피 전략이 필요합니다.

### 📌 발생하는 문제: 생성자에서 virtual 함수 호출

#### 🔹 문제 예시
```cpp
#include <iostream>
using namespace std;

class Base {
public:
    Base() {
        init(); // virtual 함수 호출
    }

    virtual void init() {
        cout << "Base init" << endl;
    }
};

class Derived : public Base {
    string message;
public:
    Derived(const string& msg) : message(msg) {}

    void init() override {
        cout << "Derived init: " << message << endl; // message는 아직 초기화되지 않음
    }
};

int main() {
    Derived d("Hello");
}
```

#### 🔹 결과
- Base 생성자가 먼저 실행되며 `init()` 호출
- 하지만 이 시점엔 `Derived` 의 `message` 필드는 아직 초기화되지 않음
- `Derived::init()` 이 호출되지만 `message` 는 `빈 값` 또는 `쓰레기 값`

### 📌 회피 전략 1: 생성자에서 virtual 함수 호출 금지
#### 🔹개선 예시
```cpp
class Base {
public:
    Base() {}
    virtual void init() = 0; // 순수 가상 함수로 선언만
};

class Derived : public Base {
    string message;
public:
    Derived(const string& msg) : message(msg) {}
    void init() override {
        cout << "Derived init: " << message << endl;
    }
};

int main() {
    Derived d("Hello");
    d.init(); // 안전하게 호출
}
```
- 생성자 외부에서 init()을 호출하므로 message는 이미 초기화된 상태


### 📌 회피 전략 2: 정적 팩토리 함수 사용
#### 🔹 팩토리 패턴 예시
```cpp
class Derived : public Base {
    string message;
    Derived(const string& msg) : message(msg) {}
public:
    static Derived* create(const string& msg) {
        Derived* d = new Derived(msg);
        d->init(); // 안전하게 초기화
        return d;
    }
    void init() override {
        cout << "Derived init: " << message << endl;
    }
};

int main() {
    Derived* d = Derived::create("Hello");
    delete d;
}
```
- 생성자에서 init()을 호출하지 않고, 팩토리 함수에서 안전하게 호출

### 📌 초기화 순서 문제 및 회피 전략 요약

| 항목                     | 설명 |
|--------------------------|------|
| 문제 발생 조건           | 상위 클래스 생성자에서 virtual 함수 호출 시, 하위 클래스 필드가 아직 초기화되지 않아 예기치 않은 동작 발생 |
| 회피 전략 ①             | 생성자에서 virtual 함수 호출 금지 → 외부에서 명시적으로 초기화 메서드 호출 |
| 회피 전략 ②             | 팩토리 메서드 패턴 사용 → 객체 생성 후 안전하게 초기화 수행 |
| Rust와의 차이점          | Rust는 상속이 없고 초기화가 명시적이므로 해당 문제 자체가 발생하지 않음 |

- 이 문제는 C++에서 객체 지향 설계를 할 때 자주 발생하는 함정.
- 생성자에서 virtual 함수를 호출하는 건 피하고, 초기화는 외부에서 명시적으로 처리하는 게 안전합니다.


### 📌 스마트 포인터 와 팩토리 패턴 사용

- C++에서 스마트 포인터와 함께 사용하는 팩토리 패턴은 객체의 생성과 메모리 관리를 안전하게 통합하는 매우 강력한 방식.  
- 특히 std::unique_ptr이나 std::shared_ptr을 사용하면 new/delete 없이 자원 누수 없이 객체를 관리할 수 있음.

### 📌 왜 스마트 포인터 + 팩토리 패턴인가?

| 문제 또는 목적             | 해결 또는 이점                      |
|----------------------------|-------------------------------------|
| `new` / `delete` 직접 사용 → 메모리 누수 위험 | `std::unique_ptr`로 자동 소멸 관리 |
| 생성자에서 virtual 함수 호출 → 초기화 순서 문제 | 팩토리 함수에서 안전하게 초기화 수행 |
| 생성자 로직 분산 → 유지보수 어려움 | 팩토리 함수로 생성 책임 집중 |
| 예외 발생 시 자원 누수 가능성 | 스마트 포인터는 예외 안전성 보장 |
| 객체 소유권 명확히 하고 싶을 때 | `unique_ptr` → 단일 소유 / `shared_ptr` → 공유 소유 |


### 📌 예제: std::unique_ptr + 팩토리 패턴
```cpp
#include <iostream>
#include <memory>
#include <string>

class Base {
public:
    virtual void init() = 0;
    virtual ~Base() = default;
};

class Derived : public Base {
    std::string message;

    // 생성자는 private 또는 protected로 감싸기
    Derived(const std::string& msg) : message(msg) {}

public:
    static std::unique_ptr<Derived> create(const std::string& msg) {
        auto ptr = std::unique_ptr<Derived>(new Derived(msg));
        ptr->init(); // 안전한 초기화
        return ptr;
    }

    void init() override {
        std::cout << "Derived init: " << message << std::endl;
    }
};

int main() {
    auto obj = Derived::create("Hello, JungHwan!");
    // obj는 자동으로 소멸됨
}
```
- `std::unique_ptr<Derived>(new Derived(msg))` 는 new보다 효율적이고 예외 안전  

### 📌 예제: std::shared_ptr 사용 (공유 소유권 필요 시)
```cpp
class Derived : public Base {
    std::string message;

    Derived(const std::string& msg) : message(msg) {}

public:
    static std::shared_ptr<Derived> create(const std::string& msg) {
        auto ptr = std::make_shared<Derived>(msg);
        ptr->init();
        return ptr;
    }

    void init() override {
        std::cout << "Shared Derived init: " << message << std::endl;
    }
};
```
-  `std::make_shared<Derived>(msg)` 는 new보다 효율적이고 예외 안전


### 📌 스마트 포인터 + 팩토리 패턴 요약

| 항목                     | 설명 |
|--------------------------|------|
| 생성자에서 virtual 호출 회피 | 생성자 내부에서 virtual 함수 호출 금지, 팩토리에서 안전하게 호출 |
| 스마트 포인터 사용 이유     | `unique_ptr` → 단일 소유권 / `shared_ptr` → 공유 소유권 / 자동 소멸 |
| 팩토리 함수 역할          | 객체 생성 + 초기화 책임 집중 / 생성자 은닉 가능 / 안전한 초기화 보장 |

- 이 패턴은 특히 RAII(Resource Acquisition Is Initialization) 원칙과 잘 맞아서 안전하고 유지보수 쉬운 객체 생성을 구현할 수 있음.

---

