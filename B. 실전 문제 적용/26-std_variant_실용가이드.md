## 📘 `std::variant` 실용 가이드 — `get`, `get_if`, `visit`와 라이브러리 설계

> 목적: C++17에서 숫자·문자열 등 여러 형식의 값을 안전하게 저장하고, **라이브러리 사용자에게는 간단한 API**를 제공하는 방법을 정리한다.
> 예제는 Taguchi 직교배열의 물리적 수준값을 기준으로 한다.

### 📌 1. `std::variant`란?

- `std::variant<Ts...>`는 지정한 자료형 중 **정확히 하나의 값** 을 저장하는 타입 안전한 합집합(tagged union)이다.

```cpp
#include <variant>
#include <string>

using FactorValue = std::variant<double, std::string>;

FactorValue temperature = 125.0;
FactorValue material = std::string("Steel");

temperature = std::string("High"); // 현재 보관 타입이 string으로 변경
```

- `double`과 `std::string`을 동시에 저장하는 것이 아니라, 둘 중 하나를 저장한다.
- `variant`에서 `double`로 **암시적 변환** 하거나 `static_cast<double>(value)` 하는 것은 불가능하다.
- `std::variant`는 C++17부터 지원한다 (`#include <variant>`).
- `std::variant<double, std::string>`은 기본 생성 시 첫 번째 대안인 `double{}`을 보관한다.
- 값이 없음을 표현하려면 `std::monostate`를 첫 번째 대안으로 추가할 수 있다.

### 📌 2. 값 꺼내기: `std::get<T>()`

- 저장 타입을 **확실히 알고 있을 때** 사용한다.

```cpp
FactorValue value = 125.0;
double temperature = std::get<double>(value); // 125.0

value = std::string("Steel");
const std::string& name = std::get<std::string>(value);

// std::get<double>(value); // std::bad_variant_access 발생
```

- `std::get<0>(value)`처럼 타입 대신 대안의 인덱스를 지정할 수도 있다.
- 다만 타입이 중복되지 않는 설계에서는 `std::get<T>`가 의도를 더 잘 드러낸다.

### 📌 3. 안전한 타입 검사: `std::get_if<T>()`

- 값의 실제 타입이 불확실하다면 포인터를 받아 검사한다.
- 불일치 시 `nullptr`을 반환하며 **타입 불일치 자체로 예외를 던지지 않는다**.

```cpp
FactorValue value = std::string("Steel");

if (const auto* number = std::get_if<double>(&value))
{
    // number는 const double*
    std::cout << *number;
}
else if (const auto* text = std::get_if<std::string>(&value))
{
    std::cout << *text;
}
```

- `std::holds_alternative<T>(value)`는 해당 타입 보관 여부를 `bool`로 반환한다.

```cpp
if (std::holds_alternative<double>(value))
{
    const double number = std::get<double>(value);
}
```

- **선택 기준:** 타입이 보장되면 `get`, 불확실하고 한 타입만 확인하면 `get_if`, 모든 타입에 같은 작업을 적용하면 `visit`.

### 📌 4. 타입별 처리: `std::visit()`

- `std::visit(visitor, variant)`는 **현재 저장된 타입에 맞춰 visitor를 호출** 한다.

```cpp
#include <iostream>
#include <iomanip>
#include <variant>
#include <string>

using FactorValue = std::variant<double, std::string>;

void PrintValue(const FactorValue& value)
{
    std::visit([](const auto& actual)
    {
        std::cout << std::setw(12) << actual;
    }, value);
}
```

- `const auto&`는 실제 `double` 또는 `std::string`에 대응한다.
- 람다 본문은 **가능한 모든 타입에 대해 컴파일 가능** 해야 한다.
- 타입에 따라 다른 동작을 하려면 C++17 `if constexpr`을 이용한다.

```cpp
#include <type_traits>

std::string ToDisplayString(const FactorValue& value)
{
    return std::visit([](const auto& actual) -> std::string
    {
        using T = std::decay_t<decltype(actual)>;
        if constexpr (std::is_same_v<T, double>)
        {
            return std::to_string(actual);
        }
        else
        {
            return actual;
        }
    }, value);
}
```

- `std::to_string`은 표시 정밀도와 형식을 직접 제어하기 어렵다.
- 표 형태의 사용자 출력에는 `std::ostringstream`와 스트림 서식을 사용하는 편이 유연하다.

### 📌 5. 비교 요약

| API | 반환 | 타입이 다를 때 | 대표 용도 |
|---|---|---|---|
| `std::get<T>(v)` | `T&` / `const T&` 등 | `std::bad_variant_access` | 타입이 계약상 보장된 API |
| `std::get_if<T>(&v)` | `T*` / `const T*` | `nullptr` | 선택적 타입 추출 |
| `std::holds_alternative<T>(v)` | `bool` | `false` | 타입 분기 |
| `std::visit(fn, v)` | visitor의 반환값 | 타입별 visitor 호출 | 출력·직렬화·변환·처리 |

