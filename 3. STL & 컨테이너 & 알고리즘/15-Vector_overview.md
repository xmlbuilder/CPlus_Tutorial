## 📚 C++ STL `std::vector` 정리

### 📌 개요

- `std::vector`는 C++ STL에서 제공하는 **가변 크기 배열 컨테이너** 입니다.  
- 동적 메모리를 사용하여 크기를 자유롭게 조절할 수 있으며, 배열처럼 요소에 직접 접근할 수 있습니다.

```cpp
#include <vector>
std::vector<int> vec = {1, 2, 3};
```

- 요소 접근은 **O(1)** 시간 복잡도
- `push_back`, `pop_back`으로 끝에 요소 추가/제거
- `insert`, `erase`로 임의 위치 조작 가능
- **메모리 재할당 시 기존 반복자가 무효화** 될 수 있음


### 📌 기본 사용법

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> vec{1, 2, 3, 4, 5};

    vec.push_back(6);     // 끝에 요소 추가
    vec.pop_back();       // 끝 요소 제거

    vec.insert(vec.begin() + 2, 7);  // 인덱스 2 위치에 7 삽입

    for (auto itr = vec.begin(); itr != vec.end(); ++itr)
        std::cout << *itr << std::endl;
}
```

#### 출력 결과
```
1
2
7
3
4
5
```


### 📌 예외 처리

```cpp
try {
    vec.at(-1);  // 잘못된 인덱스 접근
} catch (std::exception& e) {
    std::cout << "예외 발생: " << e.what() << std::endl;
}
```

- `at()`은 범위 검사를 수행하며, 잘못된 인덱스에 대해 `std::out_of_range` 예외 발생
- `operator[]`는 범위 검사를 하지 않음 → 런타임 오류 발생 가능


### 📌 반복자 주의사항

- `std::vector`는 메모리 재할당이 발생할 수 있으므로, **삽입/삭제 후 기존 반복자는 무효화** 될 수 있음
- 반복자 사용 시에는 재할당 여부를 고려해야 안전


### 📌 주요 메서드 요약

| 메서드        | 설명 |
|--------------|------|
| `push_back()` | 끝에 요소 추가 |
| `pop_back()`  | 끝 요소 제거 |
| `insert()`    | 특정 위치에 요소 삽입 |
| `erase()`     | 특정 위치 또는 범위의 요소 제거 |
| `at()`        | 범위 검사 포함한 요소 접근 |
| `[]`          | 범위 검사 없는 요소 접근 |
| `size()`      | 현재 요소 개수 반환 |
| `clear()`     | 모든 요소 제거 |
| `data()`      | 내부 배열의 포인터 반환 |


### ✅ 요약

- `std::vector`는 **동적 배열** 로 유연한 크기 조절이 가능
- 배열처럼 사용하면서 STL의 기능도 활용 가능
- 반복자 무효화, 예외 처리 등 주의할 점이 존재
- 대부분의 경우, raw pointer 배열보다 `std::vector` 사용을 권장

---

