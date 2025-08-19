# 🧮 C++에서 trunc, floor, ceil, round 함수 비교
C++ <cmath> 라이브러리에는 실수를 정수로 변환하는 다양한 함수들이 존재합니다.  그 중 trunc는 소수점 이하를 단순히 잘라내는 함수로, 다른 함수들과의 차이를 명확히 이해하는 것이 중요합니다.

## 🔹 trunc 함수란?
- trunc(x)는 소수점 이하를 버림합니다.
- 양수든 음수든 0 방향으로 잘라냅니다.
- 즉, 단순히 소수점 이하를 제거하는 것과 동일합니다.
### ✅ 예시
trunc(3.2)   → 3
trunc(3.7)   → 3
trunc(-3.2)  → -3
trunc(-3.7)  → -3



🔸 다른 함수들과의 차이점
| 값 | round | ceil | floor | trunc |
|----|-------|------|-------|-------| 
| 3.2 | 3 | 4 | 3 | 3 | 
| 3.7 | 4 | 4 | 3 | 3 | 
| -3.2 | -3 | -3 | -4 | -3 | 
| -3.7 | -4 | -3 | -4 | -3 | 


## 📌 요약
- round: 반올림 (소수점 0.5 기준)
- ceil: 올림 (무조건 큰 정수로)
- floor: 내림 (무조건 작은 정수로)
- trunc: 버림 (소수점 제거, 0 방향)
얼핏 보면 trunc와 floor가 같아 보이지만, 음수에서는 결과가 다릅니다.
예: trunc(-3.7) → -3, floor(-3.7) → -4


## 🧪 코드 예제
```cpp
#include <iostream>
#include <cmath>
using namespace std;

int main() {
    double values[] = {3.2, 3.7, -3.2, -3.7};

    cout << "origin\t round\t ceil\t floor\t trunc\n";
    for (double v : values) {
        cout << " " << v << "\t "
             << round(v) << "\t "
             << ceil(v) << "\t "
             << floor(v) << "\t "
             << trunc(v) << endl;
    }

    return 0;
}
```


## 🎯 정리
- trunc는 단순한 소수점 제거를 원할 때 사용
- floor와 trunc는 음수에서 결과가 다르므로 주의
- 실수 → 정수 변환 시, 목적에 따라 적절한 함수를 선택해야 함


