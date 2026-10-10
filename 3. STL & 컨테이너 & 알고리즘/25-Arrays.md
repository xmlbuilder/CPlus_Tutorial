## 📘 Arrays 유틸리티 사용 설명 (C++17)

- Java `java.util.Arrays`처럼 C 배열, `std::array`, `std::vector`를 동일한 호출 방식으로 처리하기 위한 편의 함수 모음이다.
- 기존 STL 알고리즘을 감싸서 출력·검색·정렬·비교·복사 등을 간단히 사용한다.

### 📌 1. 기본 사용

```cpp
#include "arrays.hpp"  // 프로젝트에서 사용하는 헤더 이름에 맞게 변경
#include <array>
#include <vector>
#include <iostream>

int main() {
    int raw[] = {3, 1, 2};
    std::array<int, 3> fixed{3, 1, 2};
    std::vector<int> dynamic{3, 1, 2};

    Arrays::Sort(raw);
    Arrays::Print(raw);                    // [1, 2, 3]
    std::cout << Arrays::ToString(fixed);  // [3, 1, 2]
    std::cout << Arrays::Contains(dynamic, 2); // 1 (true)
}
```

### 📌 2. 함수 요약

| 구분 | 함수 | 동작 및 반환 |
|---|---|---|
| 출력 | `ToString(a)` | `[1, 2, 3]` 형태의 `std::string` |
| 출력 | `ToString(a, n)` | 앞의 최대 `n`개만 출력, 나머지는 `...` |
| 출력 | `Print(a, os)` | 지정한 스트림에 문자열과 개행 출력 (`os` 생략 시 `std::cout`) |
| 출력 | `DeepToString(a)` | 중첩 배열을 재귀적으로 출력 |
| 수정 | `Sort(a)` | 오름차순 정렬; 비교 함수 인자 지정 가능 |
| 수정 | `SortDescending(a)` | 내림차순 정렬; 비교 함수 인자 지정 가능 |
| 수정 | `Fill(a, value)` | 모든 원소를 지정 값으로 설정 |
| 수정 | `Reverse(a)` | 원소 순서 뒤집기 |
| 검색 | `Contains(a, value)` | 존재하면 `true` |
| 검색 | `IndexOf(a, value)` | 첫 원소 인덱스, 없으면 `-1` |
| 검색 | `BinarySearch(a, value)` | 발견 인덱스 또는 `-(삽입 위치)-1` |
| 비교 | `Equals(a, b)` | 크기와 모든 원소가 같으면 `true` |
| 비교 | `EqualsEpsilon(a, b, eps)` | 절대 오차 기준 원소별 비교 |
| 복사 | `CopyOf(a)` | 전체 복사한 `std::vector` |
| 복사 | `CopyOf(a, n)` | 길이 `n`인 `std::vector`로 복사/잘라내기/확장 |
| 계산 | `Min(a)` / `Max(a)` | 최솟값 / 최댓값 반환 |
| 계산 | `Sum(a)` | 원소 타입으로 합산 |
| 계산 | `Sum(a, initial)` | `initial`의 타입으로 합산 |

### 📌 3. 출력

```cpp
int a[] = {10, 20, 30, 40};
Arrays::ToString(a);       // "[10, 20, 30, 40]"
Arrays::ToString(a, 2);    // "[10, 20, ...]"
Arrays::ToString(a, 0);    // "[...]"
Arrays::Print(a);          // 콘솔에 [10, 20, 30, 40] 출력

int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
Arrays::DeepToString(matrix); // "[[1, 2, 3], [4, 5, 6]]"
Arrays::ToString(matrix);     // "[<nested range>, <nested range>]"
```

- `ToString`은 원소의 `operator<<`를 사용한다.
- 사용자 정의 타입도 출력 연산자를 제공하면 된다.
- 포인터 원소는 자동으로 역참조하지 않는다.
- `std::string`은 중첩 범위로 처리하지 않고 문자열로 출력한다.

### 📌 4. 정렬·수정·검색

