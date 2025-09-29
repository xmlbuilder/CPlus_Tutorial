# std::fmod
std::fmod 함수는 C++에서 부동소수점 나머지 연산을 수행할 때 사용하는 함수입니다.\
 정수형 나머지 %와는 다르게, fmod는 실수형 값에 대해 나머지를 계산합니다.

## 🧮 함수 정의
```cpp
#include <cmath>

double fmod(double x, double y);
```

- x: 나눠지는 수 (피제수)
- y: 나누는 수 (제수)
- 반환값: x를 y로 나눈 나머지 (부동소수점 기준)

## 📌 동작 방식
fmod(x, y)는 다음과 같은 수학적 정의를 따릅니다:

**fmod(x, y)** = `x - y * trunc(x / y)`


- trunc(x / y)는 소수점 이하를 버린 몫
- 결과는 x와 같은 부호를 가짐

## ✅ 예제
```cpp
#include <iostream>
#include <cmath>

int main() {
    double a = 5.3;
    double b = 2.0;

    double result = std::fmod(a, b);
    std::cout << "fmod(5.3, 2.0) = " << result << std::endl; // 출력: 1.3
}
```

- 5.3 ÷ 2.0 = 2.65 → trunc(2.65) = 2
- 5.3 - (2 × 2.0) = 1.3

## ⚠️ 주의사항
- y가 0일 경우 정의되지 않음 → 오류 발생 가능
- std::fmod는 std::remainder와 다름 → remainder는 가장 가까운 정수로 나눈 나머지를 반환
