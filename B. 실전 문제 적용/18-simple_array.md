## 📘 SimpleArray<T> 사용 설명서 (C++17)

- **외부 API에 노출하기 위한 연속 메모리 기반 동적 배열** 을 설명한다.
- `std::vector`와 비슷한 용도이지만 인터페이스와 일부 동작이 다르다.

### 📌 1. 목적과 특징

- `SimpleArray<T>`는 `T* m_a`, `int m_count`, `int m_capacity`로 연속 저장 공간을 관리하는 템플릿 컨테이너다.
- 외부 인터페이스에서 `std::vector<T>` 대신 사용할 수 있도록 독립적인 배열 타입을 제공한다.
- 요소 저장 공간은 연속적이며 `Array()`로 `T*`를 얻는다.
- 기본 할당자는 `MallocAllocator`이고, `SimpleAllocator` 인터페이스를 통해 다른 할당자를 전달할 수 있다.
- 복사 생성/대입, 이동 생성/대입, 초기화 목록, 반복자(`begin/end`)를 지원한다.
- 원소 수(`Count`)와 확보된 공간(`Capacity`)을 분리해 관리한다.
- 원소 특성에 따라 바이트 복사/재배치 경로와 생성자·소멸자를 사용하는 경로를 나눈다.

```cpp
SimpleArray<int> values = {10, 20, 30};
values.Append(40);

for (int v : values)
    std::cout << v << ' ';

int* data = values.Array();
int count = values.Count();
```

- **의미:** `SimpleArray` 자체가 저장 공간을 소유한다.
- 다만 `T`가 포인터라면 포인터가 가리키는 객체의 소유권까지 자동으로 관리하지는 않는다.

### 📌 2. 생성과 복사

| 구문 | 의미 |
|---|---|
| `SimpleArray<T> a;` | 빈 배열 생성 |
| `SimpleArray<T> a(capacity);` | 초기 **용량** 확보; 원소 수는 0 |
| `SimpleArray<T> a(count, buffer);` | 주어진 버퍼의 원소를 복사해 추가 |
| `SimpleArray<T> a{v1, v2};` | 초기화 목록으로 생성 |
| `SimpleArray<T> b(a);` | 복사 생성 |
| `SimpleArray<T> b(std::move(a));` | 이동 생성 |
| `b = a;`, `b = std::move(a);` | 복사/이동 대입 |

```cpp
SimpleArray<double> a(100);  // Count() == 0, Capacity() >= 100

a.Append(1.5);
a.Append(2.5);

SimpleArray<double> b = a;  // 값 복사
SimpleArray<double> c = std::move(b); // 저장 공간 이동
```

### 📌 3. 주요 API

#### 🔹 조회 및 원소 접근

| 함수 | 설명 |
|---|---|
| `Count()` | 현재 원소 수 (`int`) |
| `Capacity()` | 확보된 원소 공간 (`int`) |
| `IsEmpty()` | 빈 배열 여부 |
| `operator[](int)` / `operator[](Index)` | 인덱스 접근; 범위 검사는 `assert` |
| `Array()` | 연속 버퍼의 시작 주소 (`T*` / `const T*`) |
| `First()` / `Last()` | 첫/마지막 원소의 포인터; 빈 배열이면 `nullptr` |
| `FirstValue()` / `LastValue()` | 첫/마지막 원소의 **값 복사**; 빈 배열이면 `T()` |
| `begin/end`, `cbegin/cend` | 범위 기반 `for` 및 반복자 사용 |
| `Search(key)` | 처음 일치하는 원소 인덱스; 실패하면 `-1` |

#### 🔹 추가·삽입·삭제

| 함수 | 설명 |
|---|---|
| `Append(const T&)`, `Append(T&&)` | 끝에 원소 추가 |
| `Append(count, buffer)` | 버퍼의 원소 여러 개 추가 |
| `Append({ ... })` | 초기화 목록 추가 |
| `AppendNew()` | 끝에 새 원소를 만들고 참조 반환 |
| `Insert(index, value)` | 지정 위치에 삽입 |
| `Remove()` | 마지막 원소 삭제 |
| `Remove(index)` | 지정 위치의 원소 삭제 |
| `RemoveAt(index, n)` | 지정 위치부터 최대 `n`개 삭제 |
| `RemoveValue(key)` | `key`와 같은 **모든 원소** 삭제 |
| `RemoveIf(pred)` | 조건을 만족하는 **모든 원소** 삭제 |
| `Empty()` | 원소를 비우되 할당 공간은 유지 |
| `Destroy()` | 원소 및 저장 공간 해제; Count/Capacity 0 |

