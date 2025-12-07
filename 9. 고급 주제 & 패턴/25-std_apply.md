## 📌 std::apply란?
- 정의: std::apply는 튜플을 함수 인자로 풀어서 호출해주는 함수 템플릿입니다.
- 형식:
```cpp
std::apply(f, t);
```
- 여기서
  - f = 호출할 함수(또는 함수 객체)
  - t = std::tuple (혹은 std::pair)
- 즉, tuple 안에 들어 있는 값들을 자동으로 꺼내서 f에 전달해 줍니다.

## ✨ 간단 예시
```cpp
#include <tuple>
#include <iostream>

int add(int a, int b) {
    return a + b;
}
```
```cpp
int main() {
    auto args = std::make_tuple(3, 4);

    // tuple을 add 함수 인자로 풀어서 호출
    int result = std::apply(add, args);

    std::cout << result << "\n"; // 7
}
```
- ➡️ std::apply(add, args)는 내부적으로 add(3, 4)를 호출한 것과 같습니다.
### 🔎 구조적 바인딩과 비교
- 구조적 바인딩:
```cpp
auto [x, y] = args;
int result = add(x, y);
```
- std::apply:
```cpp
int result = std::apply(add, args);
```
- 둘 다 같은 효과지만, std::apply는 인자 개수가 많거나 가변적일 때 훨씬 깔끔합니다.

### 📌 활용 패턴- 범용 함수 호출
```cpp
template <typename Func, typename Tuple>
auto call(Func f, Tuple t) {
    return std::apply(f, t);
}
```
-  어떤 함수든 튜플로 인자를 묶어서 호출 가능.
- 람다와 함께
```cpp
auto tup = std::make_tuple(1, 2, 3);
std::apply([](auto a, auto b, auto c) {
    std::cout << a+b+c << "\n";
}, tup);
```
- CAD/시뮬레이션 예시
  - 좌표 (x,y,z)를 튜플로 저장해 두고, 특정 연산 함수에 바로 전달:
```cpp
auto point = std::make_tuple(1.0, 2.0, 3.0);
auto length = std::apply([](double x, double y, double z) {
    return std::sqrt(x*x + y*y + z*z);
}, point);
```

## 📌 정리
- std::apply = 튜플 → 함수 인자 풀어주기
- 반복적인 std::get<>() 호출 없이 깔끔하게 함수에 전달 가능
- CAD, DB, 시뮬레이션처럼 데이터를 튜플로 묶어두고 함수에 넘겨야 하는 상황에서 특히 유용

---
## 실전 예시

**optional + tuple + apply** 를 한 번에 활용하는 실전 예시를 CAD에 적용

### 📌 시나리오
- CAD 프로그램에서 두 선분의 교차점을 계산하는 함수가 있다고 합시다.
- 교차점이 없을 수도 있으니 std::optional로 반환합니다.
- 교차점이 있다면 (x, y, z) 좌표를 std::tuple로 묶어 반환합니다.
- 이후 std::apply를 사용해 튜플을 함수 인자로 풀어내어 후처리(예: 거리 계산)를 합니다.

### ✨ 코드 예시
```cpp
#include <iostream>
#include <tuple>
#include <optional>
#include <cmath>
```
```cpp
// 교차점 계산 (없으면 nullopt)
std::optional<std::tuple<double, double, double>>
computeIntersection(int lineId1, int lineId2) {
    if (lineId1 == lineId2) {
        return std::nullopt; // 같은 선분이면 교차 없음
    }
    // 예시: 단순히 (1.0, 2.0, 3.0) 좌표 반환
    return std::make_tuple(1.0, 2.0, 3.0);
}
```
```cpp
// 튜플을 받아서 거리 계산
double computeDistanceFromOrigin(const std::tuple<double,double,double>& point) {
    return std::apply([](double x, double y, double z) {
        return std::sqrt(x*x + y*y + z*z);
    }, point);
}
```
```cpp
int main() {
    auto intersection = computeIntersection(1, 2);

    if (!intersection) {
        std::cout << "No intersection found\n";
    } else {
        auto dist = computeDistanceFromOrigin(*intersection);
        std::cout << "Intersection at distance " << dist << " from origin\n";
    }
}
```


## 📌 실행 결과
```
Intersection at distance 3.74166 from origin
```

## 🔎 포인트 정리
- optional: 교차점이 없을 때 nullopt로 표현 → 0.0과 구분 가능.
- tuple: 교차점 좌표 (x,y,z)를 하나의 값으로 묶어 반환.
- apply: 튜플을 풀어서 람다에 전달 → 깔끔하게 (x,y,z)를 인자로 사용.

- 👉 이렇게 하면 CAD 같은 환경에서 데이터 존재 여부 + 좌표 묶음 + 함수 호출을 모두 Modern C++ 스타일로 처리할 수 있습니다.

---

