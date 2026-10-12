## 📘 C++ 정규표현식 핵심 개념

- C++에서는 `<regex>` 헤더를 사용해 문자열 패턴을 검사하거나 검색·추출·치환할 수 있음.
- 아래는 **표준 `std::regex` 문법**

### 📌 기본 구조

```cpp
#include <regex>
#include <string>

std::regex pattern(R"(\d{2,3}-\d{3,4}-\d{4})");
std::string text = "Call 032-232-3245";
std::smatch match;
if (std::regex_search(text, match, pattern)) {
    std::string phone = match.str(0); // 032-232-3245
}
```

### 📌 주요 정규표현식 패턴

| 패턴 | 설명 | 예시 |
|---|---|---|
| `\d` | 숫자 하나 | `0`, `9` |
| `\d{3}` | 숫자 3자리 | `123` |
| `\w` | 영문·숫자·밑줄(일반적인 ASCII 입력 기준) | `a`, `9`, `_` |
| `\s` | 공백 문자 | 공백, 탭, 개행 |
| `.` | 임의 문자 하나(기본적으로 개행 제외) | `a`, `1` |
| `^` / `$` | 문자열 시작 / 끝 | `^Hello`, `world$` |
| `()` | 캡처 그룹 | `(\d{3})-(\d{4})` |
| `\|` | OR 조건 | `cat\|dog` |
| `[XYZ]` | X, Y, Z 중 하나 | `X` |
| `[A-Z]` | 문자 범위 | `K` |
| `[^XYZ]` | X, Y, Z가 아닌 문자 하나 | `A` |
| `*` / `+` / `?` | 앞 표현식 0회 이상 / 1회 이상 / 0~1회 | `a*`, `a+`, `a?` |
| `{n,m}` | 앞 표현식 n~m회 반복 | `\d{2,4}` |

### 📌 자주 쓰는 함수

| 함수 | 설명 |
|---|---|
| `std::regex_match()` | **문자열 전체** 가 패턴과 일치하는지 검사 |
| `std::regex_search()` | 문자열 중 **일부** 가 일치하는지 검색 |
| `std::regex_replace()` | 일치 부분을 다른 문자열로 치환 |
| `std::sregex_iterator` | 문자열의 **모든 일치 항목** 순회 |
| `std::smatch` | 매칭된 전체 문자열 및 캡처 그룹 보관 |

### 📌 실전 예제

#### 1️⃣ 전화번호 추출

```cpp
std::string text = "문의: 032-232-3245";
std::regex re(R"(\d{2,3}-\d{3,4}-\d{4})");
std::smatch m;
if (std::regex_search(text, m, re))
    std::cout << m.str(0) << '\n';  // 032-232-3245
```

#### 2️⃣ 여러 에러 코드 추출

```cpp
std::string text = "에러 1122 : 오류\n에러 1033 : 오류";
std::regex re(R"(에러\s+(\d+))");
for (std::sregex_iterator it(text.begin(), text.end(), re), end;
     it != end; ++it) {
    std::cout << (*it)[1].str() << '\n'; // 1122, 1033
}
```

#### 3️⃣ 캡처 그룹으로 전화번호 분리

```cpp
std::string text = "032-232-3245";
std::regex re(R"((\d{3})-(\d{3}-\d{4}))");
std::smatch m;
if (std::regex_search(text, m, re)) {
    std::cout << m[1].str() << '\n'; // 032
    std::cout << m[2].str() << '\n'; // 232-3245
}
```

#### 4️⃣ 전체 일치와 일부 검색

```cpp
std::regex re(R"(AB\d{2})");
std::regex_match(std::string("AB12"), re);      // true
std::regex_match(std::string("XAB12Y"), re);    // false
std::regex_search(std::string("XAB12Y"), re);   // true
```

### 📌  팁

- 정규식 문자열은 `R"(...)"` **raw string literal**을 사용하면 `\\d` 대신 `\d`로 적을 수 있음.
- `m[0]`은 전체 일치, `m[1]`, `m[2]`는 첫 번째·두 번째 캡처 그룹.
- 같은 패턴으로 많은 문자열을 검색할 때는 `std::regex` 객체를 **반복문 밖에서 한 번만 생성**.
- 잘못된 정규식은 `std::regex_error` 예외가 발생할 수 있음.

---

