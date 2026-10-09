## 📘 Java의 Anonymous Inner Class
- 먼저 Java에서 전통적으로 이벤트를 처리하던 방식입니다.
- cpp에는 익명 클래스가 없지만 람다을 이용해서 비슷한 동작을 구현

### 📌 java 코드 
```java
interface ClickHandler {
    void onClick();
}

class Button {
    private ClickHandler handler;

    public void setOnClick(ClickHandler handler) {
        this.handler = handler;
    }

    public void click() {
        if (handler != null) {
            handler.onClick();
        }
    }
}

public class Main {
    public static void main(String[] args) {

        Button button = new Button();

        button.setOnClick(new ClickHandler() {
            @Override
            public void onClick() {
                System.out.println("Clicked");
            }
        });

        button.click();
    }
}
```
- ClickHandler 인터페이스를 구현하는 이름 없는 클래스의 객체를 생성하는 문법입니다.
- 즉, 내부적으로 다음과 같은 클래스를 작성한 것과 비슷합니다.
```java
class MyClickHandler implements ClickHandler {
    @Override
    public void onClick() {
        System.out.println("Clicked");
    }
}
```
-  그리고 다음처럼 전달하는 것입니다.
```java
button.setOnClick(new MyClickHandler());
```
- 익명 클래스는 별도의 클래스 이름을 만들지 않고 그 자리에서 구현할 수 있다는 장점이 있습니다.

### 📌 Java Lambda 방식
```java
@FunctionalInterface
interface ClickHandler {
    void onClick();
}
```
- ClickHandler는 추상 메서드가 하나뿐이므로 Lambda를 사용할 수 있습니다.
```java
Button button = new Button();

button.setOnClick(() -> {
    System.out.println("Clicked");
});

button.click();
```
### 📌 C++17 Lambda 방식
```cpp
#include <iostream>
#include <functional>

class Button {
public:
    using ClickHandler = std::function<void()>;

    void setOnClick(ClickHandler handler) {
        handler_ = std::move(handler);
    }

    void click() {
        if (handler_)
            handler_();
    }

private:
    ClickHandler handler_;
};

int main() {
    Button button;

    button.setOnClick([]() {
        std::cout << "Clicked\n";
    });

    button.click();
}
```

### 📌 Java와 C++의 가장 중요한 차이
- Java는 인터페이스를 중심으로 이벤트 핸들러를 정의하는 방식입니다.
```java
interface ClickHandler {
    void onClick();
}
```
- C++에서는 함수의 시그니처를 중심으로 정의할 수 있습니다.
```cpp
using ClickHandler = std::function<void()>;
```
- Java에서는 onClick()이라는 메서드를 호출합니다.
```java
handler.onClick();
```
- C++에서는 저장된 함수 객체를 직접 호출합니다.
```cpp
handler_();
```


### 📌 정리
| 항목 | Java | C++17 |
| ---- | ----| ------- |
| 이벤트 인터페이스 | interface | std::function |
| 익명 클래스 | new Interface() { ... } | 직접 대응하는 익명 클래스 문법 없음 |
| Lambda | `() -> { ... }` | `[]() { ... }` |
| 외부 변수 접근 | effectively final 캡처 | 값 또는 참조 캡처 |
| 이벤트 등록 | setOnClick(handler) | setOnClick(handler) |
| 이벤트 실행 | handler.onClick() | handler_() |
| 반환값 지정 | 인터페이스 메서드에서 정의 | std::function<R(Args...)> |

---

