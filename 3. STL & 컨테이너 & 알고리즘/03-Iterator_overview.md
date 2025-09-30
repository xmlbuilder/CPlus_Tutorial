
# 🔁 C++ STL 반복자(Iterator) 정리

반복자는 **컨테이너의 요소를 가리키는 포인터 유사 객체** 로, 컨테이너의 구체 구현을 몰라도 요소에 **추상적으로 접근** 할 수 있게 해줍니다.  
반복자는 표준 **알고리즘**과 컨테이너 사이를 이어주는 다리.

![Iterator Bridge](/image/iterator_bridge.svg)

---

## 1) 핵심 개념

- 포인터와 유사하게 `*`, `++`, `--`, `==`, `!=` 연산을 지원
- 대부분의 컨테이너는 반복자를 제공하고, **반복자 기반** 으로 순회합니다.  
  (`stack`, `queue` 어댑터는 직접적인 반복자 순회를 제공하지 않음)
- `begin()`은 첫 요소, `end()`는 **마지막 다음 위치** (한 칸 뒤)를 가리킵니다.

### 시퀀스 위에서의 이동
![Iterator Sequence](/image/iterator_sequence.svg)

---

## 2) 반복자 종류

![Iterator Categories](/image/iterator_categories.svg)

- **Input / Output**: 단방향 읽기/쓰기
- **Forward**: 여러 번 읽기 가능, 전방 이동
- **Bidirectional**: 전/후진 가능 (`list`, `set`, `map` 등)
- **Random Access**: 인덱스 산술 가능 (`vector`, `deque`, `array`)

---

## 3) 기본 사용 예시

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3};
    std::vector<int>::iterator it = vec.begin();
    for (; it != vec.end(); ++it) {
        std::cout << *it << "\n";
    }
}
```

### 다양한 반복자

```cpp
#include <vector>
#include <set>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3};

    // 1) 수정 가능한 반복자
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        *it *= 2; // 값 변경
    }

    // 2) const 반복자 (읽기 전용)
    for (auto cit = vec.cbegin(); cit != vec.cend(); ++cit) {
        std::cout << *cit << " ";
    }

    // 3) 역방향 반복자
    for (auto rit = vec.rbegin(); rit != vec.rend(); ++rit) {
        std::cout << *rit << " ";
    }

    // 4) 범위 기반 for (반복자 문법 단축)
    std::set<char> ch {{'A','B','C','D'}};
    for (char c : ch) std::cout << c << " ";
}
```

---

## 4) 반복자의 연산자

- `++it`, `it++` : 다음 요소 이동 (전위가 보통 더 효율적)
- `--it` : 이전 요소 이동 (Bidirectional 이상)
- `*it` : 역참조, 요소 값 접근
- `it1 == it2`, `it1 != it2` : 동일 위치 비교
- Random Access 전용: `it + n`, `it - n`, `it[n]`, `it2 - it1`

---

## 5) 표준 알고리즘과의 결합 예시

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> arr1 = {1, 3, 5, 11, 9, 13, 5, 15};
    std::vector<int> arr2 = {3, 6, 12, 9, 15};

    std::sort(arr1.begin(), arr1.end());
    std::sort(arr2.begin(), arr2.end());

    std::vector<int> results;
    results.resize(std::min(arr1.size(), arr2.size()));
    auto it_end = std::set_intersection(arr1.begin(), arr1.end(),
                                        arr2.begin(), arr2.end(),
                                        results.begin());
    results.resize(std::distance(results.begin(), it_end));

    for (int v : results) std::cout << v << "\n";
    // 3, 9, 15
}
```

---

## 6) 주의할 점 (중요)

- **반복자 무효화(Invalidation)**  
  - `vector`, `deque`: 용량 재할당(reallocation) 또는 중간 삽입/삭제 시 기존 반복자 무효화
  - `list`, `forward_list`: 노드 단위라 **다른 노드의 반복자는 유지**, 해당 노드만 무효화
  - `map`, `set`: 요소 삭제된 위치의 반복자만 무효화 (트리 재구성에도 안전)

- **erase 사용 패턴**
```cpp
for (auto it = vec.begin(); it != vec.end(); /* no ++ */) {
    if (*it % 2 == 0) it = vec.erase(it); // erase가 다음 유효 반복자 반환
    else ++it;
}
```

- **const 안전성**  
  `cbegin()`, `cend()`, `crbegin()`, `crend()`로 읽기 전용 보장

---

## 7) 용어 정리
- **역참조(dereference)**: 반복자가 가리키는 요소 값을 읽거나 쓰는 것 (`*it`)
- **범위(range)**: `[begin, end)` 반개구간, `end()`는 **실제 요소를 가리키지 않음**

---
