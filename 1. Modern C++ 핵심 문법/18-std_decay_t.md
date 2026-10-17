## 📘 std::decay_t
- std::decay_t<T>는 자료형 T에서 `참조(&, &&)` 와 `최상위 const/volatile`을 제거하고, 배열과 함수 타입도 일반적인 값 전달 형태로 변환하는 타입 유틸리티입니다.
- <type_traits> 헤더에 정의되어 있습니다.

### 📌 1. 기본 동작
```cpp
#include <type_traits>

std::decay_t<const int&>   // int
std::decay_t<int&&>        // int
std::decay_t<const double> // double
std::decay_t<int*>         // int*
```

- 주요 변환은 다음과 같습니다.

| 원래 자료형	| std::decay_t  결과 |
|-------------|-------------------|
| int	| int |
| const int	| int |
| int&	| int |
| const int&	| int |
| int&&	| int |
| const double&	| double |
| int[5]	| int* |
| const char[6]	| const char* |

- 참고로 포인터 자체는 제거하지 않습니다.

### 📌  const가 적용되는 위치가 다를 때

| 원래 자료형	| std::decay_t 결과	| 이유 |
|-------------|-------------------|------|
| const int	| int	| 최상위 const 제거 |
| int* const	| int*	| 포인터 자체의 const 제거 |
| const int*	| const int*	| 가리키는 대상의 const 유지 |
| const int* | const	| const int*	포인터 자체의 const만 제거 |
| const char[6]	| const char*	| 배열 원소의 const 유지 |

- 예를 들어:
```cpp
int a = 10;
int b = 20;

int* const p = &a;
```
- 여기서 p는 다른 주소를 가리키도록 변경할 수 없습니다.

- 반면:
```cpp
const int* p = &a;
```
- p가 가리키는 주소는 변경할 수 있지만, p를 통해 정수 값을 변경할 수는 없습니다.
- 즉, 최상위 const(top-level const)와 하위 const(low-level const)를 구분해야 합니다.

---
