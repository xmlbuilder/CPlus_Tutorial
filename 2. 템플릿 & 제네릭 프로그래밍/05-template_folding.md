## 📘 Template Fold Expression 정리
### 1️⃣ Fold Expression이란?
- C++17부터 도입된 Fold Expression은 가변 인자 템플릿을 간결하게 처리할 수 있는 문법입니다.
- 복잡한 재귀 호출 없이 연산자를 기준으로 인자들을 접는(fold) 방식으로 계산합니다.

### 2️⃣ 기본 예제: 누적 합 계산
#### ▶️ 오른쪽 Fold ((t + ...))
```cpp
template<typename... T>
auto sum(T... t) {
    return (t + ...); // 오른쪽 fold
}
```
- 계산 순서: t1 + (t2 + (t3 + (...)))
- 오른쪽부터 접힘
- 출력: sum(1,2,3,4,5,6,7,8,9,10) → 55

#### ▶️ 왼쪽 Fold ((... + t))
```cpp
template<typename... T>
auto sum(T... t) {
    return (... + t); // 왼쪽 fold
}
```

- 계산 순서: (((t1 + t2) + t3) + ...)
- 왼쪽부터 접힘
- 출력 동일: 55

### 3️⃣ Fold Expression의 종류
| 표현식 | 설명 | 
|-------|------|
| (t + ...) | 오른쪽 fold | 
| (... + t) | 왼쪽 fold | 
| (t + ... + init) | 오른쪽 fold with 초기값 | 
| (init + ... + t) | 왼쪽 fold with 초기값 | 


####  초기값 예시
```cpp
template<typename... T>
auto sum(T... t) {
    return (0 + ... + t); // 왼쪽 fold with 초기값
}
```


### 4️⃣ Fold 없이 재귀로 구현한 예
```cpp
template<typename T>
constexpr auto tsum(T value) {
    return value;
}

template<typename T, typename... Targs>
constexpr auto tsum(T value, Targs... args) {
    return value + tsum(args...); // 재귀 호출
}
```

- C++11부터 가능한 방식
- 컴파일러 최적화가 잘 되지만, 코드가 복잡하고 유지보수 어려움
- 출력: tsum(1,2,3,4,5) → 15

### 5️⃣ Fold Expression의 장점
- 간결한 문법
- 컴파일러 최적화에 유리
- 재귀 깊이 제한 없음
- 다양한 연산자에 적용 가능 (+, *, &&, ||, , 등)

### 6️⃣ 다양한 Fold 활용 예시
#### 🔹 곱셈
```cpp
template<typename... T>
auto product(T... t) {
    return (1 * ... * t);
}
```

#### 🔹 논리 AND
```cpp
template<typename... T>
bool allTrue(T... t) {
    return (true && ... && t);
}
```

#### 🔹 출력 나열
```cpp
template<typename... T>
void printAll(T... t) {
    (std::cout << ... << t) << std::endl;
}
```


### 📌 요약
| 방식 | 문법 | 특징 |
|-----|-------|------|
| 오른쪽 Fold | (t + ...) | 오른쪽부터 접힘 | 
| 왼쪽 Fold | (... + t) | 왼쪽부터 접힘 | 
| 초기값 포함 Fold | (init + ... + t) | 초기값으로 시작 가능 | 
| 재귀 템플릿 | tsum(...) | C++11 방식, 복잡함 | 

----

### 📌 Fold 방식별 예제 분석
#### 1️⃣ 단항 좌측 Fold (... + nums)
```cpp
template <typename... Ints>
int sum_all(Ints... nums) {
    return (... + nums); // 좌측 Fold
}
```
- 호출: sum_all(1, 4, 2, 3, 10)
- 전개: ((((1 + 4) + 2) + 3) + 10)
- 결과: 20

#### 2️⃣ 이항 좌측 Fold (start - ... - nums)

```cpp
template <typename Int, typename... Ints>
Int diff_from(Int start, Ints... nums) {
    return (start - ... - nums); // 좌측 Fold with 초기값
}
```

- 호출: diff_from(100, 1, 4, 2, 3, 10)
- 전개: (((((100 - 1) - 4) - 2) - 3) - 10)
- 결과: 80


#### 3️⃣ 단항 좌측 Fold로 함수 호출 (obj.do_something(nums), ...)
```cpp
class A {
 public:
  void do_something(int x) const {
    std::cout << "Do something with " << x << std::endl;
  }
};

template <typename T, typename... Ints>
void do_many_things(const T& t, Ints... nums) {
  (t.do_something(nums), ...); // 좌측 Fold
}
```

- 호출: do_many_things(a, 1, 3, 2, 4)
- 전개: (((t.do_something(1), t.do_something(3)), t.do_something(2)), t.do_something(4))
- 결과:

```  
Do something with 1
Do something with 3
Do something with 2
Do something with 4
```

### 📌 Fold Expression 보강: 단항 우측 & 이항 우측

#### 1️⃣ 단항 우측 Fold (E op ...)
- 예제: sum_all_right

```cpp
#include <iostream>
template <typename... Ints>
int sum_all_right(Ints... nums) {
    return (nums + ...); // 단항 우측 Fold
}
int main() {
    std::cout << sum_all_right(1, 4, 2, 3, 10) << std::endl;
}
```

- 전개 형태: 1 + (4 + (2 + (3 + 10)))
- 계산 순서: 오른쪽부터 접힘
- 결과: 20


#### 2️⃣ 이항 우측 Fold (E op ... op I)
- 예제: diff_to
```cpp
#include <iostream>
template <typename... Ints>
int diff_to(Ints... nums) {
    return (nums - ... - 100); // 이항 우측 Fold
}
int main() {
    std::cout << diff_to(1, 4, 2, 3, 10) << std::endl;
}
```

- 전개 형태: 1 - (4 - (2 - (3 - (10 - 100))))
- 계산 순서: 오른쪽부터 접힘
- 결과: 1 - (4 - (2 - (3 - (10 - 100)))) = 1 - (4 - (2 - (3 - (-90)))) = ... = 최종 결과

- 이항 우측 Fold는 초기값이 마지막에 위치하며, 연산이 오른쪽부터 진행됩니다.

### 📌 재귀 템플릿과 비교
```cpp
template<typename Arg>
void print_all(const Arg& arg){
    std::cout << arg << std::endl;
}

template<typename Arg, typename ...Args>
void print_all(const Arg& arg, const Args&... args){
    std::cout << arg << ',';
    print_all(args...); // 재귀 호출
}
```

```
- 호출: print_all(1, 2, 3, 4, 5)
- 출력: 1,2,3,4,5
```

### 📌 Fold Expression으로 바꾸면:
```cpp
template<typename... Args>
void print_all_fold(const Args&... args){
    ((std::cout << args << ','), ...);
    std::cout << std::endl;
}
```

- 더 간결하고 유지보수 쉬움!

### 📌 요약

| 방식 | 문법 | 전개 방향 | 특징 |
|------|-----|----------|------| 
| 단항 우측 Fold | (E op ...) | 오른쪽 | 마지막부터 접힘 | 
| 단항 좌측 Fold | (... op E) | 왼쪽 | 처음부터 접힘 | 
| 이항 우측 Fold | (E op ... op I) | 오른쪽 | 초기값이 마지막에 위치 | 
| 이항 좌측 Fold | (I op ... op E) | 왼쪽 | 초기값이 처음에 위치 | 

---





