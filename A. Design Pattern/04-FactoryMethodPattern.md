
## 📘 FactoryMethod Pattern
```cpp
#include <iostream>
#include <memory>

class Transport {
public:
    virtual void deliver() = 0;
    virtual ~Transport() = default;
};
```
```cpp
class Truck : public Transport {
public:
    void deliver() override {
        std::cout << "Delivering by land in a box.\n";
    }
};
```
```cpp
class Ship : public Transport {
public:
    void deliver() override {
        std::cout << "Delivering by sea in a container.\n";
    }
};
```
---

```cpp
class Logistics {
public:
    virtual std::shared_ptr<Transport> createTransport() = 0;
    virtual ~Logistics() = default;
};
```
```cpp
class RoadLogistics : public Logistics {
public:
    std::shared_ptr<Transport> createTransport() override {
        return std::make_shared<Truck>();
    }
};
```
```cpp
class SeaLogistics : public Logistics {
public:
    std::shared_ptr<Transport> createTransport() override {
        return std::make_shared<Ship>();
    }
};
```
```cpp
int main() {
    std::shared_ptr<Logistics> road = std::make_shared<RoadLogistics>();
    std::shared_ptr<Logistics> sea = std::make_shared<SeaLogistics>();

    road->createTransport()->deliver();
    sea->createTransport()->deliver();

    return 0;
}
```


### 📌 왜 unique_ptr이 적합한가?
| 항목               | 설명                                           |
|--------------------|------------------------------------------------|
| `move`             | 소유권을 명확히 이전하여 객체 생명주기 관리 가능     |
| 자동 `delete`      | 객체 소멸 시 자동으로 메모리 해제 (RAII 원칙 적용)   |
| 예외 안전성        | 예외 발생 시에도 메모리 누수 없이 안전하게 처리       |
| `shared_ptr` 대비  | 참조 카운팅 오버헤드 없음, 단일 소유권에 더 적합      |

---