```cpp
std::vector<int> a{5, 2, 8, 1, 3};
Arrays::Sort(a);             // [1, 2, 3, 5, 8]
Arrays::Contains(a, 3);      // true
Arrays::IndexOf(a, 3);       // 2
Arrays::IndexOf(a, 10);      // -1
Arrays::SortDescending(a);   // [8, 5, 3, 2, 1]
Arrays::Reverse(a);          // [1, 2, 3, 5, 8]
Arrays::Fill(a, 7);          // [7, 7, 7, 7, 7]
```

- `Sort`와 `BinarySearch`는 **임의 접근 반복자** 가 있는 정렬 가능한 범위가 필요하다.
- `std::list`에는 그대로 적용할 수 없다.
- `BinarySearch`를 호출하기 전에 **같은 비교 기준으로 정렬** 해야 한다.

### 📌 5. BinarySearch 반환값 이해

- 구현은 `std::lower_bound`를 사용하고, Java `Arrays.binarySearch()` 방식으로 결과를 정수화한다.
- 발견: `index`
- 미발견: `-(insertionPoint) - 1`

```cpp
int a[] = {1, 2, 3};

Arrays::BinarySearch(a, 2);   //  1 : 인덱스 1에 존재
Arrays::BinarySearch(a, 0);   // -1 : 삽입 위치 0
Arrays::BinarySearch(a, 4);   // -4 : 삽입 위치 3
Arrays::BinarySearch(a, 2.5); // -3 : 삽입 위치 2
```

| 검색값 | 발견 여부 | 삽입 위치 | 반환값 |
|---|---|---:|---:|
| `2` | 발견 (인덱스 1) | — | `1` |
| `0` | 미발견 | 0 | `-1` |
| `4` | 미발견 | 3 | `-4` |
| `2.5` | 미발견 | 2 | `-3` |

```cpp
auto result = Arrays::BinarySearch(a, 2.5);
if (result < 0) {
    auto insertionPoint = -result - 1; // 2
}
```

- `2.5`를 `int`로 변환하지 않고 비교하므로 삽입 위치가 2로 계산된다.
- 정렬되지 않은 배열에서의 검색 결과는 의미가 없다.

### 📌 6. 비교와 부동소수점 오차

```cpp
int a[] = {1, 2, 3};
std::array<int, 3> b{1, 2, 3};
Arrays::Equals(a, b);  // true

double x[] = {1.0, 2.0};
std::array<double, 2> y{1.0 + 1e-13, 2.0};
Arrays::EqualsEpsilon(x, y, 1e-12); // true
Arrays::EqualsEpsilon(x, y, 1e-14); // false
```

- `EqualsEpsilon`은 **절대 오차** 비교이며 상대 오차 비교가 아니다.
- 길이가 다르면 `false`, NaN 원소가 있으면 `false`를 반환한다.
- 같은 부호의 무한대끼리는 같게 처리한다.
- `epsilon`이 음수·NaN·무한대이면 `std::invalid_argument` 예외를 던진다.
- 원소는 `double`로 변환 가능해야 한다.

### 📌 7. 복사와 계산

```cpp
int a[] = {1, 2, 3};

auto full = Arrays::CopyOf(a);      // vector<int>{1, 2, 3}
auto shortCopy = Arrays::CopyOf(a, 2); // vector<int>{1, 2}
auto longCopy = Arrays::CopyOf(a, 5);  // vector<int>{1, 2, 3, 0, 0}

Arrays::Min(a);      // 1
Arrays::Max(a);      // 3
Arrays::Sum(a);      // 6 (int)
Arrays::Sum(a, 0LL); // 6 (long long)
```

- `CopyOf`는 원본 타입과 관계없이 `std::vector`를 반환하며, 늘어난 공간은 값 초기화한다.
- 원소 복사가 가능해야 하고, 확장 시 기본 생성(값 초기화)이 가능해야 한다.
- `Min`과 `Max`는 빈 배열에서 `std::invalid_argument`를 던진다.
- `Sum`은 기본적으로 원소 타입으로 누적하므로 큰 정수 합계는 `0LL` 같은 넓은 초기값을 전달한다.

