# 🎯 C++에서 스마트 포인터로 데코레이터 패턴 구현하기
데코레이터 패턴은 객체에 동적으로 기능을 추가할 수 있게 해주는 디자인 패턴입니다.  
상속보다 유연하고, 런타임에 기능을 조합할 수 있다는 장점이 있어요.
스마트 포인터(std::unique_ptr, std::shared_ptr)를 사용하면 메모리 관리까지 자동으로 처리되니, 데코레이터 구현에 딱입니다.

## 🧩 기본 구조
```cpp
#include <iostream>
#include <memory>

// 기본 컴포넌트 인터페이스
class Component {
public:
    virtual void operation() const = 0;
    virtual ~Component() = default;
};

// 실제 컴포넌트
class ConcreteComponent : public Component {
public:
    void operation() const override {
        std::cout << "ConcreteComponent operation\n";
    }
};

// 데코레이터 추상 클래스
class Decorator : public Component {
protected:
    std::unique_ptr<Component> component;
public:
    Decorator(std::unique_ptr<Component> comp) : component(std::move(comp)) {}
};

// 기능을 추가하는 데코레이터
class ConcreteDecoratorA : public Decorator {
public:
    ConcreteDecoratorA(std::unique_ptr<Component> comp) : Decorator(std::move(comp)) {}

    void operation() const override {
        component->operation(); // 기존 기능
        std::cout << "ConcreteDecoratorA added behavior\n"; // 추가 기능
    }
};

class ConcreteDecoratorB : public Decorator {
public:
    ConcreteDecoratorB(std::unique_ptr<Component> comp) : Decorator(std::move(comp)) {}

    void operation() const override {
        component->operation();
        std::cout << "ConcreteDecoratorB added behavior\n";
    }
};
```


## 🚀 사용 예시
```cpp
int main() {
    std::unique_ptr<Component> comp = std::make_unique<ConcreteComponent>();
    comp = std::make_unique<ConcreteDecoratorA>(std::move(comp));
    comp = std::make_unique<ConcreteDecoratorB>(std::move(comp));

    comp->operation();
    return 0;
}
```


출력 결과:
```
ConcreteComponent operation  
ConcreteDecoratorA added behavior  
ConcreteDecoratorB added behavior
```


## 💡 포인트 정리
- 스마트 포인터를 사용하면 new/delete 없이 안전하게 메모리 관리 가능
- std::move를 통해 소유권을 넘기며 데코레이터 체인을 구성
- 데코레이터는 기존 컴포넌트를 감싸고 기능을 확장



## 🔍 std::unique_ptr에서 std::move를 하지 않으면?
### 1. 컴파일 에러가 납니다.
std::unique_ptr는 복사 불가능한 스마트 포인터입니다. 즉, 복사 생성자와 복사 대입 연산자가 삭제되어 있어요. 그래서 다음과 같은 코드는 컴파일 에러를 일으킵니다:
std::unique_ptr<Component> a = std::make_unique<ConcreteComponent>();
std::unique_ptr<Component> b = a; // ❌ 컴파일 에러: 복사 불가


### 2. 왜 move가 필요한가요?
std::move(a)를 하면 a의 소유권이 b로 이전됩니다. 이후 a는 nullptr 상태가 되며, 더 이상 유효한 포인터가 아니에요.
std::unique_ptr<Component> b = std::move(a); // ✅ OK


이렇게 하면 b가 ConcreteComponent를 소유하게 되고, a는 비워집니다.

### 💣 메모리 누수는 발생하나요?
아니요, move를 하지 않아서 생기는 문제는 메모리 누수가 아니라 컴파일 에러입니다.
unique_ptr는 소유권을 명확히 하기 때문에, 복사를 허용하지 않음으로써 의도치 않은 이중 해제나 누수를 방지해주는 거죠.

## 🧠 요약
| 상황                                 | 결과 및 설명                                                                 |
|--------------------------------------|------------------------------------------------------------------------------|
| `unique_ptr`를 복사하려고 함         | ❌ 컴파일 에러 발생 (복사 생성자/대입 연산자 삭제됨)                         |
| `std::move`로 소유권 이전             | ✅ 정상 작동, 이전된 포인터는 `nullptr` 상태가 됨                            |
| `std::move` 없이 데코레이터 체인 구성 | ❌ 컴파일 에러 (소유권 이전이 안 되어 unique_ptr 생성 불가)                 |
| 메모리 누수 가능성                   | 없음 (컴파일 단계에서 막히므로 런타임 누수 발생하지 않음)                   |



## 🧼 가독성 개선 팁
### 1. 헬퍼 함수로 감싸기
```cpp
std::unique_ptr<Component> makeDecoratedComponent() {
    auto base = std::make_unique<ConcreteComponent>();
    auto decoA = std::make_unique<ConcreteDecoratorA>(std::move(base));
    auto decoB = std::make_unique<ConcreteDecoratorB>(std::move(decoA));
    return decoB;
}
```

사용할 땐 이렇게 간단하게:
```cpp
auto comp = makeDecoratedComponent();
comp->operation();
```

### 2. 팩토리 함수 + 함수 체이닝 스타일
C++에서는 함수 체이닝이 제한적이지만, 래퍼 클래스를 만들어서 이런 식으로도 가능해요:
```cpp
class ComponentBuilder {
    std::unique_ptr<Component> comp;
public:
    ComponentBuilder() : comp(std::make_unique<ConcreteComponent>()) {}

    ComponentBuilder& addA() {
        comp = std::make_unique<ConcreteDecoratorA>(std::move(comp));
        return *this;
    }

    ComponentBuilder& addB() {
        comp = std::make_unique<ConcreteDecoratorB>(std::move(comp));
        return *this;
    }

    std::unique_ptr<Component> build() {
        return std::move(comp);
    }
};
```

사용 예:
```cpp
auto comp = ComponentBuilder().addA().addB().build();
comp->operation();
```


## 🧠 요약: 장점 vs 단점
| 항목               | 장점                                                       | 단점                                                        |
|--------------------|------------------------------------------------------------|-------------------------------------------------------------|
| `unique_ptr` 사용  | - 자동 메모리 관리<br>- 소유권 명확<br>- delete 불필요     | - `std::move` 반복 필요<br>- 복사 불가로 체인 구성 복잡     |
| 데코레이터 패턴    | - 런타임 기능 확장 가능<br>- 유연한 구조 설계 가능         | - 클래스 수 증가<br>- 체인 구성 시 코드 길어짐               |
| 가독성 개선 방법   | - 헬퍼 함수로 감싸기<br>- 빌더 패턴 적용<br>- 래퍼 클래스 활용 | - 추상화로 디버깅 어려움<br>- 구현 복잡도 증가 가능성        |

---



