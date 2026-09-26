## C++ 순수 가상 함수, 추상 클래스, 인터페이스 클래스 정리

### 📌 순수 가상 함수 (pure virtual function)
- 형식: virtual 함수명() = 0;
- 목적: 파생 클래스에서 반드시 해당 함수를 **오버라이드(재정의)** 하도록 강제
- 특징:
    - 함수의 구현이 없음
    - 선언만 존재하며 `= 0` 으로 표시
    - 해당 함수가 하나라도 있으면 클래스는 추상 클래스가 됨

```cpp
class Shape {
public:
    virtual void draw() = 0; // 순수 가상 함수
    virtual ~Shape() {}      // 가상 소멸자
};
```


### 📌 추상 클래스 (abstract class)
- 정의: 하나 이상의 순수 가상 함수를 포함한 클래스
- 특징:
    - 객체 생성 불가
    - 파생 클래스에서 순수 가상 함수를 모두 구현하지 않으면, 그 클래스도 추상 클래스가 됨
    - **다형성(polymorphism)** 을 활용하기 위한 기반 클래스 역할

```cpp
class Circle : public Shape {
public:
    void draw() override {
        std::cout << "draw called" << std::endl;
    }
    ~Circle() override {
        std::cout << "destructor called" << std::endl;
    }
};
```


### 📌 예제 코드
```cpp
int main() {
    // Shape* shape = new Shape(); // ❌ 컴파일 에러: 추상 클래스는 객체 생성 불가

    Circle* circle = new Circle();
    Shape* shape = static_cast<Shape*>(circle); // 업캐스팅

    shape->draw();   // draw called
    delete shape;    // destructor called
}
```

### 📌 동작 설명
- Shape는 추상 클래스이므로 직접 `객체 생성 불가`
- Circle은 Shape를 상속받아 draw()를 구현했기 때문에 객체 생성 가능
- Shape* 포인터를 통해 Circle의 draw()가 동적 바인딩되어 호출됨
- `소멸자도 virtual` 이므로 Circle의 소멸자가 제대로 호출됨

### 🧩 인터페이스 클래스 (interface class)
- 정의: 모든 멤버 함수가 순수 가상 함수인 추상 클래스
- 목적: 기능의 껍데기만 정의하고, 실제 구현은 파생 클래스에서 수행
- 특징:
    - C++에는 interface 키워드가 없고, 추상 클래스로 인터페이스를 구현
    - 일반적으로 클래스 이름 앞에 I를 붙여 인터페이스임을 명시

```cpp
class IShape {
public:
    virtual void draw() = 0;
    virtual void rotate() = 0;
    virtual ~IShape() {}
};
```


### 📌 인터페이스 클래스의 장점
- 구현과 인터페이스 분리: 기능 정의와 실제 구현을 분리하여 유연한 설계 가능
- 다형성 활용: 다양한 구현체를 동일한 인터페이스로 다룰 수 있음
- 유지보수 용이: 기능 변경 시 인터페이스만 유지하면 구현체 교체가 쉬움

### 📌 요약

| 항목 | 설명 | 
|------------------|-------------------------------------------|
| 순수 가상 함수 | `virtual 함수명() = 0;` 형식, 반드시 오버라이드 필요 | 
| 추상 클래스 | 순수 가상 함수 포함, 객체 생성 불가 | 
| 인터페이스 클래스 | 모든 함수가 순수 가상 함수인 추상 클래스 | 
| 다형성 | 추상 클래스 포인터/참조로 파생 클래스의 함수 동적 호출 가능 | 
| 소멸자 | 반드시 virtual로 선언해야 자식 소멸자 호출됨 | 

---




