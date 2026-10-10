## 📘 C++17 `std::array` 정리

### 📌 1. 개요
- `std::array<T, N>`은 **크기가 컴파일 시점에 고정되는 배열 컨테이너** 이다.
- `<array>` 헤더에 정의되어 있으며, C 스타일 배열처럼 원소가 연속된 메모리에 저장되지만 STL 컨테이너 인터페이스를 제공한다.

```cpp
#include <array>

std::array<int, 4> values{10, 20, 30, 40};
```

- `T`: 원소 타입
- `N`: 원소 개수. **타입의 일부** 이므로 `std::array<int, 3>`과 `std::array<int, 4>`는 서로 다른 타입이다.
- 크기를 실행 중에 바꿀 수 없다. 동적 크기가 필요하면 `std::vector`를 사용한다.
- 일반적인 `std::array<T, N>` 객체 자체에 별도의 동적 배열 할당이 필요하지 않다. 다만 `T` 내부에서 동적 할당할 수는 있다.

### 📌 2. 생성 및 초기화

```cpp
std::array<int, 3> a{1, 2, 3};  // 모든 원소 초기화
std::array<int, 3> b{};         // 모든 원소 0으로 초기화
std::array<int, 3> c{1};        // {1, 0, 0}
std::array<int, 3> d;           // int 원소는 초기화되지 않음

std::array<double, 3> xyz{1.0, 2.0, 3.0};
```

- C++17에서는 클래스 템플릿 인수 추론(CTAD)도 가능하다.

```cpp
std::array inferred{1, 2, 3}; // std::array<int, 3>
// std::array mixed{1, 2.0};   // 오류: 원소 타입이 서로 다름
```

### 📌 3. 원소 접근

```cpp
std::array<int, 3> a{10, 20, 30};

int x = a[1];       // 20, 범위 검사 없음
int y = a.at(1);    // 20, 범위 위반 시 std::out_of_range
int first = a.front();
int last  = a.back();
int* ptr  = a.data(); // 첫 원소를 가리키는 포인터
```

- `operator[]`의 범위를 벗어난 접근은 정의되지 않은 동작(UB)이다.
- `at()`은 예외를 던진다.
- 빈 `std::array<T, 0>`에는 `front()`와 `back()`을 호출하면 안 된다.

### 📌 4. 크기 및 반복

```cpp
std::array<int, 4> a{1, 2, 3, 4};

std::size_t n = a.size();  // 4
bool empty = a.empty();   // false

for (int v : a) {
    // v 사용
}

for (auto it = a.begin(); it != a.end(); ++it) {
    *it *= 2;
}
```

- 주요 API: `size()`, `empty()`, `max_size()`, `begin()`, `end()`, `cbegin()`, `cend()`, `rbegin()`, `rend()`, `data()`.

### 📌 5. 배열 전체 복사와 대입

- C 스타일 배열과 달리 `std::array`는 **배열 전체를 복사하거나 대입** 할 수 있다.

```cpp
std::array<int, 3> a{1, 2, 3};
std::array<int, 3> b = a; // 전체 원소 복사

b[0] = 99;                // a[0]은 그대로 1

a = b;                   // 전체 원소 대입
```

- 복사 비용은 일반적으로 원소 수 `N`에 비례한다.
- 큰 배열을 함수에 전달할 때 불필요한 복사를 피하려면 `const std::array<T, N>&`를 사용한다.

```cpp
template <typename T, std::size_t N>
void Process(const std::array<T, N>& values) {
    // 읽기 전용
}
```

### 📌 6. 자주 사용하는 변경 함수

```cpp
std::array<int, 4> a{1, 2, 3, 4};
a.fill(7);                  // {7, 7, 7, 7}

std::array<int, 4> b{4, 3, 2, 1};
a.swap(b);                  // 원소 교환
```

- `push_back()`, `resize()`, `reserve()`는 없다.
- `fill()`은 원소에 값을 대입하고, `swap()`은 같은 타입/크기의 배열끼리 원소를 교환한다.

### 📌 7. STL 알고리즘과 함께 사용

```cpp
#include <algorithm>
#include <array>

std::array<int, 5> a{5, 1, 4, 2, 3};
std::sort(a.begin(), a.end());       // {1, 2, 3, 4, 5}

auto it = std::find(a.begin(), a.end(), 3);
if (it != a.end()) {
    // 3을 찾음
}
```

