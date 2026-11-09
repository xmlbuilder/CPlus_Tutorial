## 📘 std::is_integral,  std::is_unsigned, std::numeric_limits

- `<type_traits>`: 자료형의 특성을 검사
- `<limits>`: 자료형의 수치적 한계를 조회

### 📌 std::is_integral 
- 정수형인지 검사
```cpp
#include <type_traits>

std::is_integral<int>::value;       // true
std::is_integral<unsigned>::value;  // true
std::is_integral<double>::value;    // false
std::is_integral<bool>::value;      // true
std::is_integral<char>::value;      // true
```
- C++17부터는 _v를 사용하여 간단하게 작성할 수 있습니다.
```cpp
std::is_integral_v<int>       // true
std::is_integral_v<double>    // false
```

### 📌 std::is_unsigned 
- 부호 없는 타입인지 검사
```cpp
std::is_unsigned_v<int>           // false
std::is_unsigned_v<unsigned int>  // true
std::is_unsigned_v<uint64_t>      // true
std::is_unsigned_v<double>        // false
std::is_unsigned_v<bool>          // true
```
- 따라서 일반적인 unsigned 정수만 허용하려면 다음과 같이 작성할 수 있습니다.
```cpp
template<typename T>
constexpr bool IsUnsignedInteger =
    std::is_integral_v<T> &&
    std::is_unsigned_v<T> &&
    !std::is_same_v<T, bool>;
```
### 📌 std::numeric_limits 
- 자료형의 수치 특성
```cpp
#include <limits>

std::numeric_limits<int>::max();
std::numeric_limits<int>::min();

std::numeric_limits<double>::max();
std::numeric_limits<double>::min();
std::numeric_limits<double>::epsilon();

using Limits = std::numeric_limits<double>;

double a = Limits::min();     // 약 +2.225e-308
double b = Limits::lowest();  // 약 -1.798e+308
double c = Limits::max();     // 약 +1.798e+308

```

### 📌 digits, digits10, max_digits10 차이
```cpp
using L = std::numeric_limits<double>;

std::cout << L::digits << '\n';        // 53
std::cout << L::digits10 << '\n';      // 15
std::cout << L::max_digits10 << '\n';  // 17

#include <iomanip>
#include <limits>
#include <iostream>

double x = 1.2345678901234567;

std::cout
    << std::setprecision(
        std::numeric_limits<double>::max_digits10)
    << x;
```

| 항목	| double	| 의미 |
|-------|-----------|-----|
| `digits`	| 53	| 이진 유효 정밀도 |
| `digits10`	| 15	| 보존 가능한 십진 유효 자릿수 |
| `max_digits10`	| 17	| 출력 후 다시 읽어 동일한 double 값을 복원하는 데 필요한 자릿수 |


### 📌 실무에서 자주 쓰는 Type Traits
| 기능	| 설명 |
|------|------|
| `std::is_integral_v<T>`	| 정수형 여부 |
| `std::is_floating_point_v<T>`	| float, double, long double 여부 |
| `std::is_signed_v<T>`	| 부호 있는 타입 여부 |
| `std::is_unsigned_v<T>`	| 부호 없는 타입 여부 |
| `std::is_arithmetic_v<T>`	| 정수 또는 부동소수점 타입 |
| `std::is_same_v<T,U>`	| 두 타입이 같은지 검사 |
| `std::is_enum_v<T>`	| 열거형 여부 |
| `std::is_pointer_v<T>`	| 포인터 타입 여부 |
| `std::is_trivially_copyable_v<T>`	| 단순 바이트 복사가 허용되는 타입 여부 |
| `std::is_constructible_v<T,Args...>`	| 주어진 인자로 생성 가능한지 |
| `std::is_base_of_v<Base,T>`	| 상속 관계 여부 |

### 📌 템플릿에서 실제 활용

```cpp
#include <type_traits>
#include <limits>
#include <iostream>

template<typename T>
void PrintTypeInfo()
{
    if constexpr (std::is_integral_v<T>)
    {
        std::cout << "Integer\n";
        std::cout << "Unsigned: "
                  << std::is_unsigned_v<T> << '\n';
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        std::cout << "Floating point\n";
        std::cout << "Epsilon: "
                  << std::numeric_limits<T>::epsilon()
                  << '\n';
    }
}
```
---
