# Observer Pattern
**옵저버 패턴 (Observer Pattern)** 패턴의 취지, 사용법, 실무 활용, 코드 설명, Mermaid 다이어그램까지 정리.

## 👁️ 옵저버 패턴이란?
옵저버 패턴은 객체의 상태 변화가 있을 때, 그 변화를 **자동으로 감지하고 반응하는 객체들(옵저버)** 에게 알림을 보내는 디자인 패턴.
## ✅ 핵심 개념
- Subject (주체): 상태를 갖고 있으며, 옵저버를 등록/해제/알림
- Observer (관찰자): 상태 변화에 반응하는 객체
- 의존성 역전(DIP): Subject는 Observer의 인터페이스만 알고 있음

## 🧠 취지와 사용 목적
| 목적 또는 상황             | 설명                                                                 |
|----------------------------|----------------------------------------------------------------------|
| 상태 변화 자동 반영         | 주체 객체의 상태가 바뀌면 등록된 옵저버들이 자동으로 반응함               |
| 느슨한 결합 구조            | 주체와 옵저버가 인터페이스로만 연결되어 있어 확장성과 유지보수성이 뛰어남   |
| 이벤트 기반 시스템 구현     | GUI, 게임, 실시간 처리 등에서 이벤트 감지 및 반응에 적합                   |
| 다중 구독자 처리            | 여러 옵저버가 하나의 주체를 동시에 감시할 수 있어 유연한 구조 설계 가능     |
| 관심사 분리                 | 상태 관리와 반응 로직을 분리하여 코드의 명확성과 재사용성을 높임           |


## 🧩 C++: Interface + Vector 기반 Observer
```cpp
#include <iostream>
#include <vector>
#include <memory>

class Observer {
public:
    virtual void update(const std::string& msg) = 0;
};

class Subject {
    std::vector<std::shared_ptr<Observer>> observers;
public:
    void registerObserver(std::shared_ptr<Observer> obs) {
        observers.push_back(obs);
    }

    void notifyAll(const std::string& msg) {
        for (auto& obs : observers) {
            obs->update(msg);
        }
    }
};
```
```cpp
class Logger : public Observer {
public:
    void update(const std::string& msg) override {
        std::cout << "[Logger] " << msg << std::endl;
    }
};
```
```cpp
/ Concrete Observer 2
class Alert : public Observer {
public:
    void update(const std::string& msg) override {
        if (msg.find("error") != std::string::npos) {
            std::cout << "[Alert] ⚠️ Critical issue detected: " << msg << std::endl;
        }
    }
};
```
```cpp
// Concrete Observer 3
class AuditTrail : public Observer {
public:
    void update(const std::string& msg) override {
        std::cout << "[AuditTrail] Logged event: " << msg << std::endl;
    }
};
```
```cpp
// main 함수
int main() {
    Subject system;

    auto logger = std::make_shared<Logger>();
    auto alert = std::make_shared<Alert>();
    auto audit = std::make_shared<AuditTrail>();

    system.registerObserver(logger);
    system.registerObserver(alert);
    system.registerObserver(audit);

    system.notifyAll("System started");
    system.notifyAll("User login successful");
    system.notifyAll("Disk error detected");

    return 0;
}
```

---

## 스마트 포인트 은닉화

- 현재 코드에서는 main 함수에서 std::make_shared를 직접 호출하고 registerObserver에 넘겨주고 있습니다.  
- 즉, 스마트 포인터 관리가 호출부에 그대로 드러나 있어서 진입 장벽이 생깁니다.
- 이를 은닉화하려면 팩토리 함수나 빌더 클래스를 두고, 내부에서 std::make_shared를 처리하게 하면 됩니다.
- 호출자는 단순히 system.addObserver<Logger>() 같은 API만 쓰면 됨.

## 📌 스마트 포인터 은닉화된 Observer 패턴
```cpp
#include <iostream>
#include <vector>
#include <memory>

// Observer 인터페이스
class Observer {
public:
    virtual void update(const std::string& msg) = 0;
    virtual ~Observer() = default;
};
```
```cpp
// Subject 클래스
class Subject {
    std::vector<std::shared_ptr<Observer>> observers;
public:
    // Observer 등록을 템플릿으로 은닉화
    template <typename T, typename... Args>
    void addObserver(Args&&... args) {
        observers.push_back(std::make_shared<T>(std::forward<Args>(args)...));
    }

    void notifyAll(const std::string& msg) {
        for (auto& obs : observers) {
            obs->update(msg);
        }
    }
};
```
```cpp
// Concrete Observers
class Logger : public Observer {
public:
    void update(const std::string& msg) override {
        std::cout << "[Logger] " << msg << std::endl;
    }
};
```
```cpp
class Alert : public Observer {
public:
    void update(const std::string& msg) override {
        if (msg.find("error") != std::string::npos) {
            std::cout << "[Alert] ⚠️ Critical issue detected: " << msg << std::endl;
        }
    }
};
```
```cpp
class AuditTrail : public Observer {
public:
    void update(const std::string& msg) override {
        std::cout << "[AuditTrail] Logged event: " << msg << std::endl;
    }
};
```
```cpp
// main 함수
int main() {
    Subject system;

    // 스마트 포인터를 직접 다루지 않고 Observer 추가
    system.addObserver<Logger>();
    system.addObserver<Alert>();
    system.addObserver<AuditTrail>();

    system.notifyAll("System started");
    system.notifyAll("User login successful");
    system.notifyAll("Disk error detected");

    return 0;
}
```


