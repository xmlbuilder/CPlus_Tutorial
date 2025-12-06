# Strategy Pattern

**전략 패턴(Strategy Pattern)** 을  C++ 로 구현한 예제.


## 🧠 전략 패턴 핵심 구조
- Strategy: 알고리즘 인터페이스 (trait / interface / abstract class)
- ConcreteStrategy: 실제 알고리즘 구현체
- Context: 전략을 사용하는 클래스, 전략을 교체 가능


### 💠 C++ 예제 (unique_ptr 기반)
```cpp
#include <iostream>
#include <memory>

class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};

class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};

class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};

class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context(std::unique_ptr<Strategy> s) : strategy(std::move(s)) {}

    void setStrategy(std::unique_ptr<Strategy> s) {
        strategy = std::move(s);
    }

    void execute() {
        strategy->execute();
    }
};

int main() {
    Context context(std::make_unique<StrategyA>());
    context.execute();

    context.setStrategy(std::make_unique<StrategyB>());
    context.execute();

    return 0;
}
```
