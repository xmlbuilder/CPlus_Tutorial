# 🧠 C++ 함수 표현 방식 예제

이 프로젝트는 C++에서 다양한 함수 표현 방식—**일반 함수**, **Functor(함수 객체)**, **Lambda 함수**—를 `std::function`을 통해 다루는 방법을 보여줍니다.

## ✨ 주요 개념

### 1. 일반 함수 (Local Function)
```cpp
int func1(const std::string& a) {
    std::cout << "Func1 call! " << a << std::endl;
    return 0;
}
```

### 2. Functor (함수 객체)
```cpp
struct S {
    void operator()(char c) {
        std::cout << "Functor call! " << c << std::endl;
    }
};
```

### 3. Lambda 함수
```cpp
std::function<void()> f3 = []() {
    std::cout << "Lambda function call!" << std::endl;
};
```

## 고급 예제: 함수 전달 및 캡처
람다를 std::function으로 전달하고 외부 변수 캡처:

```cpp
void func1(const std::function<void(Model*)>& f, Model* model) {
    f(model);


func1([&](Model* model) {
    std::cout << model->a << std::endl;
    std::cout << a << std::endl;
}, &model);
```

## 📦 요구 사항
- C++11 이상
- <functional>, <iostream>, <string> 헤더 포함

## 🚀 실행 방법
g++ -std=c++11 function_examples.cpp -o function_examples
./function_examples


### 📌 출력 예시
Func1 call! hello
Functor call! c
Lambda function call!
7
100
10

---

### 📄 `function_examples.cpp`

```cpp
#include <functional>
#include <iostream>
#include <string>

// 일반 함수
int func1(const std::string& a) {
    std::cout << "Func1 call! " << a << std::endl;
    return 0;
}

// Functor (함수 객체)
struct S {
    void operator()(char c) {
        std::cout << "Functor call! " << c << std::endl;
    }
};

int main() {
    // std::function을 통해 다양한 함수 표현 방식 사용
    std::function<int(const std::string&)> f1 = func1;
    std::function<void(char)> f2 = S();
    std::function<void()> f3 = []() {
        std::cout << "Lambda function call!" << std::endl;
    };

    f1("hello");
    f2('c');
    f3();

    // 람다를 통한 수치 연산
    std::function<int(int, int)> f4 = [](int x, int y) {
        return x + y;
    };
    std::cout << f4(3, 4) << std::endl;

    // 외부 변수 캡처 및 객체 전달
    int a = 10;
    struct Model {
        int a {100};
        int b {1};
        int c {2};
    } model;

    auto f5 = [&](Model* m) {
        std::cout << m->a << std::endl;
        std::cout << a << std::endl;
    };

    std::function<void(Model*)> f6 = f5;

    auto func2 = [](const std::function<void(Model*)>& f, Model* m) {
        f(m);
    };

    func2(f6, &model);

    return 0;
}
```