```cpp
SimpleArray<int> a{1, 2, 3, 2, 4};
a.Insert(1, 10);   // [1, 10, 2, 3, 2, 4]
a.Remove(1);       // [1, 2, 3, 2, 4]
a.RemoveValue(2);  // [1, 3, 4] : 일치하는 값 모두 제거
```

- `RemoveIf`는 현재 **함수 포인터** 를 받는다.
- 캡처 없는 람다는 사용할 수 있지만 캡처가 있는 람다는 직접 전달할 수 없다.

```cpp
SimpleArray<int> a{1, 2, 3, 4, 5, 6};
a.RemoveIf([](const int& v) { return v % 2 == 0; });
// [1, 3, 5]

// int threshold = 3;
// a.RemoveIf([threshold](const int& v) { return v > threshold; });
// 현재 시그니처에서는 컴파일 불가: 캡처 람다는 함수 포인터로 변환되지 않음
```

#### 🔹 크기·용량 및 값 변경

| 함수 | 설명 |
|---|---|
| `Reserve(newCap)` | 필요하면 용량 증가; 반환값은 버퍼 포인터 |
| `SetCapacity(cap)` | 용량을 지정 값으로 조정; 축소 시 원소 수가 줄 수 있음 |
| `SetCount(n)` | 논리적인 원소 수 조정 |
| `Resize(n, value)` | 원소 수를 `n`으로 만들고 **0부터 n-1까지 전부** `value`로 설정 |
| `Shrink()` | 용량을 현재 원소 수로 축소 시도 |
| `Fill(value)` | 현재 원소 전체를 지정 값으로 변경 |
| `SetRange(from, count, value)` | 일부 구간을 지정 값으로 변경 |
| `Zero()` | 바이트 초기화 경로 또는 `T()` 대입 경로 사용 |
| `MemSet(byte)` | 바이트 초기화 경로에만 사용 가능 |
| `SetData(count, buffer)` | 기존 저장 공간 해제 후 버퍼 데이터 복사 |

```cpp
SimpleArray<int> a;
a.Reserve(100);     // Capacity >= 100, Count == 0
a.SetCount(3);     // Count == 3; 단순 타입의 새 값은 초기화 보장 안 됨
a.Fill(7);        // [7, 7, 7]
a.Resize(5, 2);   // [2, 2, 2, 2, 2] (기존 값도 덮어씀)
```

> `std::vector::resize(n, value)`와 달리 이 구현의 `Resize(n, value)`는 **기존 원소까지 모두 덮어쓴다**.
> `SetCount`도 단순 타입 경로에서 새 요소를 값 초기화하지 않는다.

#### 🔹 정렬·검색·순서 변경

| 함수 | 설명 |
|---|---|
| `QuickSort(cmp)` | 비교 함수 포인터를 받아 `std::sort`로 정렬 |
| `HeapSort(cmp)` | `make_heap` + `sort_heap`로 정렬 |
| `QuickSortAndRemoveDuplicates(cmp)` | 정렬 후 비교 결과가 0인 중복값 제거 |
| `BinarySearch(&key, cmp)` | 정렬된 배열에서 이진 검색; 실패 시 `-1` |
| `SortIndex(index, cmp)` | 원소 자체를 바꾸지 않고 정렬 순서의 인덱스 작성 |
| `Permute(index)` | `index[i]`에 해당하는 기존 원소를 새 위치 `i`로 배치 |
| `Reverse()` | 원소 순서를 뒤집음 |
| `Swap(i, j)` | 두 원소 교환 |

- 비교 함수의 형태는 `int (*)(const T*, const T*)`이다. 음수/0/양수로 순서를 나타내도록 작성한다.

