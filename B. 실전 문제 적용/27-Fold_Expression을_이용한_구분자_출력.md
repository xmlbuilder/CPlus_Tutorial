## 📘 Fold Expression을 이용한 구분자 출력

### 📌 1. 목적과 전체 코드

- `PrintSepTo(std::cout, ", ", 10, 20, 30)`처럼 호출하여 `10, 20, 30`을 출력한다.

``` cpp
#include <iostream>
#include <utility>

template<typename... Args>
void PrintSepTo(std::ostream& os, const char* separator, Args&&... args)
{
    bool first = true;
    ((os << (std::exchange(first, false) ? "" : separator)
         << std::forward<Args>(args)), ...);
}
```

- 핵심 표현식은 **가변 인자 템플릿**, **Fold Expression**, **`std::exchange`**, **`std::forward`** 를 조합한다.

### 📌 2. 가변 인자 템플릿

``` cpp
template<typename... Args>
void Print(Args&&... args);
```

-   `typename... Args`: 타입 매개변수 팩.
-   `Args&&... args`: 임의 개수의 인자를 받는 함수 매개변수 팩.
-   여기서 `Args&&`는 forwarding reference이며 lvalue/rvalue를 모두 받을 수 있다.

``` cpp
Print(10, 3.14, "Hello");
```

- 각 인자의 타입은 독립적으로 추론된다. 문자열 리터럴의 타입은 참조 추론시 배열 타입을 유지할 수 있다.

### 📌 3. Fold Expression `(expression, ...)`

``` cpp
template<typename... Args>
void Print(Args&&... args)
{
    ((std::cout << std::forward<Args>(args)), ...);
}
```

`Print(10, 20, 30)`은 개념적으로 다음처럼 확장된다.

``` cpp
(std::cout << 10),
((std::cout << 20), (std::cout << 30));
```

- 이는 **Unary Right Fold**이며, 쉼표 연산자로 묶인 각 표현식은 왼쪽부터 순서대로 평가된다. 따라서 출력 순서가 보장된다.

> 쉼표 연산자 `,`는 함수 인자를 나누는 구분용 쉼표와 다르다.

### 📌 4. `std::exchange(first, false)`

``` cpp
bool first = true;
bool old = std::exchange(first, false);
```

- `std::exchange`는 기존 값을 반환하면서 대상 변수에 새 값을 대입한다. 위 코드는 다음과 유사하다.

``` cpp
bool old = first;
first = false;
```

| 호출      | 이전 `first`   | 반환값   | 이후 `first` |
|-----------|----------------|---------|--------------|
| 첫 번째   | true           | true     | false |
| 두 번째   | false          | false    | false |
| 세 번째   | false          | false    | false |

### 📌 5. 삼항 연산자로 구분자 선택

``` cpp
std::exchange(first, false) ? "" : separator
```

- 첫 번째 인자 앞에는 빈 문자열을 출력하고, 두 번째 인자부터는 `separator`를 출력한다.

``` cpp
const char* sep;
if (std::exchange(first, false))
    sep = "";
else
    sep = separator;
```

| 인자   | 구분자   | 누적 출력
|--------|----------|----------|
| 10     | `""`    | `10` |
|  20    | `", "`  |  `10, 20` |
|  30    | `", "`  |  `10, 20, 30` |

### 📌 6. `std::forward<Args>(args)`

``` cpp
std::forward<Args>(args)
```

- 인자가 원래 lvalue였는지 rvalue였는지에 대한 값 범주를 보존한다.

``` cpp
std::string name = "Nurbs";
PrintSepTo(std::cout, ", ", name, std::string("Curve"));
```

- 일반적인 기본형 및 `std::string` 스트림 출력에서는 단순히 `args`를사용해도 대개 같은 결과를 얻는다.
- `std::forward`는 특히 값 범주에 따라 동작이 달라지는 사용자 정의 연산자에서 의미가 있다.

### 📌 7. 전체 표현식의 전개

- 원본:

``` cpp
((os << (std::exchange(first, false) ? "" : separator)
     << std::forward<Args>(args)), ...);
```

- 호출:

``` cpp
PrintSepTo(std::cout, ", ", 10, 20, 30);
```

- 동작을 풀어 쓰면:

``` cpp
bool first = true;

os << (std::exchange(first, false) ? "" : separator) << 10;
os << (std::exchange(first, false) ? "" : separator) << 20;
os << (std::exchange(first, false) ? "" : separator) << 30;
```

- 각 단계에서 `first`의 이전 값을 확인한 뒤 `false`로 변경한다.

### 📌 8. 람다를 사용한 가독성 중심 구현

``` cpp
template<typename... Args>
void PrintSepTo(std::ostream& os, const char* separator, Args&&... args)
{
    bool first = true;

    auto printOne = [&](const auto& value)
    {
        if (!first)
            os << separator;
        first = false;
        os << value;
    };

    (printOne(args), ...);
}
```

- 이 구현은 일반적인 숫자·문자열 출력에 적합하다. 단, `const auto&`를 사용하므로 원래 인자의 값 범주는 전달하지 않는다.

### 📌 9. Fold Expression의 네 가지 형태

| 구분                    | 문법                     |  예시 전개 (`a, b, c`) |
|-------------------------|--------------------------|-----------------------|
| Unary Left Fold        |  `(... op pack)`           | `((a op b) op c)` |
| Unary Right Fold        | `(pack op ...)`           | `(a op (b op c))` |
| Binary Left Fold        | `(init op ... op pack)`   | `(((init op a) op b) op c)` |
| Binary Right Fold       | `(pack op ... op init)`   | `(a op (b op (c op init)))` |


``` cpp
template<typename... Args>
auto Sum(Args... args)
{
    return (args + ...);
}
```

- `Sum(1, 2, 3, 4)`는 오른쪽 Fold이므로 `1 + (2 + (3 + 4))` 형태로 확장된다.
- 뺄셈·나눗셈 등에서는 결합 방향이 결과에 영향을 준다.

### 📌 10. 핵심 요약

- `typename... Args`: 서로 다른 타입의 인자를 여러 개 받는다.
- `(expression, ...)`: 매개변수 팩을 쉼표 연산자로 Fold한다.
- `std::exchange(first, false)`: 이전 `first` 값을 반환하면서 `false`로 변경한다.
- `? "" : separator`: 첫 번째 인자 앞의 구분자를 생략한다.
- `std::forward<Args>(args)`: 전달받은 인자의 값 범주를 보존한다.
- 출력 호출은 컴파일 시 전개되지만 실제 성능은 주로 스트림 I/O 비용에 좌우된다.

- **참고:** 인자가 0개이면 쉼표 Fold는 아무것도 수행하지 않는다. 구분자 역시 출력되지 않는다.

--- 
