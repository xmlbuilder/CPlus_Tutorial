# 📘 C++ String Switch 구현 방법

## 🔹 문제점

-   C++의 `switch` 문은 **정수형(enum, int, char 등)**만 지원한다.
-   따라서 `std::string`을 직접 `switch`에 사용할 수 없다.

------------------------------------------------------------------------

## 🔹 해결 방법: StringSwitch 클래스 구현

``` cpp
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

class StringSwitch {
public:
    using Handler = std::function<void()>;

    void addCase(const std::string& key, Handler handler) {
        table[key] = handler;
    }

    void execute(const std::string& key) const {
        auto it = table.find(key);
        if (it != table.end()) {
            it->second();
        } else {
            std::cout << "Default case: " << key << std::endl;
        }
    }

private:
    std::unordered_map<std::string, Handler> table;
};

int main() {
    StringSwitch ss;

    ss.addCase("start", []() { std::cout << "Starting..." << std::endl; });
    ss.addCase("stop", []() { std::cout << "Stopping..." << std::endl; });
    ss.addCase("pause", []() { std::cout << "Pausing..." << std::endl; });

    std::string command;
    std::cout << "Enter command: ";
    std::cin >> command;

    ss.execute(command);

    return 0;
}
```

------------------------------------------------------------------------

## 🔹 실행 예시

    Enter command: start
    Starting...

    Enter command: stop
    Stopping...

    Enter command: hello
    Default case: hello

------------------------------------------------------------------------

## ✅ 개선 포인트

1.  **`unordered_map` 사용** → 평균 O(1) 탐색 성능 제공\
2.  **람다 함수 기반 등록** → 각 case 코드를 깔끔하게 관리 가능\
3.  **기본(default) 처리** → 존재하지 않는 문자열 입력 시 fallback 동작
    제공\
4.  **확장성** → enum/string 변환 매핑을 추가하면 혼합형 switch도 가능

------------------------------------------------------------------------

## 📝 결론

-   C++에서 문자열 기반 `switch`는 직접 지원되지 않는다.\
-   대신 `unordered_map + std::function` 기반의 디스패처 패턴을 사용하면
    깔끔하게 해결 가능하다.\
-   이는 명령어 처리, 이벤트 핸들링, 간단한 스크립팅 등에 유용하게 쓰일
    수 있다.