> `std::visit` 자체는 각 타입에 맞춰 호출하지만, visitor 내부에서 예외가 발생할 수 있다.
> 또한 드물게 `variant`가 `valueless_by_exception()` 상태라면 `visit`이 `std::bad_variant_access`를 던질 수 있다.

---

### 📌 6. 아키텍처 관점: 타입을 어디에 넣을 것인가?

- **핵심 원칙: 직교배열의 정수 수준 인덱스와 사용자에게 보이는 물리적 수준값을 분리한다.**

```text
                    [사용자 입력]
        Temperature: 100, 125, 150
        Material:    Steel, Al, Ti
                         |
                         v
            FactorDefinition / FactorValue
                         |
                         v
    OrthogonalArray (정수 수준 인덱스만 저장)
       row 0: [0, 0] / row 1: [0, 1] / ...
                         |
                         v
        BuildPhysicalDesign (표시용 매핑)
       row 0: [100.0, "Steel"]
       row 1: [100.0, "Al"]
                         |
             +-----------+-----------+
             |                       |
        GUI/CSV 출력           ANOVA 입력
      (variant 실제값)       (정수 수준 인덱스)
```

#### 🔹 기존 소스와의 관계

- 현재 `OrthogonalArray`는 `std::vector<int> data`를 사용하고, `FactorLevels`는 `std::vector<double> levels`를 사용한다. 
- **현재 파일 자체가 `variant` 기반인 것은 아니다.** 아래 코드는 그 구조를 보존하면서 추가할 수 있는 **확장 설계 예시** 다.

- 라이브러리 설계 권장 사항:

1. `OrthogonalArray::at(row, col)`은 계속 `int`를 반환한다. 직교성 검증과 통계 계산에 물리적 문자열이 필요하지 않다.
2. 기존 `FactorLevels`와 `build_design_from_orthogonal_array(...)`의 `double` API는 유지한다. 기존 사용자 코드를 깨뜨리지 않는다.
3. 혼합 타입이 필요한 사용자를 위해 **새 이름의 타입과 함수** 를 제공한다. `using FactorLevels = ...`로 기존 타입을 바꾸지 않는다.
4. `std::visit`을 모든 사용자에게 반복해서 요구하지 않고, 라이브러리의 `ToString`·`TryGet...` 등 경계 API에서 처리한다.

### 📌 7. 실용 예제: 기존 헤더를 변경하지 않는 확장 계층

```cpp
#include "orthogonal_array.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace doe
{
    using FactorValue = std::variant<double, std::string>;

    struct MixedFactorLevels
    {
        std::string name;
        std::vector<FactorValue> levels;
    };

    using MixedDesign = std::vector<std::vector<FactorValue>>;

    // UI, CSV, 로그 등 출력 경계에서 variant를 문자열로 변환한다.
    std::string ToString(const FactorValue& value)
    {
        return std::visit([](const auto& actual)
        {
            std::ostringstream out;
            out << actual;
            return out.str();
        }, value);
    }

    // 필요할 때 안전하게 숫자만 읽는다.
    const double* TryGetDouble(const FactorValue& value)
    {
        return std::get_if<double>(&value);
    }

    const std::string* TryGetString(const FactorValue& value)
    {
        return std::get_if<std::string>(&value);
    }

    // 기존 변환 알고리즘의 인덱스 매핑을 유지하는 혼합형 버전.
    MixedDesign BuildMixedDesign(
        const OrthogonalArray& oa,
        const std::vector<MixedFactorLevels>& factors,
        const std::vector<int>& factorIndices)
    {
        if (factorIndices.empty())
        {
            throw std::invalid_argument("factorIndices must not be empty");
        }

        if (oa.runs <= 0 || oa.factors <= 0 ||
            oa.data.size() != static_cast<std::size_t>(oa.runs) *
                              static_cast<std::size_t>(oa.factors))
        {
            throw std::invalid_argument("invalid orthogonal array dimensions");
        }

        for (const int factor : factorIndices)
        {
            if (factor < 0 || factor >= oa.factors ||
                static_cast<std::size_t>(factor) >= factors.size())
            {
                throw std::out_of_range("invalid factor index");
            }
        }

        MixedDesign design(
            static_cast<std::size_t>(oa.runs),
            std::vector<FactorValue>(factorIndices.size()));

        for (int row = 0; row < oa.runs; ++row)
        {
            for (std::size_t col = 0; col < factorIndices.size(); ++col)
            {
                const int factor = factorIndices[col];
                const int levelIndex = oa.at(row, factor);
                const auto& levels = factors[factor].levels;

                if (levelIndex < 0 ||
                    static_cast<std::size_t>(levelIndex) >= levels.size())
                {
                    throw std::out_of_range("invalid level index");
                }

                design[row][col] = levels[levelIndex];
            }
        }

        return design;
    }
}

int main()
{
    const OrthogonalArray& oa = OA_L9_3_4();

    const std::vector<doe::MixedFactorLevels> factors = {
        {"Temperature", {100.0, 125.0, 150.0}},
        {"Pressure",    {200.0, 300.0, 400.0}},
        {"Material",    {"Steel", "Aluminum", "Titanium"}},
        {"Washing",     {"None", "CM2", "CM3"}}
    };

    // F1, F3, F4만 표시. 인덱스는 원본 OA 열 번호(0-based).
    const std::vector<int> selectedFactors = {0, 2, 3};
    const auto design = doe::BuildMixedDesign(oa, factors, selectedFactors);

    std::cout << std::setw(6) << "Run";
    for (const int factor : selectedFactors)
    {
        std::cout << std::setw(14) << factors[factor].name;
    }
    std::cout << '\n';

    for (std::size_t row = 0; row < design.size(); ++row)
    {
        std::cout << std::setw(6) << row + 1;
        for (const auto& value : design[row])
        {
            std::cout << std::setw(14) << doe::ToString(value);
        }
        std::cout << '\n';
    }

    // 구체 타입이 필요한 사용자에게는 선택적 추출 API 제공.
    if (const double* temperature = doe::TryGetDouble(design[0][0]))
    {
        std::cout << "First temperature = " << *temperature << '\n';
    }

    if (const std::string* material = doe::TryGetString(design[0][1]))
    {
        std::cout << "First material = " << *material << '\n';
    }
}
```

