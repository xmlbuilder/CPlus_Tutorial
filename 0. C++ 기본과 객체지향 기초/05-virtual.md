# 🧾 C++ override 지정자와 가상 함수 정리

## 📌 개요
C에서 **다형성(polymorphism)** 을 구현하기 위해 사용하는 핵심 기능 중 하나가 **가상 함수(virtual function)** 입니다.  
C11부터는 가상 함수 재정의 시 실수를 방지하기 위해 override 지정자를 사용할 수 있게 되었습니다.

## 🧠 virtual function
- 멤버 함수 호출 시 포인터 타입이 아닌 실제 객체 타입에 따라 함수가 호출됨
- 실행 시간에 포인터가 가리키는 객체의 메모리를 조사하여 적절한 함수 호출
- 선언과 구현을 분리할 경우, 선언부에만 virtual 키워드를 붙이면 됨

```cpp
class Base {
public:
    virtual void say_hello() const {
        std::cout << "Hello Base" << std::endl;
    }
};
```



## 🔁 function override
- 기반 클래스의 가상 함수를 파생 클래스에서 재정의(override) 가능
- virtual 키워드는 붙여도 되고 생략해도 되지만, 실수를 방지하기 위해 override 사용 권장
- override는 C++11부터 도입된 지정자
- 선언과 구현을 분리할 경우, 선언부에만 override 키워드를 붙이면 됨

```cpp
class Derived : public Base {
public:
    void say_hello() const override {
        std::cout << "Hello Derived" << std::endl;
    }
};
```

- ✅ override를 사용하면 다음과 같은 실수를 컴파일 타임에 잡아낼 수 있음:  
// 오타로 인해 오버라이딩이 되지 않음 → 컴파일 에러 발생

```cpp 
void say_helloo() const override; // ❌
```


## 🧨 virtual destructor

- Upcasting(기반 클래스 포인터로 파생 클래스 객체를 참조) 시,
기반 클래스의 소멸자는 반드시 가상 소멸자여야 함
- 그렇지 않으면 파생 클래스의 소멸자가 호출되지 않아 자원 누수 발생 가능

```cpp 
class Base {
public:
    virtual ~Base() = default; // 가상 소멸자
};

class Derived : public Base {
public:
    ~Derived() override = default;
};
```


## 🧪 예제 코드

```cpp 
#include <iostream>

class Base {
public:
    virtual void say_hello() const {
        std::cout << "Hello Base" << std::endl;
    }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void say_hello() const override {
        std::cout << "Hello Derived" << std::endl;
    }
    ~Derived() override = default;
};

int main() {
    Base* obj = new Derived();
    obj->say_hello(); // Hello Derived
    delete obj;       // Derived 소멸자 호출됨
}
```


## 📌 요약

| 항목 | 설명 |
|-------------|------------------------------------------|   
| virtual | 런타임 다형성을 위한 함수 선언 키워드 | 
| override | 가상 함수 재정의 시 실수를 방지하는 C++11 지정자 | 
| virtual destructor | Upcasting 시 자식 소멸자를 호출하기 위해 반드시 필요 | 
| 선언 위치 | virtual, override는 선언부에만 표기 | 
| 실수 방지 | override를 사용하면 오버라이딩 실패를 컴파일 타임에 확인 가능 | 

