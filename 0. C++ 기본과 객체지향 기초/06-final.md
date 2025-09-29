# C++11 final 키워드 정리

## 📌 개요
C++11에서는 상속을 제한하거나 가상 함수의 재정의를 금지하기 위해 final 키워드가 도입되었습니다. 이를 통해 클래스 설계를 더 명확하게 하고, 의도하지 않은 오버라이딩이나 상속을 방지할 수 있습니다.

## 🔐 final 키워드의 용도
| 사용 위치 | 효과 |
|-------------|---------------------------| 
| 클래스 선언 뒤 | 해당 클래스를 상속 불가로 지정 | 
| 가상 함수 선언 뒤 | 해당 함수를 재정의 불가로 지정 | 



## 🧠 클래스에 final 사용
```cpp
class Subclass final : public Base {
    // 이 클래스는 더 이상 상속할 수 없음
};
```

- Subclass를 기반으로 다른 클래스를 만들면 컴파일 에러 발생
```cpp
// ❌ 오류 발생: Subclass는 final로 선언되어 상속 불가
class Subclass2 : public Subclass {};
```



## 🔁 함수에 final 사용
```cpp
class Base {
public:
    virtual void foo() final {
        std::cout << "final method" << std::endl;
    }
};
```

- foo()는 파생 클래스에서 재정의 불가
- override와 함께 사용할 수 있지만, final만으로도 충분히 의미 전달 가능
```cpp
// ❌ 오류 발생: foo()는 final로 선언되어 재정의 불가
void foo() override {
    std::cout << "override attempt" << std::endl;
}
```


## 🧪 예제 코드 (수정 및 정리)
```cpp
#include <iostream>

class Base {
public:
    int value1;
    int value2;

    Base() {
        value1 = 1;
    }

    Base(int value) : Base() {
        value2 = value;
    }

    virtual void foo() final {
        std::cout << "final method" << std::endl;
    }
};

// 클래스 상속 금지
class Subclass final : public Base {
public:
    using Base::Base;
};

// ❌ 오류 발생: Subclass는 final로 선언되어 상속 불가
// class Subclass2 : public Subclass {};

int main() {
    Base b(2);
    std::cout << b.value1 << std::endl; // 1
    std::cout << b.value2 << std::endl; // 2

    Subclass s(3);
    std::cout << s.value1 << std::endl; // 1
    std::cout << s.value2 << std::endl; // 3

    s.foo(); // final method

    return 0;
}
```


## ⚠️ 주의사항
- final은 컴파일 타임에 오류를 발생시켜 설계 실수를 방지함
- final 함수는 override와 함께 사용 가능하지만, final만으로도 충분히 의미 전달됨
- 클래스에 final을 붙이면 모든 파생 클래스 생성이 금지됨

## 📌 요약
| 키워드 | 사용 위치 | 효과 | 
|-------------|---------------|------------------| 
| final | 클래스 선언 뒤 | 해당 클래스를 상속 불가로 만듦 | 
| final | 가상 함수 선언 뒤 | 해당 함수를 재정의 불가로 만듦 | 

---



