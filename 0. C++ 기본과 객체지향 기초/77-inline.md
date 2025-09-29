# ✨ inline의 의미와 역할
## 1. 함수 호출을 줄이는 힌트
- inline 함수는 함수 호출을 생략하고, 호출 위치에 코드 자체를 삽입하라는 힌트예요.
- 하지만! 컴파일러가 반드시 따라야 하는 건 아님 → 최적화 판단에 따라 무시될 수 있어요.
```cpp
inline int add(int a, int b) {
    return a + b;
}
```

- 위 함수는 호출될 때마다 a + b가 직접 삽입될 수 있어요 → 성능 향상 가능
- 단, 너무 큰 함수는 오히려 코드 부풀림으로 성능 저하 가능

## 2. ODR(One Definition Rule) 해결
- inline은 헤더 파일에 함수 정의를 넣을 수 있게 해주는 키워드예요.
- 일반적으로 함수 정의는 .cpp에 하나만 있어야 하지만,
inline을 붙이면 여러 translation unit에서 정의되어도 OK!
```cpp
// header.h
inline void greet() {
    std::cout << "Hello!" << std::endl;
}
```

- 이건 여러 .cpp 파일에서 include해도 링커 에러가 발생하지 않음

### 3. 클래스 내부 정의는 자동 inline
```cpp
class MyClass {
public:
    void sayHi() {
        std::cout << "Hi!" << std::endl;
    }
};
```

- 클래스 내부에서 정의된 멤버 함수는 자동으로 inline 처리돼요
- 굳이 inline 키워드를 붙이지 않아도 돼요

🧠 inline vs static vs constexpr

|키워드|목적|메모리 영향|링커 영항|
|-----|----|---------|--------|
|inline|함수 호출 생략 + ODR 해결|없음|다중 정의 허용|
|static|파일 스코프 제한|데이터영역|외부 링크 방지|
|constexpr|컴파일 타임 계산|없음|컴파일러가 값으로 대체|


## ⚠️ 주의할 점
- inline은 성능 향상 목적보다는 ODR 해결용으로 더 자주 사용됨
- 너무 많은 inline 함수는 코드 부풀림으로 인해 캐시 미스나 디버깅 어려움을 초래할 수 있음
- inline 함수는 디버깅 시 스택 트레이스에 안 나올 수도 있음

----
