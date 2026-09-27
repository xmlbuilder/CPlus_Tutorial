## 🔍 C++ STL `std::binary_search`와 사용자 정의 비교 함수

### 📌 개요
- `std::binary_search`는 **정렬된 범위** 에서 특정 값이 존재하는지 빠르게 확인하는 알고리즘입니다.  
- 이진 탐색을 사용하므로 **시간 복잡도는 O(log N)** 입니다.
- 기본적으로 `<` 연산자를 이용해 비교하지만, 사용자 정의 비교 함수를 제공할 수도 있습니다.

---

### 📌 기본 형태
```cpp
#include <algorithm>

bool std::binary_search(
    ForwardIt first, ForwardIt last,
    const T& value,
    Compare comp
);
```

- `first`, `last` : 검색 범위를 지정하는 반복자 (정렬되어 있어야 함)
- `value` : 찾으려는 값
- `comp` : 비교 함수(선택). `bool comp(const T& a, const T& b)` 형식

---

#### 🛠 예제 1 — 함수 템플릿 사용
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

template <typename T>
bool compare(T s1, T s2) {
    return (s1 == s2);
}

int main() {
    std::vector<std::string> v = {"1", "2", "3"};

    bool result = std::binary_search(v.begin(), v.end(), "1", compare<std::string>);
    std::cout << std::boolalpha << result << std::endl; // true
    return 0;
}
```

#### 🛠 예제 2 — 람다식 사용
람다식을 사용하면 비교 함수를 코드 내부에 간결하게 작성할 수 있습니다.

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

int main() {
    std::vector<std::string> v = {"1", "2", "3"};

    bool result = std::binary_search(
        v.begin(), v.end(), "1",
        [](const std::string& s1, const std::string& s2) {
            return (s1 < s2);
        }
    );

    std::cout << std::boolalpha << result << std::endl; // true
    return 0;
}
```

### ⚠️ 주의  
> `std::binary_search`의 비교 함수는 `==` 대신 `<`를 이용하는 **정렬 기준** 이어야 합니다.  
> 위 예제에서 `==`로만 비교하면 올바르게 동작하지 않을 수 있습니다 (정렬된 순서 판단이 불가능해지기 때문).


### 📌 요약
- `std::binary_search`는 **정렬된 데이터** 에서 사용해야 함
- 비교 함수는 반드시 **정렬 기준(`<`)** 을 제공해야 함
- 람다식을 사용하면 간결하고 가독성 있는 코드 작성 가능

---
