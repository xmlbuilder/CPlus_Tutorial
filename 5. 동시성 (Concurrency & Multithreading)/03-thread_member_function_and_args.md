# C++ Thread: 멤버 함수/자유 함수 전달하기 정리

`std::thread`는 **호출 가능 대상(callable)**과 **인자들**을 받아 새 스레드를 시작합니다.  
여기에는 **자유 함수, 정적 멤버 함수, 비정적 멤버 함수, 람다, 함수 객체** 등이 모두 포함됩니다.

---

## 1) 자유 함수 전달 (기본 형태)

```cpp
#include <iostream>
#include <thread>
#include <string>

void threadFunction(const std::string& message, int num) {
    std::cout << message << " " << num << "\n";
}

int main() {
    std::string arg1 = "Hello world!";
    int arg2 = 10;

    std::thread t(threadFunction, arg1, arg2); // 인자들은 기본적으로 값 복사
    t.join();
    // 출력 예: Hello world! 10
}
```
- 스레드 생성 시 **인자들은 기본적으로 복사**됩니다. (참조로 넘기려면 `std::ref` 필요 — §4 참고)

---

## 2) 비정적 멤버 함수 전달

비정적 멤버 함수 포인터는 **첫 번째 인자로 객체(또는 포인터/참조)** 가 필요합니다.

```cpp
#include <iostream>
#include <thread>

class MyClass {
public:
    void memberFunction(int x) {
        for (int i = 0; i < x; ++i) {
            std::cout << "Working : " << i << "\n";
        }
    }
};

int main() {
    MyClass obj;

    // (멤버 함수 포인터, 객체 주소/참조, 실제 인자...)
    std::thread t(&MyClass::memberFunction, &obj, 5);
    // std::thread t(&MyClass::memberFunction, std::ref(obj), 5); // 참조로 넘기는 또 다른 방식

    // 메인 스레드 작업
    for (int i = 0; i < 3; ++i) std::cout << "Main : " << i << "\n";

    t.join();
}
```
- **주의:** `&obj` (또는 `std::ref(obj)`)로 넘긴 경우, 스레드가 실행되는 동안 **obj의 수명이 보장**되어야 합니다.

### const 멤버 함수도 동일
```cpp
struct Logger {
    void log(int n) const { std::cout << "const log " << n << "\n"; }
};
int main() {
    Logger lg;
    std::thread t(&Logger::log, &lg, 42); // OK (const 멤버도 동일한 방식)
    t.join();
}
```

### 정적 멤버 함수는 자유 함수처럼
```cpp
struct Util {
    static void run(int n) { std::cout << "static run " << n << "\n"; }
};
int main() {
    std::thread t(&Util::run, 7); // 객체 불필요
    t.join();
}
```

---

## 3) 람다로 감싸 전달 (캡처 활용)

람다는 문법이 간결하고, **캡처**를 통해 외부 상태를 쉽게 전달할 수 있습니다.

```cpp
#include <thread>
#include <vector>
#include <iostream>

int main() {
    int limit = 5;
    std::vector<int> data{1,2,3};

    std::thread t([=]() {
        // limit, data가 값 캡처됨 (복사됨)
        for (int i = 0; i < limit; ++i) std::cout << "Lambda " << i << "\n";
        std::cout << "data size copy: " << data.size() << "\n";
    });

    t.join();
}
```
- **값 캡처**는 복사, **참조 캡처**는 `&` 사용. 수명/동기화에 주의.

---

## 4) 인자 전달 규칙 (복사/참조/이동)

### (1) 참조로 넘기기 — `std::ref`
```cpp
#include <thread>
#include <iostream>
#include <functional>

void add_one(int& x) { ++x; }

int main() {
    int v = 10;
    std::thread t(add_one, std::ref(v)); // ref 없으면 복사본에 ++되어 원본 변화 없음
    t.join();
    std::cout << v << "\n"; // 11
}
```

### (2) 이동 전용(move-only) 타입 넘기기 — `std::move`
```cpp
#include <thread>
#include <memory>
#include <iostream>

void take_unique(std::unique_ptr<int> p) {
    std::cout << *p << "\n";
}

int main() {
    auto up = std::make_unique<int>(99);
    std::thread t(take_unique, std::move(up)); // unique_ptr 이동
    t.join();
    // up은 nullptr
}
```

---

## 5) `std::bind` vs 람다
`std::bind`로도 멤버/자유 함수를 결합할 수 있지만, 현대 C++에서는 **람다**가 가독성·최적화 측면에서 선호됩니다.

```cpp
#include <functional>
#include <thread>

struct S { void f(int) {} } s;

int main() {
    using std::placeholders::_1;
    auto bound = std::bind(&S::f, &s, _1); // s.f(arg)를 고정
    std::thread t(bound, 123);
    t.join();
}
```

---

## 6) 수명·예외·동기화 주의사항 (핵심 체크리스트)

- **수명 보장:** 스레드에 넘긴 객체/참조가 스레드 수행 중 파괴되면 **UB(미정의 동작)** 입니다.  
  - 필요하면 동적 수명 관리(스마트 포인터) 또는 **값 캡처/복사** 사용.
- **예외:** 스레드 함수에서 예외가 던져지면 `std::terminate`가 호출됩니다. 내부에서 **try/catch**로 처리하세요.
- **동기화:** 여러 스레드가 데이터 공유 시 **mutex/atomic** 등으로 보호하세요. (다음 장에서 다룰 내용)
- **join/detach:** 생성한 스레드는 끝에 **반드시 `join()` 또는 `detach()` 호출**.

---

## 7) 예제: 다양한 방식 종합

```cpp
#include <iostream>
#include <thread>
#include <functional>
#include <memory>

struct Worker {
    void run(int n) { while (n--) std::cout << "run\n"; }
    void run_const(int n) const { while (n--) std::cout << "const run\n"; }
    static void srun(int n) { while (n--) std::cout << "static run\n"; }
};

void free_fn(const std::string& msg) { std::cout << msg << "\n"; }

int main() {
    Worker w;
    std::thread a(&Worker::run, &w, 2);            // 멤버 함수 + 객체 포인터
    std::thread b(&Worker::run_const, &w, 2);      // const 멤버
    std::thread c(&Worker::srun, 2);               // static 멤버 (자유 함수처럼)

    int x = 0;
    std::thread d([&]{ x = 42; });                 // 람다 캡처(참조) — 수명·동기화 주의
    std::thread e(free_fn, std::string("hello"));  // 자유 함수

    a.join(); b.join(); c.join(); d.join(); e.join();
    std::cout << "x=" << x << "\n";
}
```

---

## 8) 트러블슈팅

- **컴파일 오류**: 멤버 함수 포인터 시그니처가 맞는지 확인 (`&Class::method`, 첫 인자는 객체).  
- **출력 순서가 섞임**: 스케줄링/버퍼링 때문에 정상입니다. 순서를 보장하려면 동기화 필요.  
- **객체 파괴 시 크래시**: 스레드 종료(`join`) 전에 객체가 스코프를 벗어나지 않도록 관리.

---

### 참고
- 이 문서는 **스레드 생성과 호출 규약**에 초점을 맞춥니다.  
- **동기화(mutex, condition_variable)**, **스레드 중지/취소**, **스레드 풀** 등은 후속 문서에서 다룹니다.
