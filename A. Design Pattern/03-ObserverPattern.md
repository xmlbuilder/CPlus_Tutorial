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
