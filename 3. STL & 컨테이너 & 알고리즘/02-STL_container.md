## 📘 C++ STL 컨테이너 종류 및 특징

- C++ 표준 라이브러리(STL)는 데이터를 효율적으로 저장하고 관리할 수 있도록 다양한 **컨테이너(Container)** 를 제공합니다.  
- 컨테이너는 크게 **4가지 범주**로 나눌 수 있습니다.


### 📌 1. Sequential 컨테이너 (순차 컨테이너)
삽입한 **순서** 를 유지하며 요소를 저장하는 컨테이너입니다.

| 컨테이너      | 특징 |
|--------------|------|
| `array`      | 고정 크기 배열, 컴파일 타임에 크기 결정, 빠른 접근 |
| `vector`     | 동적 배열, 임의 접근(Random Access) 가능, 맨 뒤 삽입/삭제 빠름 |
| `list`       | 이중 연결 리스트, 양방향 순회, 중간 삽입/삭제 빠름 |
| `forward_list` | 단일 연결 리스트, 메모리 절약, 전방 순회만 가능 |
| `deque`      | 양쪽 끝 삽입/삭제 모두 빠름, 내부적으로 블록 구조 |

#### ✅ 예제
```cpp
#include <list>
#include <iostream>

int main() {
    std::list<int> myList{7, 9, 3, 4, 1};
    for (int val : myList)
        std::cout << val << " ";
}
```

### 📌 2. Associative 컨테이너 (연관 컨테이너)
- 정렬된 규칙(트리 구조) 또는 해시 기반으로 저장하며, **검색에 최적화** 되어 있습니다.

| 컨테이너     | 특징 |
|-------------|------|
| `set`       | 중복 없는 키 저장, 자동 정렬 |
| `multiset`  | 중복 허용, 자동 정렬 |
| `map`       | 키-값 쌍 저장, 키 중복 불가, 자동 정렬 |
| `multimap`  | 키-값 쌍 저장, 키 중복 허용, 자동 정렬 |

#### ✅ 예제 (map)
```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    std::map<std::string, int> m1{
        {"1", 1},
        {"2", 2}
    };
    m1.insert(std::make_pair("3", 3));

    std::cout << m1.count("1") << std::endl; // 1
    std::cout << m1.count("4") << std::endl; // 0
}
```

#### ✅ set 삽입 결과 확인
```cpp
#include <set>
#include <iostream>

int main() {
    std::set<int> mySet{1, 2, 3};
    auto result = mySet.insert(2);
    if (!result.second)
        std::cout << "이미 존재하는 값입니다: " << *result.first << std::endl;
}
```


### 📌 3. Unordered 컨테이너 (비정렬 해시 컨테이너)
- 해시 기반으로 동작하며, 요소의 순서나 정렬이 보장되지 않습니다.  
- 검색/삽입 평균 시간복잡도는 **O(1)** 입니다.

| 컨테이너              | 중복 | 정렬 | 사용 예 |
|-----------------------|------|------|--------|
| `unordered_set`       | ❌ | ❌ | 중복 없는 키 저장 |
| `unordered_multiset`  | ✅ | ❌ | 중복 허용 |
| `unordered_map`       | ❌ | ❌ | 키-값 저장 |
| `unordered_multimap`  | ✅ | ❌ | 키-값 중복 허용 |



### 📌 4. 컨테이너 어댑터 (Container Adapter)
- 기존 컨테이너를 기반으로 동작을 제한하거나 특정 기능에 특화시킨 구조입니다.

| 어댑터            | 내부 컨테이너 (기본) | 특징 |
|-------------------|----------------------|------|
| `stack`           | `deque`              | 후입선출(LIFO) |
| `queue`           | `deque`              | 선입선출(FIFO) |
| `priority_queue`  | `vector`             | 우선순위 기반 정렬 |


### 📌 사용자 정의 타입 사용 시 주의사항
- 컨테이너에 사용자 정의 타입을 저장할 경우, 다음 조건을 만족해야 합니다.

#### ✅ 복사 및 대입 가능  
- 복사 생성자와 대입 연산자가 필요  
- 포인터 멤버가 없고 기본 동작이면 자동 생성됨

#### ✅ 비교 연산자 제공  
- `set`, `map` 등 **정렬 기반 컨테이너**: `operator<` 필요  
- `unordered_set`, `unordered_map` 등 **해시 기반 컨테이너**:  
  해시 함수(`std::hash` 특수화)와 `operator==` 필요  

#### 예제: 사용자 정의 타입을 `set`에 저장
```cpp
#include <set>

struct Point {
    int x, y;
    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

int main() {
    std::set<Point> pointSet;
    pointSet.insert({1, 2});
    pointSet.insert({2, 3});
}
```



### 📌 요약
- **Sequential**: 순서 유지, 반복자 기반 순회 (`vector`, `list` 등)
- **Associative**: 자동 정렬 기반 저장 (`set`, `map` 등)
- **Unordered**: 해시 기반 저장, 빠른 검색
- **Adapter**: 제한된 인터페이스 제공 (`stack`, `queue` 등)
- 사용자 정의 타입: **복사 가능 + 비교 가능** 조건 필수

---