```cpp
int CompareInt(const int* a, const int* b)
{
    return (*a < *b) ? -1 : ((*a > *b) ? 1 : 0);
}

SimpleArray<int> a{5, 1, 3, 3, 2};
a.QuickSort(&CompareInt);  // [1, 2, 3, 3, 5]

int key = 3;
int index = a.BinarySearch(&key, &CompareInt); // 일치하는 인덱스 (중복 시 위치 비보장)

// 주의: SimpleArray::BinarySearch 실패 반환값은 항상 -1.
// 앞서 만든 Arrays::BinarySearch의 Java식 -(삽입 위치)-1 규칙과 다름.
```

### 📌 4. 외부 데이터 연동

```cpp
void ConsumePoints(const double* values, int count);

SimpleArray<double> a{1.0, 2.0, 3.0};
ConsumePoints(a.Array(), a.Count());
```

- `Array()`가 반환하는 포인터는 `SimpleArray`가 소유한 메모리를 가리킨다.
- `Append`, `Insert`, `Reserve`, `SetCapacity`, `Shrink`, `Destroy` 등으로 저장 공간이 재할당되거나 해제되면 기존 포인터·참조·반복자가 무효화될 수 있다.
- 외부 코드가 이 주소를 장기간 보관하지 않도록 주의한다.

- `SetAllocator()`는 할당자 포인터를 바꾸지만 기존 버퍼를 옮기지 않는다.
- **이미 메모리가 할당된 상태에서 다른 할당자로 교체하면 할당/해제 주체가 달라질 수 있다.**
- 따라서 초기 생성 시 할당자를 정하고 수명 동안 유지하는 것이 안전하다.
- 사용자 정의 할당자 객체 역시 배열보다 오래 살아 있어야 한다.

### 📌 5. STL 및 CAD 작업과의 연결

```cpp
SimpleArray<int> a{3, 1, 2};
std::sort(a.begin(), a.end());

for (const auto& value : a)
    std::cout << value << '\n';
```

- 연속 저장과 반복자를 제공하므로 `std::sort`, `std::find` 등 일부 STL 알고리즘을 사용할 수 있다.
- `Windows(window_size)`는 별도,  `ArrayWindows<T>` 뷰를 반환한다.
- 파일 끝에는 `PrintArray`, 스트림 `operator<<`, 배열 뒤집기, 행/열 반전(행 우선·열 우선),  
  `CircularNextIndex`, `CircularPreviousIndex`, `CircularNext`, `CircularPrevious` 등 보조 함수도 정의되어 있다.

### 📌 6. std::vector와 차이

| 항목 | `SimpleArray<T>` | `std::vector<T>` |
|---|---|---|
| 크기 조회 | `Count()` | `size()` |
| 추가 | `Append()` | `push_back()` / `emplace_back()` |
| 제거 | `Remove()` / `Remove(index)` | `pop_back()` / `erase()` |
| 용량 예약 | `Reserve()` | `reserve()` |
| 원소 수 변경 | `SetCount()` / `Resize()` | `resize()` |
| 연속 버퍼 | `Array()` | `data()` |
| 사용자 정의 할당 | `SimpleAllocator*` | allocator 템플릿 매개변수 |
| 실패한 이진 검색 | `-1` | STL `lower_bound`는 반복자 반환 |
| 범위 검사 | `operator[]`의 `assert` | `at()`은 예외, `[]`는 범위 검사 없음 |

- `SimpleArray`는 외부 노출용 인터페이스를 단순화하려는 목적에 적합하지만 **`std::vector`와 완전히 호환되는 대체 구현은 아니다.**
- 특히 원소 생성/초기화, 예외 처리, 메모리 재할당 실패 처리는 별도로 확인해야 한다.


### 📌 7. 간단한 사용 예제

```cpp
#include "simple_array.hpp"
#include <iostream>

int CompareInt(const int* a, const int* b)
{
    return (*a < *b) ? -1 : ((*a > *b) ? 1 : 0);
}

int main()
{
    SimpleArray<int> values{5, 2, 4, 2, 1};

    values.Append(3);
    values.RemoveValue(2);
    values.QuickSort(&CompareInt);

    std::cout << values << '\n';  // [ 1, 3, 4, 5 ]
    std::cout << "count=" << values.Count()
              << ", capacity=" << values.Capacity() << '\n';

    values.Empty();   // 요소 수만 0으로
    values.Destroy(); // 저장 공간까지 해제
}
```

---