## 📊 개선된 점
- main 함수에서 스마트 포인터 코드가 완전히 사라짐.
- system.addObserver<Logger>()처럼 객체 생성만 신경 쓰면 됨.
- 내부에서 std::make_shared와 std::forward로 안전하게 자원 관리.
- 확장성 ↑ → 새로운 Observer 추가 시에도 호출부는 단순.

## ✅ 요약
- Observer 패턴에서도 스마트 포인터를 Subject 내부에서 은닉화하면,  
  호출자는 **객체를 그냥 등록한다** 는 직관적인 경험만 하게 됩니다.

---

## 📌 인자가 필요한 Observer 등록 예제
```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Observer 인터페이스
class Observer {
public:
    virtual void update(const std::string& msg) = 0;
    virtual ~Observer() = default;
};
```
```cpp
// Subject 클래스
class Subject {
    std::vector<std::shared_ptr<Observer>> observers;
public:
    // Observer 등록을 템플릿으로 은닉화 (생성자 인자 지원)
    template <typename T, typename... Args>
    void addObserver(Args&&... args) {
        observers.push_back(std::make_shared<T>(std::forward<Args>(args)...));
    }

    void notifyAll(const std::string& msg) {
        for (auto& obs : observers) {
            obs->update(msg);
        }
    }
};
```
```cpp
// Concrete Observers
class Logger : public Observer {
    std::string prefix;
public:
    Logger(const std::string& p) : prefix(p) {}
    void update(const std::string& msg) override {
        std::cout << "[" << prefix << "] " << msg << std::endl;
    }
};
```
```cpp
class Alert : public Observer {
    int level;
public:
    Alert(int lvl) : level(lvl) {}
    void update(const std::string& msg) override {
        if (msg.find("error") != std::string::npos) {
            std::cout << "[Alert L" << level << "] ⚠️ Critical issue detected: " << msg << std::endl;
        }
    }
};
```
```cpp
class AuditTrail : public Observer {
public:
    AuditTrail() {}
    void update(const std::string& msg) override {
        std::cout << "[AuditTrail] Logged event: " << msg << std::endl;
    }
};
```
```cpp
// main 함수
int main() {
    Subject system;

    // 생성자 인자를 넘겨서 Observer 등록
    system.addObserver<Logger>("SystemLogger");
    system.addObserver<Alert>(2); // Alert 레벨 지정
    system.addObserver<AuditTrail>(); // 기본 생성자

    system.notifyAll("System started");
    system.notifyAll("User login successful");
    system.notifyAll("Disk error detected");

    return 0;
}
```


## 📊 실행 결과
```
[SystemLogger] System started
[Alert L2] ⚠️ Critical issue detected: System started
[AuditTrail] Logged event: System started
[SystemLogger] User login successful
[AuditTrail] Logged event: User login successful
[SystemLogger] Disk error detected
[Alert L2] ⚠️ Critical issue detected: Disk error detected
[AuditTrail] Logged event: Disk error detected
```
## ✅ 요약
- template <typename T, typename... Args> + std::forward<Args>(args)...를 쓰면 Observer 생성자 인자까지 은닉화 가능.
- 호출부는 system.addObserver<Logger>("prefix")처럼 단순하게 작성.
- 스마트 포인터(std::make_shared)와 move/forward는 내부에서 처리 → 가독성↑, 진입장벽↓.

