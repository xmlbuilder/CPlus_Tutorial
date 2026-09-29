## 📘 `std::for_each` 예제

### 📌 개요

`std::for_each`는 **반복자의 범위 내 모든 요소** 에 대해 지정된 함수를 적용하는 알고리즘입니다.
- `<algorithm>` 헤더에 포함되어 있습니다.
- **함수 포인터**, **Functor(함수 객체)**, **Lambda 표현식** 을 모두 지원합니다.
- 범위의 각 요소에 대해 지정한 함수를 호출합니다.


### 📌 예제 코드

#### 함수 포인터 사용
```cpp
#include <iostream>
#include <vector>
#include <algorithm>

// 짝수만 출력하는 함수
void printEvent(int n){
    if(n % 2 == 0) std::cout << n << " ";
}

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::for_each(v.begin(), v.end(), printEvent);
    std::cout << std::endl;
    // 출력: 2 4 6 8
}
```

#### 람다 표현식 사용
```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::for_each(v.begin(), v.end(), [](int n) {
        if(n % 2 == 0) std::cout << n << " ";
    });
    std::cout << std::endl;
    // 출력: 2 4 6 8
}
```

### 📌 특징 및 활용
- `std::for_each`는 순회하면서 부수 효과(side effect)를 적용할 때 주로 사용됩니다.
- 조건부 로직을 적용하거나, 요소 가공, 출력 등에 활용됩니다.
- 반복자 범위를 변경하지 않으며, 단순 순회 목적에 적합합니다.


### 📌 요약
- `std::for_each`는 모든 요소에 지정된 함수를 적용하는 범용 순회 알고리즘
- 함수 포인터, Functor, Lambda 모두 사용 가능
- 주로 출력, 조건부 처리, 값 변환 등 부수 효과 작업에 사용

---
