## 📘 타입 정보 확인하기
- typeid, std::is_base_of, instanceof 완전 정복

### 📌 1. typeid — 런타임 타입 확인

#### 🔹 개념
- 객체의 실제 타입을 런타임에 확인하는 연산자
- 다형성(virtual)이 적용된 클래스에서만 정확하게 작동

#### 🔹 사용 예
```cpp
Base* ptr = new DerivedA();
std::cout << typeid(*ptr).name(); // "class DerivedA"
std::cout << typeid(ptr).name();  // "class Base *"
```

#### 🔹✅ 주의
- typeid(ptr) → 포인터 자체의 타입
- typeid(*ptr) → 포인터가 가리키는 실제 객체 타입


### 📌 2. std::is_base_of<Base, Derived> 
— 컴파일 타임 상속 관계 확인

#### 🔹개념
- Derived가 Base로부터 상속받았는지 컴파일 타임에 확인
- true 또는 false 반환

#### 🔹사용 예
```cpp
std::cout << std::is_base_of<Base, DerivedA>::value; // true
std::cout << std::is_base_of<Base, Sample>::value;   // false
```


### 📌 3. instanceof 
— 템플릿 기반 상속 관계 확인 함수

#### 🔹 구현
```cpp
template<typename Base, typename T>
inline bool instanceof(const T*) {
    return std::is_base_of<Base, T>::value;
}
```

#### 🔹 사용 예
```cpp
class Object {
};

class Base : public Object {
public:
    virtual void vf(){}
};

class DerivedA : public Base {
public:
    bool b;
};

class DerivedB : public Base {
public:
    int c;
};

class Sample : public Object{
};


DerivedA* m4 = new DerivedA();
Base* m1 = m4;

std::cout << instanceof<Object>(m1);     // true
std::cout << instanceof<DerivedA>(m1);   // false
std::cout << instanceof<DerivedA>(m4);   // true
```

#### 🔹 주의
- T는 전달된 포인터 타입에 따라 추론됨
- Base* 타입이면 T = Base로 추론되므로 DerivedA와의 관계를 알 수 없음
- 정확한 결과를 얻으려면 정확한 타입의 포인터를 전달해야 함

#### 🔹 클래스 구조 예시
```cpp
class Object {};               // 최상위 부모
class Base : public Object {}; // Object 상속
class DerivedA : public Base {}; // Base 상속
class DerivedB : public Base {}; // Base 상속
class Sample : public Object {}; // Object 상속
```


#### 🔹 typeid를 활용한 런타임 타입 확인
```cpp
void memberPrint(Base *a){
    if (typeid(DerivedA) == typeid(*a)) {
        DerivedA* b = (DerivedA*)a;
        std::cout << typeid(b->b).name(); // bool
    }
    else if (typeid(DerivedB) == typeid(*a)) {
        DerivedB* b = (DerivedB*)a;
        std::cout << typeid(b->c).name(); // int
    }
}
```

- typeid(*a) → 실제 객체 타입 반환
- typeid(Base) == typeid(*a) → 다형성 여부 확인

#### 🔹 출력 결과 해석
```cpp
instanceof<Object>(m1) : 1   // DerivedA → Base → Object
instanceof<Base>(m1) : 1     // DerivedA → Base
instanceof<DerivedA>(m1) : 0 // m1은 Base* → T = Base
instanceof<DerivedA>(m4) : 1 // m4는 DerivedA* → T = DerivedA
```

- instanceof는 포인터 타입에 따라 추론되는 T가 달라지므로 결과가 달라질 수 있음
- m1이 실제로 DerivedA 객체를 가리키고 있다는 사실은 정적 타입 정보만으로는 알 수 없고, 다음 두 가지 방법 중 하나를 사용해야 합니다:

#### 🔹 1. typeid(*m1) — 런타임 타입 확인
```cpp
if (typeid(*m1) == typeid(DerivedA)) {
    std::cout << "m1은 실제로 DerivedA 객체를 가리킴" << std::endl;
}
```
- typeid(*m1)은 m1이 가리키는 실제 객체의 타입을 반환
- 이 비교는 Base 클래스에 virtual 함수가 있을 때만 정확하게 작동함
→ 즉, Base에 virtual ~Base() {} 또는 virtual void foo() {} 같은 멤버가 있어야 함

#### 🔹 2. dynamic_cast<DerivedA*>(m1) — 안전한 다운캐스팅
```cpp
DerivedA* derived = dynamic_cast<DerivedA*>(m1);
if (derived != nullptr) {
    std::cout << "m1은 DerivedA 객체를 가리킴" << std::endl;
}
```
- dynamic_cast는 런타임에 타입을 확인하며, 실패 시 nullptr 반환
- 역시 Base에 virtual 함수가 있어야 작동함

### 📌 요약
| 방법 | 필요 조건 | 결과 |
|------|---------|------| 
| `typeid(*m1)` | Base에 virtual 함수 | 정확한 타입 비교 가능 | 
| dynamic_cast<DerivedA*>(m1) | Base에 virtual 함수 | 안전한 다운캐스팅 가능 | 

---

### 📌 **추론의 핵심은 다형성(polymorphism)**
- Base* 가 실제로 DerivedA를 가리키는지 확인하려면, 런타임 타입 정보가 필요하고, 이를 위해선 Base가 다형성을 지원해야 함.



### 📌 typeid 출력 결과
```cpp
typeid(*a).name() : class DerivedA
(typeid(Base) == typeid(*a)) : 0
typeid(b->b).name() : bool
```
- typeid(*a) → 실제 객체 타입 (DerivedA)
- typeid(Base) == typeid(*a) → false (다형성으로 인해 타입 다름)
- typeid(b->b) → 멤버 변수 b의 타입 (bool)

### 📌 기능 요약
| 기능 | 설명 | 사용 예 |
|------|-----|--------| 
| typeid(obj) | 객체의 실제 타입 확인 | typeid(*ptr).name() | 
| std::is_base_of<Base, Derived> | 상속 관계 확인 (컴파일 타임) | is_base_of<Base, Derived>::value | 
| instanceof<Base>(ptr) | 템플릿으로 상속 여부 확인 | instanceof<Object>(m1) | 



### 📌 세 가지 비교 요약
| 기능 | 확인 시점 | 확인 대상 | 특징 | 
|------|---------|----------|-----|
| typeid | 런타임 | 실제 객체 타입 | 다형성 필요 | 
| is_base_of | 컴파일 타임 | 상속 관계 | 템플릿 기반 | 
| instanceof | 컴파일 타임 | 상속 관계 | 포인터 타입에 따라 결과 달라짐 | 



### 📌 Java vs C# vs C++ — 타입 추론 및 확인 비교
| 기능 | Java | C# | C++ |
|------|------|----|----| 
| 런타임 타입 확인 | instanceof | is | typeid (다형성 필요) | 
| 안전한 다운캐스팅 | (Derived)obj + instanceof | obj as Derived | dynamic_cast<Derived*> | 
| 자동 타입 추론 | var (Java 10+) | var | auto | 
| 리플렉션 | Class<?>, getClass() | Type, GetType() | 제한적 (typeid, RTTI) | 
| 타입 정보 유지 | 항상 유지 | 항상 유지 | RTTI 필요, virtual 필수 | 



### 📌 팁
- typeid는 **다형성(polymorphism)** 이 적용된 클래스에서만 정확한 타입을 반환함
- std::is_base_of는 정적 타입 검사이므로 런타임 객체 타입과는 다를 수 있음
- instanceof는 Java의 instanceof와 유사하지만, 정적 타입 기반이므로 런타임 객체 타입과는 다를 수 있음

---