---
## 📌 Fluent Builder 스타일 Observer 패턴
```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Observer 인터페이스
class Observer {
public:
    virtual void update(const std::string& msg) = 0;
    virtual ~Observer() = default;
};
```
```cpp
// Subject 클래스 (Fluent Builder 스타일)
class Subject {
    std::vector<std::shared_ptr<Observer>> observers;
public:
    // 체인식 등록 지원
    template <typename T, typename... Args>
    Subject& add(Args&&... args) {
        observers.push_back(std::make_shared<T>(std::forward<Args>(args)...));
        return *this; // 자기 자신 반환 → 체인 가능
    }

    Subject& notifyAll(const std::string& msg) {
        for (auto& obs : observers) {
            obs->update(msg);
        }
        return *this; // 체인식 호출 가능
    }
};
```
```cpp
// Concrete Observers
class Logger : public Observer {
    std::string prefix;
public:
    Logger(const std::string& p) : prefix(p) {}
    void update(const std::string& msg) override {
        std::cout << "[" << prefix << "] " << msg << std::endl;
    }
};
```
```cpp
class Alert : public Observer {
    int level;
public:
    Alert(int lvl) : level(lvl) {}
    void update(const std::string& msg) override {
        if (msg.find("error") != std::string::npos) {
            std::cout << "[Alert L" << level << "] ⚠️ Critical issue detected: " << msg << std::endl;
        }
    }
};
```
```cpp
class AuditTrail : public Observer {
public:
    AuditTrail() {}
    void update(const std::string& msg) override {
        std::cout << "[AuditTrail] Logged event: " << msg << std::endl;
    }
};
```
```cpp
// main 함수
int main() {
    Subject system;

    // 체인식 등록 및 호출
    system.add<Logger>("SystemLogger")
          .add<Alert>(2)
          .add<AuditTrail>()
          .notifyAll("System started")
          .notifyAll("User login successful")
          .notifyAll("Disk error detected");

    return 0;
}
```

## 📊 장점
- 스마트 포인터 은닉화: 호출부에서는 std::make_shared나 std::move를 전혀 보지 않음.
- 체인식 API: add<>()와 notifyAll()을 이어서 호출 → 가독성↑.
- 유연성: Observer 생성자 인자도 자연스럽게 전달 가능.

## ✅ 요약
- 이제 Observer 패턴을 Fluent Builder 스타일로 구현했으니, 호출자는 **객체를 그냥 등록하고 체인으로 이어가는** 직관적인 경험을 할 수 있습니다.


---

## 📌 DSL 스타일 Observer 패턴 구현
```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Observer 인터페이스
class Observer {
public:
    virtual void update(const std::string& msg) = 0;
    virtual ~Observer() = default;
};
```
```cpp
// Subject 클래스 (DSL 스타일)
class Subject {
    std::vector<std::shared_ptr<Observer>> observers;
public:
    // Observer 등록 DSL: on<T>(args...)
    template <typename T, typename... Args>
    Subject& on(Args&&... args) {
        observers.push_back(std::make_shared<T>(std::forward<Args>(args)...));
        return *this; // 체인식 호출 가능
    }

    // 이벤트 발생 DSL: emit(msg)
    Subject& emit(const std::string& msg) {
        for (auto& obs : observers) {
            obs->update(msg);
        }
        return *this; // 체인식 호출 가능
    }
};
```
```cpp
// Concrete Observers
class Logger : public Observer {
    std::string prefix;
public:
    Logger(const std::string& p) : prefix(p) {}
    void update(const std::string& msg) override {
        std::cout << "[" << prefix << "] " << msg << std::endl;
    }
};
```
```cpp
class Alert : public Observer {
    int level;
public:
    Alert(int lvl) : level(lvl) {}
    void update(const std::string& msg) override {
        if (msg.find("error") != std::string::npos) {
            std::cout << "[Alert L" << level << "] ⚠️ Critical issue detected: " << msg << std::endl;
        }
    }
};
```
```cpp
class AuditTrail : public Observer {
public:
    AuditTrail() {}
    void update(const std::string& msg) override {
        std::cout << "[AuditTrail] Logged event: " << msg << std::endl;
    }
};
```
```cpp
// main 함수
int main() {
    Subject system;

    // DSL 스타일로 Observer 등록과 이벤트 발생
    system.on<Logger>("SystemLogger")
          .on<Alert>(2)
          .on<AuditTrail>()
          .emit("System started")
          .emit("User login successful")
          .emit("Disk error detected");

    return 0;
}
```

## 📊 DSL 스타일의 장점
- 가독성↑: on<>()과 emit()만 보이므로, 마치 이벤트 시스템 DSL처럼 읽힘.
- 스마트 포인터 은닉화: 내부에서 std::make_shared 처리 → 호출부는 단순.
- 체인식 API: Observer 등록과 이벤트 발생을 자연스럽게 이어서 표현 가능.

## ✅ 요약
- 이제 Observer 패턴을 DSL처럼 표현했으니, 호출부는 **이벤트 시스템 스크립트** 처럼 직관적으로 읽힙니다.

---






