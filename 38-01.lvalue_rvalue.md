# 📘 C++의 LValue, RValue, 그리고 RValue Reference
C++11부터 도입된 RValue Reference는 성능 최적화와 템플릿 프로그래밍의 유연성을 위해 중요한 역할을 합니다.\ 이 문서에서는 LValue와 RValue의 개념을 정리하고, RValue Reference의 특징과 함께 Move Semantics 및 Perfect Forwarding에 대해 알아봅니다.

## 1️⃣ LValue와 RValue란?
C++의 모든 표현식은 LValue 또는 RValue로 분류됩니다.
| 구분 | 설명 |
|------|-----| 
| LValue | 메모리 주소를 갖고 있으며, 지속적으로 참조 가능한 값 (예: 변수) | 
| RValue | 일시적인 값으로, 한 번 사용되고 나면 사라지는 임시 객체 (예: 3 + 4) | 


### 🔍 예시
```cpp
int x = 5;       // x는 LValue
int y = x + 2;   // x + 2는 RValue
```

- x는 메모리에 존재하며 주소를 갖는 LValue
- x + 2는 계산 결과로 생성된 RValue

### 🧠 핵심 개념
- LValue: scope 내에서 지속적으로 참조 가능
- RValue: 임시 객체, 한 번 사용 후 소멸
```cpp
class A {};
A(); // 임시 객체 → RValue
```


## 2️⃣ LValue Reference (&)
LValue Reference는 기존 변수에 **별명(Alias)**을 붙이는 방식입니다.
```cpp
int a = 10;
int& ref = a; // ref는 a의 LValue Reference
ref = 20;     // a도 20으로 변경됨
```

### 📦 구조체 예제
```cpp
#include <iostream>
#include <string>
using namespace std;

struct Person {
    string Name;
    short Age;
};

int main() {
    Person myFriend;
    Person& rFriend = myFriend;

    myFriend.Name = "Bill";
    rFriend.Age = 40;

    cout << rFriend.Name << " is " << myFriend.Age << " years old." << endl;
}
```

### 🖨️ 출력 결과:
```
Bill is 40 years old.
```

- rFriend는 myFriend의 LValue Reference
- 두 변수는 동일한 객체를 참조

### 3️⃣ RValue Reference (&&)
RValue Reference는 **임시 객체(RValue)**를 참조할 수 있게 해주는 C++11의 새로운 기능입니다.
int&& rVal = 10; // 10은 RValue → rVal은 RValue Reference

#### 🚫 잘못된 예시
int nVal = 10;
int&& rRef = nVal; // ❌ 오류: nVal은 LValue

- int&는 LValue만 참조 가능
- int&&는 RValue만 참조 가능

## 4️⃣ RValue Reference의 활용
### ✅ Move Semantics
- 복사 비용이 큰 객체를 효율적으로 이동
- std::move()를 통해 LValue를 RValue로 캐스팅
```cpp
std::string a = "Hello";
std::string b = std::move(a); // a의 자원을 b로 이동
```

- a는 더 이상 유효한 문자열을 갖지 않음
- 복사 대신 이동 → 성능 향상

### ✅ Perfect Forwarding
- 템플릿 함수에서 인자의 **값 범주(LValue/RValue)**를 유지하며 전달
- std::forward<T>(arg) 사용
```cpp
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg)); // LValue/RValue 유지
}
```


##🧾 요약

| 개념 | 설명 | 
|------|-----|
| LValue | 지속적으로 참조 가능한 값 (변수 등) | 
| RValue | 임시로 생성된 값 (연산 결과, 임시 객체 등) | 
| LValue Reference | &를 사용하여 LValue를 참조 | 
| RValue Reference | &&를 사용하여 RValue를 참조 | 
| Move Semantics | 자원 복사를 피하고 이동하여 성능 향상 | 
| Perfect Forwarding | 템플릿 인자의 값 범주를 유지하며 함수에 전달 | 


## 🔍 에러 메시지가 주의

예를 들어 이런 메시지:
'none of the 4 overloads could convert all the argument types'

이건 사실상 “너가 넘긴 인자 중 하나가 참조 타입에 안 맞아”라는 뜻인데,
rvalue를 lvalue 참조로 넘겼을 때 자주 나오는 메시지예요.
하지만 메시지 자체는 **“인자 타입이 안 맞음”**이라고만 하니까,
처음엔 “오버로드 갯수가 안 맞나?” 하고 착각하기 딱 좋죠.

## 💡 앞으로 헷갈리지 않게 보는 팁

에러 메시지를 보면 이렇게 체크해보세요:
1. 	함수 시그니처 확인
T& → 면 lvalue만 가능, const T& 면 rvalue도 가능
2. 	넘기는 값이 임시 객체인지 확인
→ func(T()), func(a+b), func(std::move(x)) 등은 rvalue
3. 	에러 메시지에 “cannot bind to non-const lvalue reference”가 있으면 100% rvalue 문제


## 🔍 넘겨주는 방식에 따른 혼선, 왜 생길까?
C++은 함수 인자 전달 방식에 따라 다른 의미와 제약을 부여합니다:

| 전달 방식 | 의미 | 주의할 점 |
|----------|-------|-----------|
| T& | 수정 가능한 lvalue 참조 | rvalue는 전달 불가 | 
| const T& | 읽기 전용 참조 | lvalue/rvalue 모두 가능 | 
| T&& | rvalue 참조 (이동 가능) | lvalue는 전달 불가 | 
| T | 값 복사 | 복사 비용, slicing 위험 | 




---



