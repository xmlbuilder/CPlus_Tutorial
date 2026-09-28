## C++ `decltype` & `decltype(auto)` 완전 정리

### 📌 1. 개념
- **`auto`** : 변수의 **타입을 추론** 하여 새로운 변수를 정의.
- **`decltype`** : **표현식(expression)** 을 받아, 그 표현식의 **타입을 가져옴**.
- **decltype(auto)** : `auto` 추론 규칙과 `decltype`의 참조/const 유지 규칙을 결합.
- decltype 은 C++11 에서 추가된 키워드로 한마디로 값에 맞는 타입을 추출해낸다



### 📌 2. 기본 사용 예
```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 10;
    const int& y = x;

    auto a = y;              // a → int (const &는 무시됨)
    decltype(y) b = x;       // b → const int& (원본 타입 그대로 유지)

    if (std::is_same<decltype(a), int>::value)
        std::cout << "a is int\n";
    if (std::is_same<decltype(b), const int&>::value)
        std::cout << "b is const int&\n";
}
```

- 출력:
```
a is int
b is const int&
```



### 📌 3. 함수 반환 타입에서의 `decltype`
```cpp
template<typename T, typename U>
auto add_auto(T a, U b) {
    return a + b;  // auto → 값 타입 (참조/const 정보 사라짐)
}

template<typename T, typename U>
decltype(auto) add_decltype(T&& a, U&& b) {
    return (a + b); // decltype(auto) → 참조/const 유지
}
```


### 📌 4. `decltype`의 괄호 규칙
`decltype(x)`와 `decltype((x))`는 다를 수 있음.

```cpp
int n = 0;

decltype(n) a = n;     // int
decltype((n)) b = n;   // int& (괄호가 있으면 lvalue 표현식 → 참조)
```



### 📌 5. 참조 반환 유지 예시
```cpp
class C {
    int data[5] = {1, 2, 3, 4, 5};
public:
    int& operator[](int i) { return data[i]; }
};

template<typename T>
decltype(auto) access(T& c, int idx) {
    return c[idx]; // 참조 유지
}

int main() {
    C obj;
    access(obj, 2) = 100;
    std::cout << obj[2] << '\n'; // 100
}
```



### 📌 6. 실무 패턴
- **Generic 프로그래밍** 에서 `decltype`은 템플릿 인자의 연산 결과 타입 추출에 사용됨.
- **Perfect forwarding** 과 결합해 참조/const를 유지.
- 예:
```cpp
template<typename T, typename U>
decltype(auto) multiply(T&& a, U&& b) {
    return std::forward<T>(a) * std::forward<U>(b);
}
```



### 📌 7. 핵심 정리
| 구분            | 참조/const 유지 | 주 용도 |
|-----------------|----------------|---------|
| `auto`          | ❌              | 단순 변수 타입 추론 |
| `decltype(expr)`| ⭕              | 표현식의 원본 타입 추출 |
| `decltype(auto)`| ⭕              | 반환 타입 추론 시 참조/const 유지 |



### 📌 8. 전체 예제
```cpp
#include <iostream>
#include <type_traits>
using namespace std;

int add(auto x, auto y) {
    return x + y;
}

class MyClass {
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
public:
    int& operator[](int i) { return arr[i]; }
};

template<typename T>
decltype(auto) changearr(T& c, int index) {
    return c[index];
}

template<typename T>
int& changearr1(T& c, int index) {
    return c[index];
}

int main() {
    int i = 1;
    int j = 1;
    add(i, j);

    if (std::is_same<decltype(i), int>::value) std::cout << "type i == int\n";
    if (std::is_same<decltype(i), float>::value) std::cout << "type i == float\n";
    if (std::is_same<decltype(i), decltype(j)>::value) std::cout << "type i == type j\n";

    MyClass myclass;
    changearr(myclass, 3) = 100;
    cout << myclass[3] << endl; // 100
    myclass[3] = 200;
    changearr1(myclass, 3) = 100;
    cout << myclass[3] << endl; // 100
}
```
---

