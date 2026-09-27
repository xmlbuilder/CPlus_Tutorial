## 📦 C++ STL `std::array` 정리

### 📌 개요

`std::array`는 C++11에 도입된 **고정 크기 배열 컨테이너** 입니다.  
기존 C 스타일 배열(`T[N]`)과 유사하지만, STL 컨테이너의 장점을 함께 제공합니다.

```cpp
#include <array>
std::array<int, 3> a = {1, 2, 3};
```

- 크기는 **컴파일 타임** 에 고정됨
- 반복자, 대입 연산자, 메서드 등 STL 기능 지원
- `[]` 연산자를 이용한 인덱스 접근 가능
- STL 컨테이너이므로 다른 컨테이너와 동일한 인터페이스 제공
- 특별한 이유가 없다면 raw array 대신 `std::array` 사용 권장



### 📌 기본 사용법

```cpp
#include <iostream>
#include <array>
#include <algorithm>

int main() {
    std::array<int, 5> my_arr = {0, 1, 2, 3, 4};

    std::cout << my_arr[0] << std::endl;       // 인덱스 접근
    std::cout << my_arr.at(1) << std::endl;    // bounds-checked 접근
    std::cout << my_arr.size() << std::endl;   // 배열 크기

    std::sort(my_arr.begin(), my_arr.end());   // 정렬
}
```


### 📌 반복자 사용

```cpp
std::array<int, 5> arr = {0, 1, 2, 3, 4};

for (auto itr = arr.begin(); itr != arr.end(); ++itr)
    std::cout << *itr << std::endl;

for (int& x : arr)
    std::cout << x << " ";
```

- `begin()`, `end()`, `cbegin()`, `cend()`, `rbegin()`, `rend()` 등 다양한 반복자 제공
- `front()`는 `*begin()`과 동일
- `back()`은 마지막 요소 참조



### 📌 기타 메서드

```cpp
arr.fill(5);  // 모든 요소를 5로 채움
int* data_ptr = arr.data();  // 내부 배열 주소 반환
```

- `fill(value)`: 모든 원소를 동일한 값으로 설정
- `data()`: 내부 배열의 포인터 반환



### 📌 예외 처리

```cpp
try {
    arr.at(10) = 7;  // 범위를 벗어나면 std::out_of_range 예외 발생
} catch (const std::out_of_range& e) {
    std::cout << "예외 발생: " << e.what() << std::endl;
}
```

- `at(index)`는 범위 검사를 수행하며, 범위를 벗어나면 예외 발생
- `operator[]`는 범위 검사를 하지 않음


### 📌 다차원 배열

```cpp
constexpr size_t rows = 4, cols = 5;
std::array<std::array<int, cols>, rows> arr_2d;

for (size_t i = 0; i < rows; ++i)
    for (size_t j = 0; j < cols; ++j)
        arr_2d[i][j] = i * j;
```

- `std::array`를 중첩해 다차원 배열 생성 가능
- 내부 반복문으로 요소 접근


### 📌 형 변환 제한

```cpp
#include <array>
#include <iostream>

class Base {
    virtual void print() { std::cout << "Base" << std::endl; }
};

class Derived : public Base {
    virtual void print() { std::cout << "Derived" << std::endl; }
};

void foo(std::array<Base, 5> arr) { /* ... */ }

int main() {
    std::array<Derived, 5> data;
    // foo(data); // ❌ 컴파일 에러
    return 0;
}
```

- `std::array<Derived, N>` → `std::array<Base, N>` 암시적 변환 불가
- 이는 타입 안전성을 강화하기 위함
- 일반적인 C 스타일 배열과 달리 컨테이너 템플릿 인스턴스 간의 변환을 허용하지 않음

---

### 📌 요약

- `std::array`는 **고정 크기 배열** 을 안전하고 효율적으로 다룰 수 있는 STL 컨테이너
- 반복자, 정렬, 대입 등 다양한 STL 기능 지원
- `at()`를 통한 안전한 접근 가능
- C++11 이상 환경에서 raw array보다 `std::array` 사용 권장

---

