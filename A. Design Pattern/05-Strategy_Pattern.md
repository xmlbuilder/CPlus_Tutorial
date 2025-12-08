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
```
```cpp
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
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
```
```cpp
int main() {
    Context context(std::make_unique<StrategyA>());
    context.execute();

    context.setStrategy(std::make_unique<StrategyB>());
    context.execute();

    return 0;
}
```
---

## 📌 스마트 포인터 은닉화된 전략 패턴
```cpp
#include <iostream>
#include <memory>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
// Context 클래스
class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context() = default;

    // 전략 설정을 템플릿으로 은닉화
    template <typename T, typename... Args>
    void setStrategy(Args&&... args) {
        strategy = std::make_unique<T>(std::forward<Args>(args)...);
    }

    void execute() {
        if (strategy) {
            strategy->execute();
        } else {
            std::cout << "No strategy set!\n";
        }
    }
};
```
```cpp
// main 함수
int main() {
    Context context;
    // 스마트 포인터를 직접 다루지 않고 전략 교체
    context.setStrategy<StrategyA>();
    context.execute();
    context.setStrategy<StrategyB>();
    context.execute();
    return 0;
}
```
## 📊 개선된 점
- 호출부 단순화: context.setStrategy<StrategyA>()처럼 타입만 지정하면 됨.
- 스마트 포인터 은닉화: 내부에서 std::make_unique와 std::move 처리 → 호출자는 신경 쓸 필요 없음.
- 유연성: 생성자 인자가 필요한 전략도 context.setStrategy<StrategyX>(args...)로 전달 가능.

## ✅ 요약
- 전략 패턴에서도 스마트 포인터를 Context 내부에서 은닉화했으니, 호출자는 단순히 **전략을 교체한다”**  개념만 신경 쓰면 됩니다.

---

- Fluent DSL 스타일로 전략 패턴을 구현하면, 호출부가 context.use<StrategyA>().execute();처럼 훨씬 직관적으로 읽히게 됩니다. 
- 내부에서는 여전히 std::unique_ptr로 자원을 관리하지만, 호출자는 전혀 신경 쓸 필요가 없음.

## 📌 Fluent DSL 스타일 전략 패턴
```cpp
#include <iostream>
#include <memory>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
// Context 클래스 (Fluent DSL 스타일)
class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context() = default;

    // DSL 메서드: use<T>(args...)
    template <typename T, typename... Args>
    Context& use(Args&&... args) {
        strategy = std::make_unique<T>(std::forward<Args>(args)...);
        return *this; // 자기 자신 반환 → 체인 가능
    }

    Context& execute() {
        if (strategy) {
            strategy->execute();
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this; // 체인식 호출 가능
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    // DSL 스타일로 전략 교체 및 실행
    context.use<StrategyA>().execute()
           .use<StrategyB>().execute();

    return 0;
}
```


## 📊 장점
- 스마트 포인터 은닉화: 호출부에서는 std::make_unique나 std::move를 전혀 보지 않음.
- 체인식 API: use<>()와 execute()를 자연스럽게 이어서 호출 가능.
- 가독성↑: “전략을 선택하고 실행한다”는 의도가 코드에 그대로 드러남.
- 유연성: 생성자 인자가 필요한 전략도 context.use<StrategyX>(args...)로 전달 가능.

## ✅ 요약
- 이제 전략 패턴을 Fluent DSL 스타일로 구현했으니, 호출자는 단순히 **전략을 선택하고 실행한다** 는 개념만 신경 쓰면 됩니다.

---


## 📌 조건부 실행 기능 추가한 Fluent DSL 전략 패턴
```cpp
#include <iostream>
#include <memory>
#include <string>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
// Context 클래스 (Fluent DSL + 조건부 실행)
class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context() = default;
    // DSL 메서드: use<T>(args...)
    template <typename T, typename... Args>
    Context& use(Args&&... args) {
        strategy = std::make_unique<T>(std::forward<Args>(args)...);
        return *this; // 체인 가능
    }

    // 기본 실행
    Context& execute() {
        if (strategy) {
            strategy->execute();
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }

    // 조건부 실행
    Context& executeIf(bool condition) {
        if (condition && strategy) {
            strategy->execute();
        } else if (!condition) {
            std::cout << "Condition not met, skipping strategy.\n";
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool systemReady = true;
    bool errorDetected = false;

    // DSL 스타일 + 조건부 실행
    context.use<StrategyA>().executeIf(systemReady)   // 조건 만족 → 실행
           .use<StrategyB>().executeIf(errorDetected); // 조건 불만족 → 스킵
    return 0;
}
```

## 📊 실행 결과 (예상)
```
Executing Strategy A
Condition not met, skipping strategy.
```
## ✅ 요약
- executeIf(condition) 메서드를 추가해 조건부 실행 지원.
- 호출부는 context.use<StrategyX>().executeIf(flag)처럼 직관적으로 작성.
- DSL 스타일과 결합해 **전략 선택 → 조건부 실행** 흐름을 자연스럽게 표현 가능.

---

## 📌 람다 조건 지원 Fluent DSL 전략 패턴
```cpp
#include <iostream>
#include <memory>
#include <functional>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
// Context 클래스 (Fluent DSL + 조건부 실행 + 람다 지원)
class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context() = default;

    // DSL 메서드: use<T>(args...)
    template <typename T, typename... Args>
    Context& use(Args&&... args) {
        strategy = std::make_unique<T>(std::forward<Args>(args)...);
        return *this; // 체인 가능
    }

    // 기본 실행
    Context& execute() {
        if (strategy) {
            strategy->execute();
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }

    // 조건부 실행 (bool)
    Context& executeIf(bool condition) {
        if (condition && strategy) {
            strategy->execute();
        } else if (!condition) {
            std::cout << "Condition not met, skipping strategy.\n";
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }

    // 조건부 실행 (람다/함수)
    Context& executeIf(const std::function<bool()>& conditionFn) {
        if (conditionFn && conditionFn() && strategy) {
            strategy->execute();
        } else if (conditionFn && !conditionFn()) {
            std::cout << "Lambda condition not met, skipping strategy.\n";
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool systemReady = true;
    bool errorDetected = false;

    auto someCheck = []() {
        return true; // 조건 검사 로직
    };

    // DSL 스타일 + 조건부 실행 + 람다 조건
    context.use<StrategyA>().executeIf(systemReady)   // bool 조건
           .use<StrategyB>().executeIf(errorDetected) // bool 조건
           .use<StrategyA>().executeIf(someCheck);    // 람다 조건

    return 0;
}
```

## 📊 실행 결과 (예상)
```
Executing Strategy A
Condition not met, skipping strategy.
Executing Strategy A
```

## ✅ 요약
- executeIf(bool) → 단순 조건.
- executeIf(lambda) → 동적 조건 검사.
- 호출부는 context.use<StrategyX>().executeIf([]{ return someCheck(); });처럼 DSL로 표현 가능.
- 스마트 포인터는 내부에서 은닉화 → 호출자는 전략 교체와 조건만 신경 쓰면 됨.


---

## 📌 여러 조건 체인 실행 지원 Fluent DSL 전략 패턴

- 여러 조건을 체인으로 묶어서 실행하는 버전을 만들어 보겠습니다.  
- 핵심은 executeIf를 체인식으로 호출할 수 있도록 Context&를 반환하고, 각 조건을 독립적으로 평가하도록 하는 겁니다.  
- 이렇게 하면 context.use<StrategyA>().executeIf(cond1).executeIf(cond2); 같은 DSL이 가능합니다.

```cpp
#include <iostream>
#include <memory>
#include <functional>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
// Context 클래스 (Fluent DSL + 조건부 실행 체인)
class Context {
    std::unique_ptr<Strategy> strategy;

public:
    Context() = default;

    // DSL 메서드: use<T>(args...)
    template <typename T, typename... Args>
    Context& use(Args&&... args) {
        strategy = std::make_unique<T>(std::forward<Args>(args)...);
        return *this; // 체인 가능
    }

    // 기본 실행
    Context& execute() {
        if (strategy) {
            strategy->execute();
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this;
    }

    // 조건부 실행 (bool)
    Context& executeIf(bool condition) {
        if (condition && strategy) {
            strategy->execute();
        } else if (!condition) {
            std::cout << "Condition not met, skipping strategy.\n";
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this; // 체인 가능
    }

    // 조건부 실행 (람다/함수)
    Context& executeIf(const std::function<bool()>& conditionFn) {
        if (conditionFn && conditionFn() && strategy) {
            strategy->execute();
        } else if (conditionFn && !conditionFn()) {
            std::cout << "Lambda condition not met, skipping strategy.\n";
        } else {
            std::cout << "No strategy set!\n";
        }
        return *this; // 체인 가능
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool cond1 = true;
    bool cond2 = false;

    auto lambdaCond = []() {
        return true; // 조건 검사 로직
    };

    // DSL 스타일 + 여러 조건 체인 실행
    context.use<StrategyA>()
           .executeIf(cond1)        // true → 실행
           .executeIf(cond2)        // false → 스킵
           .executeIf(lambdaCond);  // true → 실행

    context.use<StrategyB>()
           .executeIf([] { return false; }) // 람다 조건 false → 스킵
           .executeIf([] { return true; }); // 람다 조건 true → 실행

    return 0;
}
```

## 📊 실행 결과
```
Executing Strategy A
Condition not met, skipping strategy.
Executing Strategy A
Lambda condition not met, skipping strategy.
Executing Strategy B
```


## ✅ 요약
- executeIf(bool)과 executeIf(lambda) 모두 체인식 호출 가능.
- 여러 조건을 이어 붙여서 조건부 실행 흐름을 자연스럽게 표현할 수 있음.
- 스마트 포인터는 내부에서 은닉화 → 호출자는 전략 교체와 조건만 신경 쓰면 됨.

---

## 📌 여러 전략 등록 + 조건부 선택 실행 버전
```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <typeindex>
#include <unordered_map>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp
// Context 클래스 (여러 전략 등록 + 조건부 실행)
class Context {
    std::unordered_map<std::type_index, std::unique_ptr<Strategy>> strategies;

public:
    Context() = default;

    // 전략 등록
    template <typename T, typename... Args>
    Context& add(Args&&... args) {
        strategies[typeid(T)] = std::make_unique<T>(std::forward<Args>(args)...);
        return *this;
    }

    // 특정 전략 실행
    template <typename T>
    Context& execute() {
        auto it = strategies.find(typeid(T));
        if (it != strategies.end() && it->second) {
            it->second->execute();
        } else {
            std::cout << "Strategy not found!\n";
        }
        return *this;
    }

    // 조건부 실행 (bool)
    template <typename T>
    Context& executeIf(bool condition) {
        if (condition) {
            execute<T>();
        } else {
            std::cout << "Condition not met, skipping strategy.\n";
        }
        return *this;
    }

    // 조건부 실행 (람다/함수)
    template <typename T>
    Context& executeIf(const std::function<bool()>& conditionFn) {
        if (conditionFn && conditionFn()) {
            execute<T>();
        } else {
            std::cout << "Lambda condition not met, skipping strategy.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool cond1 = true;
    bool cond2 = false;

    auto lambdaCond = []() {
        return true; // 조건 검사 로직
    };

    // 여러 전략 등록
    context.add<StrategyA>()
           .add<StrategyB>()
           .add<StrategyC>();

    // 조건에 따라 선택 실행
    context.executeIf<StrategyA>(cond1)       // true → 실행
           .executeIf<StrategyB>(cond2)       // false → 스킵
           .executeIf<StrategyC>(lambdaCond); // true → 실행

    return 0;
}
```


## 📊 실행 결과 (예상)
```
Executing Strategy A
Condition not met, skipping strategy.
Executing Strategy C
```


## ✅ 요약
- Context가 여러 전략을 unordered_map에 보관.
- add<T>()로 전략을 등록하고, execute<T>()로 특정 전략 실행.
- executeIf<T>(condition)으로 조건부 실행 지원.
- 호출부는 context.add<StrategyA>().add<StrategyB>().executeIf<StrategyA>(cond1)처럼 DSL 스타일로 작성 가능.


---
## 📌 우선순위 기반 실행 전략 패턴

- 여러 전략을 동시에 등록하고, 조건을 만족하는 전략 중 가장 높은 우선순위를 실행하는 버전을 만들어 보겠습니다.  
- 핵심은 Context가 전략을 우선순위와 함께 저장하고, 실행 시 조건을 평가한 뒤 최고 우선순위 전략만 실행하도록 하는 것입니다.

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp
// Context 클래스 (우선순위 기반 실행)
class Context {
    struct Entry {
        int priority;
        std::unique_ptr<Strategy> strategy;
        std::function<bool()> condition;
    };

    std::vector<Entry> strategies;

public:
    Context() = default;

    // 전략 등록 (우선순위 + 조건)
    template <typename T, typename... Args>
    Context& add(int priority, std::function<bool()> condition, Args&&... args) {
        strategies.push_back({
            priority,
            std::make_unique<T>(std::forward<Args>(args)...),
            condition
        });
        return *this;
    }

    // 조건 만족하는 전략 중 가장 높은 우선순위 실행
    Context& execute() {
        // 조건 만족하는 전략만 필터링
        std::vector<Entry*> valid;
        for (auto& e : strategies) {
            if (e.condition && e.condition()) {
                valid.push_back(&e);
            }
        }

        if (!valid.empty()) {
            // 우선순위 높은 전략 선택
            auto best = *std::max_element(valid.begin(), valid.end(),
                [](Entry* a, Entry* b) {
                    return a->priority < b->priority;
                });
            best->strategy->execute();
        } else {
            std::cout << "No valid strategy to execute.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool condA = true;
    bool condB = true;
    bool condC = false;

    // 여러 전략 등록 (우선순위와 조건 함께 지정)
    context.add<StrategyA>(1, [&]{ return condA; })
           .add<StrategyB>(3, [&]{ return condB; }) // 우선순위 가장 높음
           .add<StrategyC>(2, [&]{ return condC; });

    // 실행 → 조건 만족하는 전략 중 우선순위 가장 높은 StrategyB 실행
    context.execute();

    return 0;
}
```

## 📊 실행 결과 (예상)
```
Executing Strategy B
```


## ✅ 요약
- add<T>(priority, condition)으로 전략 등록 시 우선순위와 조건을 함께 지정.
- execute()는 조건을 만족하는 전략 중 가장 높은 우선순위 전략만 실행.
- 호출부는 context.add<StrategyA>(1, condFn).add<StrategyB>(3, condFn).execute();처럼 DSL 스타일로 작성 가능.

---
## 📌 조건 만족 전략을 우선순위 순으로 모두 실행하는 버전

- 조건을 만족하는 전략을 우선순위 순으로 모두 실행하는 버전을 만들어 보겠습니다.
- 앞서 만든 우선순위 기반 실행은 **최고 우선순위 하나만 실행** 이었는데, 이번에는 조건을 만족하는 전략들을 우선순위 내림차순으로 정렬해서 전부 실행하도록 확장합니다.

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp
// Context 클래스 (조건 만족 전략을 우선순위 순으로 모두 실행)
class Context {
    struct Entry {
        int priority;
        std::unique_ptr<Strategy> strategy;
        std::function<bool()> condition;
    };

    std::vector<Entry> strategies;

public:
    Context() = default;

    // 전략 등록 (우선순위 + 조건)
    template <typename T, typename... Args>
    Context& add(int priority, std::function<bool()> condition, Args&&... args) {
        strategies.push_back({
            priority,
            std::make_unique<T>(std::forward<Args>(args)...),
            condition
        });
        return *this;
    }

    // 조건 만족하는 전략을 우선순위 순으로 모두 실행
    Context& executeAll() {
        // 조건 만족하는 전략만 필터링
        std::vector<Entry*> valid;
        for (auto& e : strategies) {
            if (e.condition && e.condition()) {
                valid.push_back(&e);
            }
        }

        if (!valid.empty()) {
            // 우선순위 내림차순 정렬
            std::sort(valid.begin(), valid.end(),
                [](Entry* a, Entry* b) {
                    return a->priority > b->priority;
                });

            // 모두 실행
            for (auto* e : valid) {
                e->strategy->execute();
            }
        } else {
            std::cout << "No valid strategies to execute.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool condA = true;
    bool condB = true;
    bool condC = false;

    // 여러 전략 등록 (우선순위와 조건 함께 지정)
    context.add<StrategyA>(1, [&]{ return condA; })
           .add<StrategyB>(3, [&]{ return condB; })
           .add<StrategyC>(2, [&]{ return condC; });

    // 실행 → 조건 만족하는 전략들을 우선순위 순으로 모두 실행
    context.executeAll();

    return 0;
}
```

## 📊 실행 결과 (예상)
```
Executing Strategy B
Executing Strategy A
```

## ✅ 요약
- add<T>(priority, condition)으로 전략 등록 시 우선순위와 조건을 함께 지정.
- executeAll()은 조건을 만족하는 전략들을 우선순위 내림차순으로 정렬 후 모두 실행.
- 호출부는 context.add<StrategyA>(1, condFn).add<StrategyB>(3, condFn).executeAll();처럼 DSL 스타일로 작성 가능.

---

## 📌 Short-Circuit 버전 구현

- 이번에는 조건 만족 전략을 우선순위 순으로 실행하되, 중간에 특정 전략이 실행되면 나머지를 스킵하는 **단락(short-circuit)** 버전을 만들어 보겠습니다.  
- 핵심은 전략 엔트리에 **shortCircuit** 플래그를 두고, 해당 전략이 실행되면 이후 전략 실행을 중단하는 방식입니다.

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp              
// Context 클래스 (조건 만족 전략을 우선순위 순으로 실행 + Short-Circuit 지원)
class Context {
    struct Entry {
        int priority;
        bool shortCircuit; // 실행 후 단락 여부
        std::unique_ptr<Strategy> strategy;
        std::function<bool()> condition;
    };

    std::vector<Entry> strategies;

public:
    Context() = default;

    // 전략 등록 (우선순위 + 조건 + shortCircuit 여부)
    template <typename T, typename... Args>
    Context& add(int priority, bool shortCircuit, std::function<bool()> condition, Args&&... args) {
        strategies.push_back({
            priority,
            shortCircuit,
            std::make_unique<T>(std::forward<Args>(args)...),
            condition
        });
        return *this;
    }

    // 조건 만족하는 전략을 우선순위 순으로 실행 (shortCircuit 지원)
    Context& executeAll() {
        // 조건 만족하는 전략만 필터링
        std::vector<Entry*> valid;
        for (auto& e : strategies) {
            if (e.condition && e.condition()) {
                valid.push_back(&e);
            }
        }

        if (!valid.empty()) {
            // 우선순위 내림차순 정렬
            std::sort(valid.begin(), valid.end(),
                [](Entry* a, Entry* b) {
                    return a->priority > b->priority;
                });

            // 실행 (shortCircuit 체크)
            for (auto* e : valid) {
                e->strategy->execute();
                if (e->shortCircuit) {
                    std::cout << "Short-circuit triggered, skipping remaining strategies.\n";
                    break;
                }
            }
        } else {
            std::cout << "No valid strategies to execute.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool condA = true;
    bool condB = true;
    bool condC = true;

    // 여러 전략 등록 (우선순위와 조건, shortCircuit 여부 지정)
    context.add<StrategyA>(1, false, [&]{ return condA; })
           .add<StrategyB>(3, true,  [&]{ return condB; }) // 실행 후 단락
           .add<StrategyC>(2, false, [&]{ return condC; });

    // 실행 → StrategyB 실행 후 short-circuit 발생 → StrategyC는 스킵
    context.executeAll();

    return 0;
}
```
## 📊 실행 결과 (예상)
```
Executing Strategy B
Short-circuit triggered, skipping remaining strategies.
```

## ✅ 요약
- add<T>(priority, shortCircuit, condition)으로 전략 등록 시 우선순위, 조건, 단락 여부를 함께 지정.
- executeAll()은 조건 만족 전략을 우선순위 순으로 실행하다가, shortCircuit 전략이 실행되면 나머지 실행 중단.
- 호출부는 context.add<StrategyB>(3, true, condFn)처럼 DSL 스타일로 작성 가능.


## 📌 Short-Circuit + Fallback 메커니즘 버전

- 이번에는 short-circuit 발생 시 자동으로 대체(fallback) 전략을 실행하는 메커니즘을 추가해 보겠습니다.
- 핵심은 각 전략 엔트리에 fallback 전략 타입을 함께 저장하고, short-circuit가 발생하면 해당 대체 전략을 바로 실행하는 구조입니다.
```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <typeindex>
#include <unordered_map>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp
class StrategyFallback : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Fallback Strategy\n";
    }
};
```
```cpp
// Context 클래스 (조건 만족 전략 실행 + Short-Circuit + Fallback)
class Context {
    struct Entry {
        int priority;
        bool shortCircuit; // 실행 후 단락 여부
        std::unique_ptr<Strategy> strategy;
        std::function<bool()> condition;
        std::function<std::unique_ptr<Strategy>()> fallbackFactory; // fallback 전략 생성기
    };

    std::vector<Entry> strategies;

public:
    Context() = default;

    // 전략 등록 (우선순위 + 조건 + shortCircuit 여부 + fallback)
    template <typename T, typename FallbackT = StrategyFallback, typename... Args>
    Context& add(int priority, bool shortCircuit, std::function<bool()> condition, Args&&... args) {
        strategies.push_back({
            priority,
            shortCircuit,
            std::make_unique<T>(std::forward<Args>(args)...),
            condition,
            [](){ return std::make_unique<FallbackT>(); } // fallback 생성기
        });
        return *this;
    }

    // 조건 만족하는 전략을 우선순위 순으로 실행 (shortCircuit + fallback 지원)
    Context& executeAll() {
        // 조건 만족하는 전략만 필터링
        std::vector<Entry*> valid;
        for (auto& e : strategies) {
            if (e.condition && e.condition()) {
                valid.push_back(&e);
            }
        }

        if (!valid.empty()) {
            // 우선순위 내림차순 정렬
            std::sort(valid.begin(), valid.end(),
                [](Entry* a, Entry* b) {
                    return a->priority > b->priority;
                });

            // 실행 (shortCircuit + fallback 체크)
            for (auto* e : valid) {
                e->strategy->execute();
                if (e->shortCircuit) {
                    std::cout << "Short-circuit triggered, executing fallback...\n";
                    auto fb = e->fallbackFactory();
                    fb->execute();
                    break;
                }
            }
        } else {
            std::cout << "No valid strategies to execute.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool condA = true;
    bool condB = true;
    bool condC = true;

    // 여러 전략 등록 (우선순위와 조건, shortCircuit 여부, fallback 지정)
    context.add<StrategyA>(1, false, [&]{ return condA; })
           .add<StrategyB, StrategyC>(3, true,  [&]{ return condB; }) // StrategyB 실행 후 short-circuit → StrategyC fallback 실행
           .add<StrategyC>(2, false, [&]{ return condC; });

    // 실행 → StrategyB 실행 후 short-circuit 발생 → StrategyC fallback 실행
    context.executeAll();

    return 0;
}
```

## 📊 실행 결과
```
Executing Strategy B
Short-circuit triggered, executing fallback...
Executing Strategy C
```

## ✅ 요약
- add<T, FallbackT>(priority, shortCircuit, condition)으로 전략 등록 시 우선순위, 조건, 단락 여부, fallback 전략을 함께 지정.
- executeAll()은 조건 만족 전략을 우선순위 순으로 실행하다가, short-circuit 발생 시 fallback 전략을 자동 실행.
- 호출부는 context.add<StrategyB, StrategyC>(3, true, condFn)처럼 DSL 스타일로 작성 가능.

---

## 📌 다단계 Fallback 체인 지원 버전

- 이제는 여러 fallback 체인을 등록해서 “전략 → fallback1 → fallback2 …” 순으로 이어지는 다단계 대체 실행을 지원하는 버전을 만들어 보겠습니다.
- 핵심은 각 전략 엔트리에 여러 fallback 전략 생성기 리스트를 두고, short‑circuit 발생 시 순서대로 실행하는 구조입니다.
```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

// Strategy 인터페이스
class Strategy {
public:
    virtual void execute() = 0;
    virtual ~Strategy() = default;
};
```
```cpp
// Concrete Strategies
class StrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy A\n";
    }
};
```
```cpp
class StrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy B\n";
    }
};
```
```cpp
class StrategyC : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy C\n";
    }
};
```
```cpp
class StrategyD : public Strategy {
public:
    void execute() override {
        std::cout << "Executing Strategy D\n";
    }
};
```
```cpp
// Context 클래스 (조건 만족 전략 실행 + Short-Circuit + 다단계 Fallback)
class Context {
    struct Entry {
        int priority;
        bool shortCircuit; // 실행 후 단락 여부
        std::unique_ptr<Strategy> strategy;
        std::function<bool()> condition;
        std::vector<std::function<std::unique_ptr<Strategy>()>> fallbacks; // 여러 fallback
    };

    std::vector<Entry> strategies;

public:
    Context() = default;

    // 전략 등록 (우선순위 + 조건 + shortCircuit 여부)
    template <typename T, typename... Args>
    Context& add(int priority, bool shortCircuit, std::function<bool()> condition, Args&&... args) {
        strategies.push_back({
            priority,
            shortCircuit,
            std::make_unique<T>(std::forward<Args>(args)...),
            condition,
            {} // 초기에는 fallback 없음
        });
        return *this;
    }

    // fallback 체인 추가
    template <typename FallbackT, typename... Args>
    Context& addFallbackToLast(Args&&... args) {
        if (!strategies.empty()) {
            strategies.back().fallbacks.push_back(
                [=]() { return std::make_unique<FallbackT>(args...); }
            );
        }
        return *this;
    }

    // 조건 만족하는 전략을 우선순위 순으로 실행 (shortCircuit + 다단계 fallback 지원)
    Context& executeAll() {
        // 조건 만족하는 전략만 필터링
        std::vector<Entry*> valid;
        for (auto& e : strategies) {
            if (e.condition && e.condition()) {
                valid.push_back(&e);
            }
        }

        if (!valid.empty()) {
            // 우선순위 내림차순 정렬
            std::sort(valid.begin(), valid.end(),
                [](Entry* a, Entry* b) {
                    return a->priority > b->priority;
                });

            // 실행 (shortCircuit + fallback 체인 체크)
            for (auto* e : valid) {
                e->strategy->execute();
                if (e->shortCircuit) {
                    std::cout << "Short-circuit triggered, executing fallback chain...\n";
                    for (auto& fbFactory : e->fallbacks) {
                        auto fb = fbFactory();
                        fb->execute();
                    }
                    break; // short-circuit → 이후 전략은 스킵
                }
            }
        } else {
            std::cout << "No valid strategies to execute.\n";
        }
        return *this;
    }
};
```
```cpp
// main 함수
int main() {
    Context context;

    bool condA = true;
    bool condB = true;
    bool condC = true;

    // 전략 등록 + 다단계 fallback 체인
    context.add<StrategyA>(1, false, [&]{ return condA; })
           .add<StrategyB>(3, true,  [&]{ return condB; })
               .addFallbackToLast<StrategyC>()   // fallback1
               .addFallbackToLast<StrategyD>()   // fallback2
           .add<StrategyC>(2, false, [&]{ return condC; });

    // 실행 → StrategyB 실행 후 short-circuit 발생 → fallback1 → fallback2 실행
    context.executeAll();

    return 0;
}
```

## 📊 실행 결과
```
Executing Strategy B
Short-circuit triggered, executing fallback chain...
Executing Strategy C
Executing Strategy D
```


## ✅ 요약
- add<T>(priority, shortCircuit, condition)으로 전략 등록.
- addFallbackToLast<FallbackT>()로 직전에 등록한 전략에 fallback 체인을 추가.
- executeAll()은 조건 만족 전략을 우선순위 순으로 실행하다가 short‑circuit 발생 시 등록된 fallback 체인을 순서대로 실행.
- 호출부는 context.add<StrategyB>(3, true, condFn).addFallbackToLast<StrategyC>().addFallbackToLast<StrategyD>()처럼 DSL 스타일로 작성 가능.


## 개발 내용
- 지금까지 우리가 발전시킨 흐름은 이렇게 됩니다:
    - 기본 전략 패턴 → 스마트 포인터 은닉화
    - Fluent DSL 스타일 → context.use<StrategyA>().execute()
    - 조건부 실행 → executeIf(bool) / executeIf(lambda)
    - 체인식 조건 실행 → .executeIf(cond1).executeIf(cond2)
    - 여러 전략 등록 → context.add<StrategyA>().add<StrategyB>()
    - 우선순위 기반 실행 → 조건 만족 전략 중 최고 우선순위 실행
    - 조건 만족 전략 모두 실행 → 우선순위 순으로 전부 실행
    - Short-circuit 실행 → 특정 전략 실행 후 나머지 스킵
    - Fallback 메커니즘 → Short-circuit 발생 시 대체 전략 실행
    - 다단계 Fallback 체인 → “전략 → fallback1 → fallback2 …” 순으로 이어지는 실행

 ---

