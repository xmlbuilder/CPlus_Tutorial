
## 📘 람다(Lambda) 정리

- C++11에서 **람다(lambda)** 는 익명 함수(Anonymous Function)를 만드는 새로운 문법입니다.  
- 람다는 이름이 없는 함수이며, **STL 함수 객체(Functor)** 처럼 동작합니다.


### 📌 람다 기본 구조 그림
![람다구조](/image/lambda.png)


### 📌 1. 람다의 기본 구조

```cpp
[capture](parameter_list) mutable -> return_type {
    // body
}
```

| 구성 요소 | 설명 |
|-----------|------|
| **[capture]** | 외부 변수 캡처 방식 지정 |
| **(parameter_list)** | 매개변수 목록 |
| **mutable** | 값 캡처 시 내부 변수 변경 허용 |
| **-> return_type** | 반환 타입 지정 |
| **{ body }** | 함수 본문 |

---

### 📌 2. 캡처(Capture)

- 람다는 외부 스코프 변수에 접근하기 위해 캡처를 사용합니다.

| 구문 | 설명 |
|------|------|
| `[=]` | 모든 외부 변수를 값(value)으로 캡처 |
| `[&]` | 모든 외부 변수를 참조(reference)로 캡처 |
| `[a, &b]` | a는 값, b는 참조로 캡처 |
| `[=, &a]` | a만 참조, 나머지는 값 |
| `[&, a]` | a만 값, 나머지는 참조 |

#### 🔹 예시:

```cpp
int a = 1, b = 2;

[=]() { std::cout << a << b; }();   // 값 복사
[&]() { b = 10; }();               // 참조로 수정 가능
```

```cpp
int main(){
    sum(10,10);
    int c = [](int a, int b){
        std::cout<< "Sum:" << a+b <<std::endl;
        return a + b;
    }(30,40);

    std::cout << c << std::endl;
    
    auto ret = [](int a, int b){
        std::cout<< "Sum:" << a+b <<std::endl;
        return std::tuple<int,int,int>{a,b,a+b};
    }(30,40);

    int a = std::get<0>(ret);
    int b = std::get<1>(ret);
    int c = std::get<2>(ret);

    std::cout<< a << "," << b << "," << c <<std::endl;

    int result1 = 1;
    int result2 = 2;
    int result3 = 3;
    int result4 = 4;

    //변수복사
    [result1, result2](int a, int b){
        std::cout<< "Result:" << result1 + result2 << std::endl;
    }(10,20);


    //변수참조
    [&result3, &result4](inta,intb){
        std::cout<< "Result:" << result3 + result4 << std::endl;
        result3 = 100;
        result4 = 200;
    }(10,20);

    //전체복사
    [=](int x){
        std::cout<< "Result1,2" << result1<<"," << result2 << std::endl;
        std::cout<< "Result3,4" << result3<<"," << result4 << std::endl;
    }(30);
    

    //전체참조
    [&](int x){
        result1 = 10;
        result2 = 20;
        result3 = 30;
        result4 = 40;
        std::cout<<"Result1,2"<< result1 <<","<< result2 <<std::endl;
        std::cout<<"Result3,4"<< result3 <<","<< result4 <<std::endl;
    }(30);

    std::cout<<"Result1,2"<<result1<<","<<result2<<std::endl;
    std::cout<<"Result3,4"<<result3<<","<<result4<<std::endl;

    //전체복사 일부참조
    [=,&result3,&result4](intx){
        //result1=10;
        //result2=20;
        result3=30;
        result4=40;
        std::cout<<"Result1,2"<<result1<<","<<result2<<std::endl;
        std::cout<<"Result3,4"<<result3<<","<<result4<<std::endl;
    }(30);
    return0;
}
```

### 📌 3. mutable 키워드

- 값(value) 캡처 시, 기본적으로 변수는 변경할 수 없습니다.  
- `mutable`을 사용하면 람다 내부에서 값 캡처 변수를 수정할 수 있습니다.

```cpp
int x = 10;
[=]() mutable { x = 20; }(); // 내부에서 수정 가능 (외부 값에는 영향 없음)
```

### 📌 4. 예외 명시 (throw)

- 람다 뒤에 `throw()`를 붙이면 예외를 던지지 않겠다는 의미입니다.

```cpp
[]() throw() { /* 예외 없음 */ };

[]() throw(int) { throw 42; }; // int 예외 가능
```


### 📌 5. 반환 타입(Return Type)

`-> 타입` 형태로 반환 타입을 지정할 수 있습니다.

