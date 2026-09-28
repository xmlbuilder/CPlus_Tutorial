## std::shuffle
- std::shuffle은 C++11부터 도입된 함수인데, 기존의 random_shuffle보다 더 안전하고 유연한 방식으로 컨테이너의 요소를 무작위로 섞는 함수입니다.

## 📘 std::shuffle이란?
- 헤더: <algorithm>
- 기능: 컨테이너의 요소들을 무작위로 섞음
- 필요한 것: 반드시 난수 생성기를 함께 전달해야 함 (std::mt19937 등)

### 📌 기본 사용법
```cpp
#include <algorithm>
#include <vector>
#include <random>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister 엔진

    std::shuffle(v.begin(), v.end(), gen);

    for (int n : v)
        std::cout << n << " ";
}
```

#### 🔹출력 예:
```
3 1 5 2 4  // 실행할 때마다 달라짐
```


### 📌 std::shuffle vs random_shuffle
| 항목               | `random_shuffle`           | `std::shuffle`                  |
|--------------------|----------------------------|----------------------------------|
| 도입 시기          | C++98                      | C++11                            |
| 난수 생성기 지정   | 불가능                     | 가능 (`std::mt19937` 등)         |
| 안전성             | 낮음 (내부 구현에 의존)    | 높음 (사용자 지정 엔진 사용 가능) |
| 현재 상태          | C++17부터 **제거됨**       | 표준으로 사용됨                  |
| 대표 난수 엔진     | 없음                       | `std::mt19937` (Mersenne Twister) |


### 📌 팁
- std::shuffle은 보안적으로 안전한 난수를 쓰고 싶다면 std::random_device와 함께 쓰면 좋음.
- 게임, 시뮬레이션, 로또, 카드 섞기 등에 자주 쓰입니다.

---
