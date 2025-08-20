# 📏 sizeof vs sizeof... in C++
C에서는 객체나 타입의 크기를 구할 때 sizeof 연산자를 사용합니다.  하지만 C11부터 도입된 가변인자 템플릿에서는 sizeof...라는 새로운 연산자가 등장했습니다.  이 연산자는 파라미터 팩의 인자 개수를 구하는 데 사용됩니다.

## 🔹 sizeof 기본 사용법
```cpp
int x = 42;
std::cout << sizeof(x);        // 4 (보통 int는 4바이트)
std::cout << sizeof(int);      // 4
std::cout << sizeof(double);   // 8
```

- 타입 또는 변수의 **메모리 크기(바이트 단위)**를 반환
- 컴파일 타임에 결정됨

## 🔸 sizeof... — 파라미터 팩의 크기 구하기
```cpp
template <typename... Args>
void countArgs(Args... args) {
    std::cout << "Number of arguments: " << sizeof...(Args) << std::endl;
}
```

- sizeof...(Args)는 템플릿 파라미터 팩의 인자 개수
- sizeof...(args)는 함수 인자 팩의 개수
- 둘 다 컴파일 타임에 결정되며, 팩 확장 없이도 개수를 알 수 있음

## 🧪 예제: print() 함수 개선
```cpp
#include <iostream>
using namespace std;

void print() {
    cout << endl;
}

template <typename First, typename... Rest>
void print(const First& first, const Rest&... rest) {
    cout << first << " ";
    
    if constexpr (sizeof...(rest) > 0) {
        print(rest...); // 재귀 호출
    }
}
```

## 📌 실행 예시
```cpp
print();                            // 출력: (빈 줄)
print(1);                           // 출력: 1
print(10, 20);                      // 출력: 10 20
print("first", 2, "third", 3.14159); // 출력: first 2 third 3.14159
```


## 📊 비교표: sizeof vs sizeof...
| 연산자 | 용도 | 반환값 | 사용 시기 |
|--------|-----|-------|---------| 
| sizeof | 타입 또는 변수의 크기 | 바이트 단위 크기 | 일반 C/C++ 코드 | 
| sizeof... | 파라미터 팩의 인자 개수 | 정수 (개수) | C++11 템플릿 | 



## 🎯 요약
- sizeof...는 가변인자 템플릿에서 인자 개수를 구하는 데 사용
- sizeof는 메모리 크기를 구하는 데 사용
- sizeof...를 활용하면 재귀 종료 조건을 명확하게 처리할 수 있어, 특수화된 템플릿 없이도 유연한 함수 구현이 가능

---

