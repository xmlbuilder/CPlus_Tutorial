## 🧠 enum class란?
- enum class는 C++11부터 도입된 스코프가 있는 열거형. 
- 기존의 enum은 전역 네임스페이스에 값을 노출했지만, enum class는 자체 스코프를 가지므로 이름 충돌을 방지할 수 있음.
```cpp
enum class Color { Red, Green, Blue };
Color c = Color::Red;  // OK
```


### 📌 기존 enum과의 주요 차이점 및 장점
| 특징 | 기존 enum | enum class |
|------|----------|-------------| 
| 스코프 | 전역에 노출됨 | 열거형 내부에 국한됨 | 
| 암시적 변환 | int로 자동 변환됨 | 명시적 캐스팅 필요 | 
| 타입 안전성 | 낮음 | 높음 | 
| 중복 이름 허용 | 불가 | 가능 (다른 enum class끼리는 OK) | 
| 기본 타입 지정 | 불가능 | 가능 (enum class Color : uint8_t) | 


### 예시:
```cpp
enum Fruit { Apple, Banana };
enum Color { Red, Green, Apple }; // 오류! Apple 중복

enum class Fruit2 { Apple, Banana };
enum class Color2 { Red, Green, Apple }; // OK!
```


### 🧩 형(type)을 지정하면 뭐가 달라질까?
- enum class는 기본적으로 int 타입을 사용하지만, 원하는 타입으로 지정할 수 있음.  
- 예를 들어:
```cpp
enum class Status : uint8_t { OK = 0, Error = 1 };
```

### 📌 타입 지정의 효과
- 메모리 절약: uint8_t처럼 작은 타입을 쓰면 메모리 사용량을 줄일 수 있어. 특히 임베디드 시스템에서 유용.
- 직렬화/통신: 네트워크나 파일 저장 시, 타입을 명확히 지정하면 데이터 포맷을 일관되게 유지할 수 있음.
- 정렬/비교: 타입에 따라 정렬이나 비교 연산의 성능이 달라질 수 있음.
- 단, 타입을 지정해도 여전히 암시적 변환은 허용되지 않기 때문에 명시적으로 캐스팅해야 함:
```cpp
Status s = Status::OK;
uint8_t raw = static_cast<uint8_t>(s);  // 명시적 변환 필요
```


### 활용 예제

#### 📌 1. 상태 관리(State Machine) 예제
- enum class는 명확한 상태 표현에 아주 적합. 
- 예를 들어, 게임 캐릭터의 상태를 관리한다면:
```cpp
enum class CharacterState : uint8_t {
    Idle,
    Walking,
    Attacking,
    Dead
};

class Character {
private:
    CharacterState state;

public:
    Character() : state(CharacterState::Idle) {}

    void update() {
        switch (state) {
            case CharacterState::Idle:
                // 대기 중 로직
                break;
            case CharacterState::Walking:
                // 이동 중 로직
                break;
            case CharacterState::Attacking:
                // 공격 중 로직
                break;
            case CharacterState::Dead:
                // 사망 처리
                break;
        }
    }

    void setState(CharacterState newState) {
        state = newState;
    }
};
```
- 이렇게 하면 상태 전환이 명확하고, 실수로 잘못된 값을 넣는 일이 줄어듬.

#### 📌 2. 전략 패턴(Strategy Pattern)과의 조합
- enum class를 사용해서 전략을 선택하고, 해당 전략에 따라 동작을 위임할 수 있음.
```cpp
enum class SortStrategy {
    QuickSort,
    MergeSort,
    BubbleSort
};

class Sorter {
public:
    void sort(std::vector<int>& data, SortStrategy strategy) {
        switch (strategy) {
            case SortStrategy::QuickSort:
                quickSort(data);
                break;
            case SortStrategy::MergeSort:
                mergeSort(data);
                break;
            case SortStrategy::BubbleSort:
                bubbleSort(data);
                break;
        }
    }

private:
    void quickSort(std::vector<int>& data) { /* 구현 생략 */ }
    void mergeSort(std::vector<int>& data) { /* 구현 생략 */ }
    void bubbleSort(std::vector<int>& data) { /* 구현 생략 */ }
};
```

- 이런 방식은 전략을 명확하게 표현하고, 확장성도 좋음.

#### 📌 3. 명령 패턴(Command Pattern)에서의 활용
- 사용자 입력이나 이벤트를 처리할 때도 enum class가 유용함.
```cpp
enum class CommandType {
    MoveLeft,
    MoveRight,
    Jump,
    Shoot
};

class Command {
public:
    virtual void execute() = 0;
};

class MoveLeftCommand : public Command {
    void execute() override { /* 왼쪽 이동 */ }
};

class JumpCommand : public Command {
    void execute() override { /* 점프 */ }
};

// 매핑 테이블
std::unique_ptr<Command> createCommand(CommandType type) {
    switch (type) {
        case CommandType::MoveLeft: return std::make_unique<MoveLeftCommand>();
        case CommandType::Jump: return std::make_unique<JumpCommand>();
        // ...
        default: return nullptr;
    }
}
```

- 이렇게 하면 입력 → 명령 객체 → 실행의 흐름이 깔끔하게 정리됨.

---

