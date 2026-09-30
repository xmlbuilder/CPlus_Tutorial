## 📘 함수 인자 전달과 값 범주 유지
- C++에서 함수에 인자를 넘길 때, 그 인자가 `LValue`인지 `RValue`인지에 따라 함수의 동작이 달라질 수 있습니다.
- 특히 템플릿 함수나 오버로드 함수에서 이 차이를 정확히 유지하는 것이 중요합니다.

### 1️⃣ 기본 개념: 값 범주란?
| 표현식 | 값 범주 | 설명 |
|-------|--------|------| 
| int x = 5; | LValue | 이름이 있고 주소가 있음 | 
| x + 1 | RValue | 계산 결과, 임시 값 | 
| std::string("hi") | RValue | 임시 객체 생성 | 



### 2️⃣ 함수 인자 전달 방식
#### 🔹 일반적인 전달 방식
```cpp
void foo(int x);       // 값 복사
void bar(int& x);      // LValue Reference
void baz(int&& x);     // RValue Reference
```

| 함수 시그니처 | 받을 수 있는 인자 | 설명 |
|-------------|-----------------|-----| 
| foo(int) | LValue, RValue | 복사됨 | 
| bar(int&) | LValue만 | 참조 | 
| baz(int&&) | RValue만 | 참조 | 



### 3️⃣ RValue가 LValue가 되는 경우
```cpp
void take(int&& x) {
    x = 100; // x는 RValue Reference지만 함수 내에서는 LValue!
}
```
- x는 RValue Reference로 받았지만, 이름이 있는 변수이므로 함수 내부에서는 LValue로 간주됩니다.
- 따라서 std::move(x)를 사용하지 않으면 RValue로서의 특성이 사라집니다.

```
int a = 10;
take(std::move(a));
std::cout << a; // 100
```


### 4️⃣ 값 범주를 유지하는 방법
#### 🔹 std::move: LValue → RValue로 캐스팅
```cpp
std::string a = "hello";
std::string b = std::move(a); // a는 RValue로 취급됨
```

- std::move는 실제로 값을 이동시키지 않음
- 단지 **LValue를 RValue** 로 캐스팅 하는 역할

#### 🔹 std::forward: 템플릿 인자의 값 범주 유지
```cpp
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg)); // LValue/RValue 유지
}
```

- `T&&`는 Forwarding Reference (또는 Universal Reference)
- std::forward<T>(arg)를 써야만 원래 값 범주를 유지함

### 5️⃣ Forwarding Reference vs RValue Reference
| 선언 방식 | 의미 |
|----------|-----| 
| T&& in 템플릿 | Forwarding Reference | 
| int&& | RValue Reference | 


#### 🔹 예시
```cpp
template<typename T>
void func(T&& arg); // Forwarding Reference

void func(int&& arg); // RValue Reference
```
- `T&&`는 `LValue`도 `RValue`도 받을 수 있음
- `int&&`는 `RValue`만 받을 수 있음

### 6️⃣ 완벽한 전달 (Perfect Forwarding)
```cpp
template<typename T>
void wrapper(T&& arg) {
    realFunction(std::forward<T>(arg)); // 값 범주 유지
}
```
- wrapper는 어떤 값 범주든 받아서 realFunction에 그대로 전달
- 복사/이동/참조 모두 정확히 전달됨

### 📌 요약
| 기술 | 목적 | 사용 예 |
|--------|---------|-----------------| 
| std::move | LValue → RValue 캐스팅 | std::move(x) | 
| std::forward<T> | 값 범주 유지 (Perfect Forwarding) | std::forward<T>(arg) | 
| RValue Reference | 임시 객체 참조 | void foo(int&& x) | 
| Forwarding Reference | 템플릿에서 값 범주 유지 | template<typename T> void f(T&&) | 

----



