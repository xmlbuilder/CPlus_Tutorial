# std::forward<Args>(args)...

- std::forward<Args>(args)... 는 템플릿 함수에서 전달받은 인자들을 **완벽 전달(perfect forwarding)** 하기 위한 구문입니다.  
- 즉, 원래 인자가 lvalue였는지 rvalue였는지를 그대로 보존해서 다른 함수로 넘겨줍니다.

## 📌 배경
- C++ 템플릿에서 Args&&... args 같은 forwarding reference (universal reference) 를 쓰면,  
  인자가 lvalue인지 rvalue인지에 따라 타입이 달라집니다.
- 하지만 함수 내부에서 이름을 붙여버리면, 원래 rvalue였던 것도 lvalue처럼 취급됩니다.
- 이때 std::forward<T>(arg) 를 사용하면, 원래의 값 카테고리(lvalue/rvalue)를 복원해서 전달할 수 있습니다.

## 📌 구문 의미
```rust
template <typename... Args>
void wrapper(Args&&... args) {
    // 다른 함수로 인자를 그대로 전달
    target(std::forward<Args>(args)...);
}
```
- Args&&... args → 인자를 lvalue든 rvalue든 다 받을 수 있음.
- std::forward<Args>(args)... → 각 인자를 원래 타입(lvalue/rvalue)을 유지한 채로 target 함수에 전달.
- `...` → pack expansion으로 여러 인자를 한 번에 펼쳐서 전달.

## 📌 예시
```cpp
#include <iostream>
#include <utility>

void process(int& x) { std::cout << "lvalue\n"; }
void process(int&& x) { std::cout << "rvalue\n"; }

template <typename... Args>
void forwarder(Args&&... args) {
    process(std::forward<Args>(args)...);
}
``
``cpp
int main() {
    int a = 10;
    forwarder(a);        // lvalue
    forwarder(20);       // rvalue
}
```

- forwarder(a) → a는 lvalue → process(int&) 호출.
- forwarder(20) → 20은 rvalue → process(int&&) 호출.

## 📌 핵심 요약
- std::forward는 인자의 원래 성격(lvalue/rvalue)을 보존해서 전달합니다.
- std::forward<Args>(args)...는 가변 인자 팩을 펼쳐서 모두 완벽 전달합니다.
- std::move와 달리, std::forward는 원래 rvalue일 때만 rvalue로 캐스팅합니다.

- 👉 정리하면, std::forward<Args>(args)... 는 템플릿 함수에서 받은 인자들을 다른 함수로 넘길 때,  
  원래의 lvalue/rvalue 특성을 그대로 유지하면서 전달하는 문법입니다.


## 📊 std::move vs std::forward 비교

| 구분 | std::move | std::forward |
|------|-----------|--------------|
| 목적 | 무조건 rvalue로 캐스팅 | 원래 lvalue/rvalue 성격을 보존 |
| 사용 상황 | 객체를 더 이상 사용하지 않을 때 | 템플릿에서 인자를 다른 함수로 전달할 때 |
| 동작 방식 | 항상 rvalue 참조로 변환 | 템플릿 타입에 따라 lvalue/rvalue 구분 유지 |
| 안전성 | 잘못 쓰면 아직 필요한 객체를 move 해버릴 위험 | 원래 성격을 유지하므로 안전하게 전달 |
| 타입 표현 | `T&&` | `T&&` |
| 대표 예시 | `vec.push_back(std::move(x));` | ```cpp template<typename T> void f(T&& arg){ g(std::forward<T>(arg)); } ``` |


## 📝 핵심 요약
- std::move → **이제 이 객체는 안 씀, rvalue로 취급!**
- std::forward → **원래 인자가 lvalue였는지 rvalue였는지 그대로 전달!**

- 👉 즉, std::move는 강제 이동, std::forward는 원래 성격 유지라고 기억하면 됩니다.

## 📌 예제 코드
```cpp
#include <iostream>
#include <utility>
#include <string>

void process(const std::string& s) {
    std::cout << "lvalue: " << s << "\n";
}

void process(std::string&& s) {
    std::cout << "rvalue: " << s << "\n";
}
```
```cpp
template <typename T>
void use_move(T&& arg) {
    // 무조건 rvalue로 캐스팅
    process(std::move(arg));
}
```cpp
template <typename T>
void use_forward(T&& arg) {
    // 원래 인자의 성격(lvalue/rvalue)을 보존
    process(std::forward<T>(arg));
}
```
```cpp
int main() {
    std::string str = "Hello";

    std::cout << "=== std::move ===\n";
    use_move(str);          // lvalue였지만 move → rvalue로 처리됨
    use_move(std::string("World")); // rvalue → rvalue

    std::cout << "=== std::forward ===\n";
    use_forward(str);       // lvalue → lvalue 그대로 전달
    use_forward(std::string("World")); // rvalue → rvalue 그대로 전달
}
```


## 📊 실행 결과
```
=== std::move ===
rvalue: Hello
rvalue: World
=== std::forward ===
lvalue: Hello
rvalue: World
```


## ✅ 핵심 포인트
- std::move → 항상 rvalue로 캐스팅 → lvalue도 rvalue로 바뀜.
- std::forward → 원래 성격 유지 → lvalue는 lvalue, rvalue는 rvalue.
- 👉 이렇게 보면 std::move는 강제 이동, std::forward는 완벽 전달(perfect forwarding) 이라는 차이가 확실히 드러납니다.


## 📌 클래스 버전 예제
```cpp
#include <iostream>
#include <string>
#include <utility>

class Wrapper {
    std::string data;

public:
    // 생성자
    Wrapper(std::string s) : data(std::move(s)) {}

    // 멤버 함수: std::move 사용 → 항상 rvalue로 캐스팅
    template <typename T>
    void setDataMove(T&& arg) {
        data = std::move(arg);  // 무조건 rvalue로 처리
    }

    // 멤버 함수: std::forward 사용 → 원래 성격 유지
    template <typename T>
    void setDataForward(T&& arg) {
        data = std::forward<T>(arg);  // lvalue는 lvalue, rvalue는 rvalue
    }

    void print() const {
        std::cout << "data = " << data << "\n";
    }
};
```
```cpp
int main() {
    std::string str = "Hello";

    Wrapper w("Init");
    w.print();

    std::cout << "=== Using setDataMove ===\n";
    w.setDataMove(str);              // lvalue지만 move → str은 비워짐
    w.print();
    std::cout << "str after move: " << str << "\n";  // str은 빈 문자열

    std::cout << "=== Using setDataForward ===\n";
    str = "World";
    w.setDataForward(str);           // lvalue 그대로 전달 → str은 유지됨
    w.print();
    std::cout << "str after forward: " << str << "\n";  // str은 여전히 "World"

    w.setDataForward(std::string("Temp")); // rvalue 전달 → 이동 발생
    w.print();
}
```


### 📊 실행 결과
```
data = Init
=== Using setDataMove ===
data = Hello
str after move: 
=== Using setDataForward ===
data = World
str after forward: World
data = Temp
```
---

✅ 핵심 포인트
- setDataMove: std::move를 쓰면 항상 rvalue로 캐스팅 → lvalue도 강제로 이동 → 원본이 비워짐.
- setDataForward: std::forward를 쓰면 원래 성격 유지 → lvalue는 복사, rvalue는 이동 → 원본 안전하게 유지 가능.

---