#### 🔹 예상 출력

```text
   Run   Temperature      Material       Washing
     1           100         Steel          None
     2           100      Aluminum           CM2
     3           100      Titanium           CM3
     4           125      Aluminum           CM3
     5           125      Titanium          None
     6           125         Steel           CM2
     7           150      Titanium           CM2
     8           150         Steel           CM3
     9           150      Aluminum          None
First temperature = 100
First material = Steel
```

### 📌  8. 실제 라이브러리 공개 API 제안

- 혼합형 값을 자주 다룬다면, 사용자에게 `std::visit`를 강요하기보다 다음처럼 작은 API를 노출한다.

```cpp
namespace doe
{
    using FactorValue = std::variant<double, std::string>;

    std::string ToString(const FactorValue& value);
    const double* TryGetDouble(const FactorValue& value);
    const std::string* TryGetString(const FactorValue& value);
}
```

- 사용자 코드는 다음 정도로 끝난다.

```cpp
std::cout << doe::ToString(design[row][col]);

if (const auto* value = doe::TryGetDouble(design[row][col]))
{
    // 숫자인 경우에만 수치 계산
}
```

### 📌 무엇을 `variant`로 만들지 말아야 하나?

- `OrthogonalArray::data`: **정수 수준 인덱스 유지**. 직교성 검사·배열 선택의 핵심 데이터다.
- ANOVA에 전달하는 수준 식별자: `Steel`이라는 문자열 자체를 숫자 응답처럼 취급하지 않는다. 원본 OA 수준 인덱스 또는 기존 통계 API가 요구하는 별도 수준 식별자를 사용한다.
- 실험 결과(Response): 수치형 응답은 수치형으로 유지한다. 문자열 수준값과 혼합하지 않는다.

### 📌 9. 실무 주의 사항

1. **문자열은 범주형이다.** `"Steel"`과 `"Aluminum"` 사이에 수치적 크기나 거리를 가정하지 않는다.
2. **타입은 값마다 달라질 수 있다.** 위 구조는 한 인자의 각 수준이 모두 같은 타입인지 강제하지 않는다. 필요한 경우 인자 정의 시 동일 타입 검증을 추가한다.
3. **정수값이 필요하면 설계를 정한다.** 현재 예제는 `double`과 `std::string`만 지원한다. `int`를 별도 대안으로 추가하면 `get<int>`가 가능하지만, visitor가 처리해야 할 타입도 늘어난다.
4. **직교배열 검증과 표시값 검증은 다르다.** OA의 직교성은 수준 인덱스에 대해 검사한다. 실제 값 매핑에서는 인자별 수준 개수와 인덱스 범위를 검사한다.
5. **기존 ABI/API 보존:** 공개된 `FactorLevels`의 멤버 타입을 `vector<double>`에서 `vector<variant<...>>`로 직접 바꾸면 소스 호환성과 바이너리 호환성이 깨질 수 있다. 별도 타입·함수 추가가 안전하다.
6. **성능:** 대규모 수치 연산의 내부 루프에서는 `variant`보다 수치형 전용 배열이 유리할 수 있다. 물리적 수준값의 매핑·출력 경계에서 사용하는 것이 자연스럽다.

### 📌 10. 기억용 결론

- **`get<T>`**: 저장 타입을 알 때 꺼낸다. 다르면 예외.
- **`get_if<T>`**: 저장 타입을 모를 때 확인하며 꺼낸다. 다르면 `nullptr`.
- **`visit`**: 저장 타입에 따라 공통 작업이나 타입별 처리를 수행한다.
- **라이브러리 설계**: 내부의 직교배열/통계 계산은 정수·수치형으로 유지하고, 사용자-facing 물리 수준값에 `variant`를 적용한다.
- **API 호환성**: 기존 `double` 함수는 보존하고 혼합형 함수와 `ToString`·`TryGet...`를 추가한다.

---