### 📌 8. 구조적 바인딩과 `std::get`

```cpp
#include <array>
#include <tuple>

std::array<double, 3> point{1.0, 2.0, 3.0};

auto [x, y, z] = point; // 값 복사
auto& [rx, ry, rz] = point; // 원본 참조

std::get<0>(point) = 10.0; // 인덱스는 컴파일 타임 상수
```

### 📌 9. `std::array<T, 0>` 주의점

```cpp
std::array<int, 0> empty{};

static_assert(empty.size() == 0);
// empty.front(); // 사용 금지
// empty.back();  // 사용 금지
```

`begin() == end()`이며 `data()`가 널 포인터라고 보장되지는 않는다. 역참조하면 안 된다.

### 📌 10. C 배열 / `std::array` / `std::vector` 비교

| 항목 | C 배열 `T[N]` | `std::array<T,N>` | `std::vector<T>` |
|---|---|---|---|
| 원소 개수 | 고정 | 고정 | 실행 중 변경 가능 |
| 연속 저장 | 예 | 예 | 예 |
| 크기 조회 | 별도 처리 필요 | `size()` | `size()` |
| 전체 복사 대입 | 불가 | 가능 | 가능 |
| `at()` 범위 검사 | 없음 | 있음 | 있음 |
| 반복자/STL 사용 | 포인터 등 사용 | 직접 지원 | 직접 지원 |
| 동적 배열 할당 | 선언 방식에 따름 | 컨테이너 자체에는 불필요 | 일반적으로 사용 |

- `std::array` 객체가 항상 스택에 놓이는 것은 아니다.
- 멤버 변수, 전역 변수, 힙에 생성된 객체의 멤버 등 어디에든 놓일 수 있다.

### 📌 11. CAD/수치 계산 예제
- 고정 크기 벡터나 행렬의 저장에 적합하다.

```cpp
#include <array>

using Vector3 = std::array<double, 3>;
using Matrix3 = std::array<std::array<double, 3>, 3>;

Vector3 p{1.0, 2.0, 3.0};
Matrix3 identity{{
    {{1.0, 0.0, 0.0}},
    {{0.0, 1.0, 0.0}},
    {{0.0, 0.0, 1.0}}
}};

double z = p[2];
double m12 = identity[1][2];
```
- 행렬의 메모리 배치(행 우선/열 우선)는 **인덱싱 규약을 어떻게 정하느냐** 에 달려 있다. 위 예제는 `matrix[row][column]` 형식이다.

### 📌 12. 핵심 주의 사항

1. 크기 `N`은 컴파일 시점 상수여야 한다.
2. `std::array<T, N>`과 `std::array<T, M>`은 `N != M`이면 다른 타입이다.
3. `std::array<T, N>`은 값처럼 복사되므로 큰 배열 전달 시 복사 비용을 고려한다.
4. `operator[]`는 범위 검사를 하지 않는다. 검사가 필요하면 `at()`을 사용한다.
5. `data()`로 얻은 포인터는 배열 객체가 유효한 동안만 유효하다. 해당 배열을 복사하면 복사본의 `data()`는 별도의 저장소를 가리킨다.
6. 크기를 늘리거나 줄이는 API는 없다.

### 📌 13. 최소 실행 테스트 (C++17)

```cpp
#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
    std::array<int, 3> a{3, 1, 2};
    assert(a.size() == 3);
    assert(a.front() == 3);
    assert(a.back() == 2);

    std::sort(a.begin(), a.end());
    assert((a == std::array<int, 3>{1, 2, 3}));

    auto b = a;
    b[0] = 99;
    assert(a[0] == 1); // 독립적인 복사본

    a.fill(7);
    assert((a == std::array<int, 3>{7, 7, 7}));

    bool thrown = false;
    try {
        (void)a.at(3);
    } catch (const std::out_of_range&) {
        thrown = true;
    }
    assert(thrown);

    std::array<int, 0> empty{};
    assert(empty.empty());

    std::cout << "std::array tests PASS\n";
}
```

> `assert`는 `NDEBUG`가 정의된 Release 빌드에서 비활성화된다.
> 실제 회귀 테스트에서는 별도 검사 매크로를 사용하는 편이 안전하다.

---