### 📌 8. STL과의 관계 및 제약

| Arrays 함수 | 기반 STL |
|---|---|
| `Sort` | `std::sort` |
| `Fill` | `std::fill` |
| `Reverse` | `std::reverse` |
| `Contains` / `IndexOf` | `std::find` |
| `BinarySearch` | `std::lower_bound` |
| `Equals` | `std::equal` |
| `CopyOf` | `std::vector` + 원소 복사 |
| `Min` / `Max` | `std::min_element` / `std::max_element` |
| `Sum` | `std::accumulate` |

- C 배열, `std::array`, `std::vector` 등 `std::begin`/`std::end`를 제공하는 범위에서 사용한다.
- 정렬·채우기·뒤집기는 원본을 변경한다. `CopyOf`는 새 벡터를 만든다.
- `ToString`과 `DeepToString`은 출력 문자열을 생성하므로 대용량 배열에서는 출력 개수 제한을 활용한다.
- 포인터 요소의 소유권을 관리하거나 유효성을 확인하지 않는다.
- `EqualsEpsilon`은 현재 수치형 원소를 위한 함수이며 CAD 좌표의 상대 오차 정책까지 포함하지 않는다.

### 📌 9. 소스 코드
```cpp
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// C++17. Supports C arrays, std::array, std::vector and compatible ranges.
// All functions operate on elements; no ownership of pointer elements is assumed.
namespace Arrays {

    template <typename T> struct IsRange {
    private:
        template <typename U>
        static auto Check(int) -> decltype(std::begin(std::declval<const U&>()),
                                           std::end(std::declval<const U&>()), std::true_type{});
        template <typename> static std::false_type Check(...);
    public:
        static constexpr bool value = decltype(Check<T>(0))::value;
    };

    template <typename T>
    void Append(std::ostringstream& os, const T& value, const bool deep) {
        if constexpr (IsRange<T>::value && !std::is_convertible_v<T, std::string>) {
            if (deep) {
                os << '[';
                bool first = true;
                for (const auto& element : value) {
                    if (!first) os << ", ";
                    Append(os, element, true);
                    first = false;
                }
                os << ']';
            } else {
                os << "<nested range>";
            }
        } else {
            os << value;
        }
    }

    template <typename Range>
    std::string ToString(const Range& range, std::size_t maxElements = std::numeric_limits<std::size_t>::max()) {
        std::ostringstream os;
        os << '[';
        std::size_t count = 0;
        auto it = std::begin(range);
        const auto last = std::end(range);
        for (; it != last && count < maxElements; ++it, ++count) {
            if (count) os << ", ";
            Append(os, *it, false);
        }
        if (it != last) os << (count ? ", ..." : "...");
        os << ']';
        return os.str();
    }

    template <typename Range>
    std::string DeepToString(const Range& range) {
        std::ostringstream os;
        Append(os, range, true);
        return os.str();
    }

    template <typename Range>
    void Print(const Range& range, std::ostream& os = std::cout,
                      std::size_t maxElements = std::numeric_limits<std::size_t>::max()) {
        os << ToString(range, maxElements) << '\n';
    }

    template <typename Range, typename Compare = std::less<>>
    void Sort(Range& range, Compare compare = {}) {
        std::sort(std::begin(range), std::end(range), compare);
    }

    template <typename Range, typename Compare = std::greater<>>
    void SortDescending(Range& range, Compare compare = {}) {
        std::sort(std::begin(range), std::end(range), compare);
    }

    template <typename Range, typename Value>
    void Fill(Range& range, const Value& value) {
        std::fill(std::begin(range), std::end(range), value);
    }

    template <typename Range>
    void Reverse(Range& range) {
        std::reverse(std::begin(range), std::end(range));
    }

    template <typename Range, typename Value>
    bool Contains(const Range& range, const Value& value) {
        return std::find(std::begin(range), std::end(range), value) != std::end(range);
    }

    // Returns -1 when absent. Index is measured from the start of the range.
    template <typename Range, typename Value>
    std::ptrdiff_t IndexOf(const Range& range, const Value& value) {
        const auto it = std::find(std::begin(range), std::end(range), value);
        return it == std::end(range) ? -1 : std::distance(std::begin(range), it);
    }

    template <typename A, typename B>
    bool Equals(const A& a, const B& b) {
        return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b));
    }

    // For real floating-point elements; rejects NaN and negative/NaN epsilon.
    template <typename A, typename B>
    bool EqualsEpsilon(const A& a, const B& b, double epsilon) {
        if (!(epsilon >= 0.0) || !std::isfinite(epsilon))
            throw std::invalid_argument("epsilon must be finite and nonnegative");
        auto ia = std::begin(a);
        auto ib = std::begin(b);
        const auto ea = std::end(a);
        const auto eb = std::end(b);
        for (; ia != ea && ib != eb; ++ia, ++ib) {
            const double x = static_cast<double>(*ia);
            const double y = static_cast<double>(*ib);
            if (std::isnan(x) || std::isnan(y)) return false;
            if (x == y) continue; // equal infinities also compare equal
            if (!std::isfinite(x) || !std::isfinite(y) || std::abs(x - y) > epsilon) return false;
        }
        return ia == ea && ib == eb;
    }

    // Java-like insertion index encoding: found index, otherwise -(insertion index)-1.
    // The range must already be sorted with the same comparator.
    template <typename Range, typename Value, typename Compare = std::less<>>
    std::ptrdiff_t BinarySearch(const Range& range, const Value& value, Compare compare = {}) {
        const auto first = std::begin(range), last = std::end(range);
        const auto it = std::lower_bound(first, last, value, compare);
        const auto index = std::distance(first, it);
        if (it != last && !compare(*it, value) && !compare(value, *it)) return index;
        return -index - 1;
    }

    // Copy to a vector of requested length. Extra positions are value-initialized.
    template <typename Range>
    auto CopyOf(const Range& range, std::size_t newSize) {
        using T = std::decay_t<decltype(*std::begin(range))>;
        std::vector<T> result(newSize);
        auto src = std::begin(range);
        auto dst = result.begin();
        while (src != std::end(range) && dst != result.end()) {
            *dst++ = *src++;
        }
        return result;
    }

    template <typename Range>
    auto CopyOf(const Range& range) {
        return CopyOf(range, static_cast<std::size_t>(std::distance(std::begin(range), std::end(range))));
    }

    template <typename Range>
    auto Min(const Range& range) -> std::decay_t<decltype(*std::begin(range))> {
        const auto it = std::min_element(std::begin(range), std::end(range));
        if (it == std::end(range)) throw std::invalid_argument("Min: empty range");
        return *it;
    }

    template <typename Range>
    auto Max(const Range& range) -> std::decay_t<decltype(*std::begin(range))> {
        const auto it = std::max_element(std::begin(range), std::end(range));
        if (it == std::end(range)) throw std::invalid_argument("Max: empty range");
        return *it;
    }

    // Accumulator type defaults to the element type. Supply a wider initial value to avoid overflow.
    template <typename Range, typename T>
    T Sum(const Range& range, T initial) {
        return std::accumulate(std::begin(range), std::end(range), initial);
    }

    template <typename Range>
    auto Sum(const Range& range) {
        using T = std::decay_t<decltype(*std::begin(range))>;
        return Sum(range, T{});
    }
};
````
### 📌 10. 빠른 테스트

```cpp
#include "arrays.hpp"
#include <array>
#include <cassert>
#include <vector>

int main() {
    int raw[] = {3, 1, 2};
    Arrays::Sort(raw);
    assert(Arrays::ToString(raw) == "[1, 2, 3]");
    assert(Arrays::BinarySearch(raw, 2) == 1);
    assert(Arrays::BinarySearch(raw, 2.5) == -3);

    std::array<double, 2> x{1.0, 2.0};
    std::vector<double> y{1.0 + 1e-13, 2.0};
    assert(Arrays::EqualsEpsilon(x, y, 1e-12));

    assert(Arrays::ToString(Arrays::CopyOf(raw, 5)) == "[1, 2, 3, 0, 0]");
}
```
---
