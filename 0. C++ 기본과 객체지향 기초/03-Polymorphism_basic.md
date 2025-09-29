# 🧬 C++ 다형성 (Polymorphism) 정리

## 📌 개요
다형성은 동일한 인터페이스를 통해 서로 다른 동작을 수행할 수 있게 하는 객체지향 프로그래밍의 핵심 개념입니다.  
C++에서는 정적 바인딩과 동적 바인딩을 통해 다형성을 구현합니다.  

## 🧠 다형성의 종류

### 🔹 정적 바인딩 (Compile-time)
    - 함수 오버로딩: 동일한 함수 이름에 대해 매개변수 타입/개수에 따라 다른 함수 호출
    - 연산자 오버로딩: 사용자 정의 타입에 대해 연산자 동작을 정의
    - ✅ 컴파일 시점에 어떤 함수가 호출될지 결정됨

### 🔸 동적 바인딩 (Run-time)
    - 함수 오버라이딩: 상속 관계에서 자식 클래스가 부모 클래스의 가상 함수를 재정의
    - 가상 함수(Virtual Function): 런타임에 실제 객체 타입에 따라 함수 호출 결정
    - ✅ 실행 시점에 어떤 함수가 호출될지 결정됨
    - ✅ 추상화된 프로그래밍을 가능하게 함

## 🧪 런타임 다형성을 위한 조건
| 조건 | 설명 |
|-------------|------------------------------------------|  
| 상속 | 부모 클래스로부터 자식 클래스가 기능을 확장 | 
| 가상 함수 선언 | virtual 키워드를 통해 오버라이딩 가능하게 함 | 
| 기본 클래스의 참조 또는 포인터 사용 | 다형성을 활용하려면 부모 타입으로 접근해야 함 | 


## 🧾 예제 코드 분석
```cpp
class First {
public:
    void firstFunc() {
        std::cout << "firstFunc called" << std::endl;
    }
};

class Second : public First {
public:
    void secondFunc() {
        std::cout << "secondFunc called" << std::endl;
    }
};

class Third : public Second {
public:
    void thirdFunc() {
        std::cout << "thirdFunc called" << std::endl;
    }
    virtual ~Third() {
        std::cout << "third destructor called" << std::endl;
    }
};
```

## ▶️ 포인터 형 변환과 함수 호출
```cpp
std::unique_ptr<Third> third(new Third());
Second* second = reinterpret_cast<Second*>(third.get());
First* first = reinterpret_cast<First*>(third.get());

third->firstFunc();   // OK
third->secondFunc();  // OK
third->thirdFunc();   // OK

second->firstFunc();  // OK
second->secondFunc(); // OK
// second->thirdFunc(); // ❌ 컴파일 에러

first->firstFunc();   // OK
// first->secondFunc(); // ❌ 컴파일 에러
// first->thirdFunc();  // ❌ 컴파일 에러
```

## 🧾 출력 결과
```cpp
firstFunc called
secondFunc called
thirdFunc called
firstFunc called
secondFunc called
firstFunc called
third destructor called
```

## ⚠️ 주의사항
    - 포인터의 자료형 기준으로 호출 가능한 함수가 결정됨
    - 실제 객체의 타입이 Third여도, First*로 접근하면 First의 멤버 함수만 호출 가능
    - virtual 키워드가 없으면 동적 바인딩이 일어나지 않음
    - 소멸자에 virtual을 붙이지 않으면 자식 객체의 소멸자가 호출되지 않을 수 있음

## 📌 요약
| 항목 | 정적 바인딩 | 동적 바인딩 |
|-------------|-------------|------------|  
| 결정 시점 | 컴파일 타임 | 런타임 | 
| 대표 예시 | 함수 오버로딩, 연산자 오버로딩 | 함수 오버라이딩, 가상 함수 | 
| 키워드 필요 여부 | 없음 | virtual 필요 | 
| 유연성 | 낮음 | 높음 | 
| 성능 | 빠름 | 느릴 수 있음 | 

---


