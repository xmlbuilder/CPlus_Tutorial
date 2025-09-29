# template 활용하기

## 🧠 첫 번째 코드: print_type_info
```cpp
template<typename T>
auto print_type_info(const T& t){
    if constexpr(std::is_integral<T>::value){
        return t + 1;
    }
    else{
        return t + 0.001;
    }
}
```

### 🔍 핵심 개념
- template<typename T>: 타입에 따라 동작을 다르게 하도록 만든 함수 템플릿.
- if constexpr: 컴파일 타임 조건문. T가 정수형이면 t + 1, 아니면 t + 0.001.
- std::is_integral<T>::value: T가 int, long, char 같은 정수형인지 판별.
### 🧪 실행 결과
```cpp
std::cout << print_type_info(5) << std::endl;       // 6 (정수니까 +1)
std::cout << print_type_info(3.1415) << std::endl;  // 3.1425 (실수니까 +0.001)
```


### ✅ 요약
이 함수는 입력값의 타입에 따라 정수면 +1, 실수면 +0.001을 더해주는 타입 기반 조건 처리 예제예요.

## 🧠 두 번째 코드: add2
```cpp
template<typename T, typename U>
auto add2(T x, T y) -> decltype(x + y){
    return x + y;
}
```

### 🔍 핵심 개념
- template<typename T, typename U>: 두 개의 타입을 받는 템플릿.
- decltype(x + y): x + y의 타입을 자동 추론해서 반환 타입으로 지정.
- 하지만 실제로는 T와 U를 받지만, x와 y는 둘 다 T 타입으로 선언되어 있어요.

### 🧪 실행 결과
```cpp
auto ret = add2<int, int>(3, 3); // 3 + 3 = 6
std::cout << ret << std::endl;  // 출력: 6
```

### 🧪 T와 U를 모두 활용하는 add2 템플릿
```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename U>
auto add2(T x, U y) -> decltype(x + y) {
    return x + y;
}

int main() {
    auto a = add2(3, 4.5);         // int + double → double
    auto b = add2(2.5f, 7);        // float + int → float
    auto c = add2(100L, 200);      // long + int → long

    std::cout << a << std::endl;  // 7.5
    std::cout << b << std::endl;  // 9.5
    std::cout << c << std::endl;  // 300

    return 0;
}
```


### ✅ 설명
- T x, U y: 서로 다른 타입의 인자를 받음.
- decltype(x + y): 두 타입의 합산 결과 타입을 자동 추론.
- 다양한 타입 조합을 테스트할 수 있음.

### 💡 확장 아이디어
이걸 더 발전시키면, 타입이 다를 때 경고를 출력하거나, 특정 타입 조합만 허용하는 식으로 제약을 걸 수 있음:
```cpp
template<typename T, typename U>
auto add2_checked(T x, U y) -> decltype(x + y) {
    static_assert(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value,
                  "add2_checked only supports arithmetic types");
    return x + y;
}
```
이렇게 하면 std::string 같은 비수치 타입을 넣었을 때 컴파일 에러가 나서 실수를 방지할 수 있어.


### 🧩 전체 정리
| 함수 이름 | 목적 | 타입 처리 방식 | 결과 예시 |
|----------|------|-------------|----------} 
| print_type_info | 타입에 따라 다른 값을 더함 | if constexpr + std::is_integral | 5 → 6, 3.1415 → 3.1425 | 
| add2 | 두 값을 더함 | decltype(x + y)로 반환 타입 추론 | 3 + 3 → 6 | 


---