```cpp
auto sum = [](int a, int b) -> int { return a + b; };
```

- 반환 타입을 생략하면 `return`문의 타입을 추론합니다.


### 📌 6. std::function과 람다

- 람다는 `std::function`과 함께 사용해 고차 함수(Higher-order function) 구현이 가능합니다.

```cpp
#include <functional>
std::function<int(int)> f = [](int x) { return x + 1; };
```

### 📌 7. 함수 인자로 람다 전달

```cpp
#include <algorithm>
#include <vector>

std::vector<int> v = {5, 2, 8, 1};
std::sort(v.begin(), v.end(), [](int a, int b) {
    return a < b;
});
```

### 📌 8. Function 과 연동

```cpp
#include <functional>
int foo_return(int para) {
    return para;
}

int lambda_test4() {
// std::function wraps a function that take int parameter and returns int value
    std::function<int(int)> func = foo_return;

    int important = 10;
    std::function<int(int)> func2 = [&](int value) -> int {
        return 1 + value + important;
    };
    
    std::cout << func(10) << std::endl;
    std::cout << func2(10) << std::endl;
}
```
```cpp
#include <iostream>
#include <functional>

std::function<int(int)> retFunc (int& a) {
    return [&a](int b) {
        return a+b;
    };
}

int main() {
    int a = 20;
    std::function<int(int)> ret = retFunc(a);
    std::cout << ret(10) << std::endl;
    //30
    a = 40;
    std::cout << ret(10) << std::endl;
    //50
    return 0;
}
```

- 람다식을 함수 전달에 실용적으로 쓰는 방법
```cpp
#include <iostream>
typedef int (*func) (int a, int b);
void funCall(func f, int a, int b) {
    std::cout << f(a, b) << std::endl;
}
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

void main() {
    int a = 1;
    int b = 2;
    funCall([](int a, int b) -> int{ return a + b; }, a, b);
    func funAdd = add;
    func funSub = sub;
    funCall(funAdd, a, b);
    funCall(funSub, a, b);
}
```
```cpp
#include <iostream>
typedef int (*func) (int a, int b);
func getFunction(std::string name) {
    if (name == "add") {
        return [](int a, int b)->int {return a + b;};
    }
    else if (name == "sub") {
        return [](int a, int b)->int {return a - b;};
    }
    else {
        return [](int a, int b)->int {return 0;};
    }
}
void main() {
    int a = 1;
    int b = 2;
    func f = getFunction("add");
    std::cout << f(a, b) << std::endl;
}
```
```cpp
#include <functional>
#include <iostream>
typedef std::function<double(double)> FuncDouble;
double funcTest(FuncDouble f) {
    return f(10);
}
int main(void) {
    std::function<int(int)> f1 = [](int x) -> int {
        return x + 1;
    };
    std::cout << f1(10) << std::endl;
    std::cout << funcTest([](double d )->double {
        return  d * 20;
    }) << std::endl;
    return 0;
}
```
```cpp
#include <functional>
#include <iostream>
typedef std::function<double(double)> FuncDouble;
double funcTest(FuncDouble f) {
    return f(10);
}
int main(void) {
    std::function<int(int)> f1 = [](int x) -> int {
        return x + 1;
    };
    std::cout << f1(10) << std::endl;
    double doub1 = 32;
    
    std::cout << funcTest([&](double d )->double {
        return  d * doub1;
    }) << std::endl;
    
    return 0;
}
```

### 📌 9. Auto로 람다식 다루기

```cpp
#include <iostream>
#include <memory>

int main(){

    int a = 12;
    auto func1 = [=](int b, int c) {
        std::cout << "a = " << a << ", b = " << b << ", c = " << c << std::endl;
    };

    auto func2 = [&](int b, int c) {
        std::cout << "a = " << a << ", b = " << b << ", c = " << c << std::endl;
    };

    func1(20, 30); //a = 12, b = 20, c = 30
    func2(20, 30); //a = 12, b = 20, c = 30

    a = 24;
    func1(30, 40); //a = 12, b = 30, c = 40
    func2(30, 40); //a = 24, b = 30, c = 40
}
```

### 📌 10. 정리

- 람다는 함수 객체의 문법적 설탕(syntactic sugar)
- 외부 변수 접근을 위해 **캡처** 사용
- `mutable`로 값 캡처 수정 가능
- `throw()`로 예외 명시 가능
- 반환 타입 지정 가능 (`-> type`)
- STL 알고리즘과 함께 자주 사용됨

---

