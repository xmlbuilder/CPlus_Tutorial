# 🧠 auto와 자동 형 변환 (Type Cast) 개념 정리

## 🔹 auto란?
- C++11부터 도입된 키워드로, 변수의 타입을 컴파일러가 자동으로 추론합니다.
- 초기화 값에 따라 타입이 결정됩니다.
auto x = 10;     // int
auto y = 10.5;   // double
auto z = 'A';    // char



## 🔹 자동 형 변환 (Type Cast)란?
- 서로 다른 타입의 값이 연산될 때, 컴파일러가 적절한 타입으로 변환하여 처리합니다.
- 예: int + double → double로 변환 후 연산

## 🔍 예제 분석
### ✅ 예제 1: int 리턴으로 인한 형 변환
```cpp
int add(auto x, auto y){
    return x + y;
}
```

- auto 매개변수는 double로 추론됨 (10.0, 20.5)
- 하지만 return 타입이 int로 명시되어 있어, 결과 30.5가 int로 형 변환되어 30으로 출력됨
📌 핵심 포인트: return 타입이 int이므로 double 결과가 int로 자동 형 변환(cast) 됨

### ✅ 예제 2: double 리턴으로 정확한 결과 출력
```cpp
double add(auto x, auto y){
    return x + y;
}
```

- auto는 double로 추론
- return 타입도 double이므로 형 변환 없이 30.5 그대로 출력
📌 핵심 포인트: 타입 일치 → 형 변환 없음 → 정확한 결과 출력

### ✅ 예제 3: float 리턴으로 처리
```cpp
float add(auto x, auto y){
    return x + y;
}
```

- auto는 float로 추론 (10.0f, 20.5f)
- return 타입도 float → 결과 30.5f 출력
📌 주의할 점: float은 double보다 정밀도가 낮아 소수점 오차가 발생할 수 있음

## 🧩 요약 정리
| 상황 | auto 추론 타입 | 리턴 타입 | 결과 | 형 변환 여부 | 
|------|---------------|----------|-----|------------|
| int add(auto, auto) | double | int | 30 | 있음 (double → int) | 
| double add(auto, auto) | double | double | 30.5 | 없음 | 
| float add(auto, auto) | float | float | 30.5 | 없음 | 



## 💡 팁
- auto는 매개변수 타입 추론에 유용하지만, 리턴 타입과 일치하지 않으면 형 변환이 발생할 수 있음
- 정확한 결과를 원한다면 리턴 타입을 명확히 지정하거나 템플릿 사용을 고려하세요

---
