## 📘 RegularExpressionUtil 사용법과 개념 정리

- `RegularExpressionUtil`은 복잡한 `std::regex` 문법 대신 `?`, `*`, `[XYZ]` 등의 간단한 패턴으로 문자열을 검색하는 **유틸리티** 입니다.
- 단일 문자열 비교뿐 아니라 `std::vector<std::string>`에서 조건에 맞는 문자열만 추출할 수 있습니다.
- 내부적으로 `<regex>`를 사용하며, 주로 CAD/CAE 채널명·부품명처럼 ASCII 기반 식별자 검색에 적합합니다.

### 📌 정의

```cpp
#include "RegularExpressionUtil.h"

class RegularExpressionUtil {
public:
    static std::string ToRegex(const std::string& pattern);
    static bool Match(const std::string& text,
        const std::string& pattern, bool ignoreCase = false);
    static bool Contains(const std::string& text,
        const std::string& pattern, bool ignoreCase = false);
    static std::vector<std::string> Filter(
        const std::vector<std::string>& values,
        const std::string& pattern, bool ignoreCase = false);
    static std::vector<std::string> FilterContains(
        const std::vector<std::string>& values,
        const std::string& pattern, bool ignoreCase = false);
};
```

- `text`: 검사할 문자열
- `values`: 검색할 문자열 목록
- `pattern`: 간단한 와일드카드 검색식
- `ignoreCase`: `true`이면 대소문자 구분 없이 검색 (기본값 `false`)

### 📌 패턴 문법

| 패턴 | 의미 | 예시 |
|------|------|------|
| `?` | 임의의 문자 정확히 1개 | `AB?D` → `ABCD` |
| `*` | 임의의 문자 0개 이상 | `AB*D` → `ABD`, `AB123D` |
| `[XYZ]` | X, Y, Z 중 한 문자 | `AC[XYZ]` → `ACX` |
| `[A-Z]` | A~Z 중 한 문자 | `AC[A-Z]` → `ACF` |
| `[0-9]` | 숫자 한 문자 | `TH0[1-9]` → `TH03` |
| `[!XYZ]` | X, Y, Z를 제외한 한 문자 | `AC[!XYZ]` → `ACA` |
| `\?`, `\*`, `\[` | 특수 기호 자체 | `A\?B` → `A?B` |

- `.` , `+`, `(`, `)` 등의 일반 정규식 기호는 패턴 바깥에서 **문자 그대로** 처리합니다.
- C++ 문자열 리터럴에서 역슬래시를 사용할 때는 `"A\\?B"`처럼 이중으로 쓰거나 `R"(A\?B)"` 원시 문자열을 사용합니다.

### 📌 핵심 개념

| 함수 | 반환형 | 검색 기준 |
|------|--------|----------|
| `Match()` | `bool` | **전체 문자열** 이 패턴과 일치 |
| `Contains()` | `bool` | 문자열 **일부** 가 패턴과 일치 |
| `Filter()` | `vector<string>` | 전체 일치하는 원소만 추출 |
| `FilterContains()` | `vector<string>` | 일부 일치하는 원소만 추출 |
| `ToRegex()` | `string` | 간단한 패턴을 ECMAScript 정규식 문자열로 변환 (앵커 없음) |

- `Filter()`와 `FilterContains()`는 **새로운 vector** 를 반환하며 입력 vector를 변경하지 않습니다.
- 결과의 **원래 순서와 중복 원소** 를 유지합니다.
- vector 검색 시 정규식을 **함수 호출당 한 번만 컴파일** 합니다.

### 📌 Match와 Contains 차이

```cpp
using R = RegularExpressionUtil;

bool a = R::Match("00ABTH01ACX", "00??TH01AC[XYZ]");
// true: 문자열 전체가 일치

bool b = R::Match("PRE_00ABTH01ACX_END", "00??TH01AC[XYZ]");
// false: 앞뒤에 다른 문자열이 존재

bool c = R::Contains("PRE_00ABTH01ACX_END", "00??TH01AC[XYZ]");
// true: 문자열 일부에 해당 패턴이 존재
```

### 📌 vector 필터링 예제

```cpp
#include "RegularExpressionUtil.h"
#include <string>
#include <vector>

int main() {
    std::vector<std::string> channels = {
        "00ABTH01ACX",
        "00ABTH01ACY",
        "00ABTH01ACZ",
        "00CDTH01ACX",
        "00ABTH02ACX"
    };

    // ?? = 정확히 두 문자, [XYZ] = X/Y/Z 중 하나
    auto selected = RegularExpressionUtil::Filter(
        channels, "00??TH01AC[XYZ]");

    // selected = {
    //   "00ABTH01ACX", "00ABTH01ACY",
    //   "00ABTH01ACZ", "00CDTH01ACX"
    // };
}
```

### 📌 부분 문자열 검색 예제

```cpp
std::vector<std::string> names = {
    "PRE_00ABTH01ACX_END",
    "PRE_00CDTH01ACY_END",
    "00ABTH02ACZ",
    "OTHER"
};

auto result = RegularExpressionUtil::FilterContains(
    names, "TH01AC[XYZ]");

// result = {
//   "PRE_00ABTH01ACX_END",
//   "PRE_00CDTH01ACY_END"
// };
```

### 📌 대소문자 무시 및 와일드카드

```cpp
using R = RegularExpressionUtil;

bool a = R::Match("00ABTH01ACX", "00??th01ac[xyz]", true);
// true: 대소문자 무시

bool b = R::Match("TH01ACZ", "TH*ACZ");
// true: *는 0개 이상의 문자에 대응

bool c = R::Match("THACZ", "TH*ACZ");
// true: *는 빈 문자열에도 대응

bool d = R::Match("THACZ", "TH?ACZ");
// false: ?는 정확히 한 문자가 필요
```

### 📌 특수문자 검색 및 정규식 변환

```cpp
using R = RegularExpressionUtil;

bool literal = R::Match("A?B", R"(A\?B)");
// true: \?는 물음표 자체

std::string expression = R::ToRegex("00??TH01AC[XYZ]");
// "00..TH01AC[XYZ]"
// ToRegex 결과에는 ^, $가 붙지 않음
```

### 📌 잘못된 패턴 처리

```cpp
#include <stdexcept>

try {
    bool ok = RegularExpressionUtil::Match("ABC", "AB[");
} catch (const std::invalid_argument& e) {
    // 닫는 ]가 없는 잘못된 패턴
}
```

- 닫히지 않은 `[` 또는 빈 문자 클래스 `[]`, 끝에 단독으로 있는 `\` 등은 `std::invalid_argument`를 발생시킵니다.
- 지원하지 않는 복잡한 문자 클래스는 정규식 규칙의 영향을 받을 수 있으므로, 단순 패턴 위주로 사용합니다.

### 📌 요약

- 문자열 **전체 일치** 는 `Match()` / `Filter()`를 사용합니다.
- 문자열 **일부 검색** 은 `Contains()` / `FilterContains()`를 사용합니다.
- `?`는 한 문자, `*`는 0개 이상, `[XYZ]`는 지정 문자 중 하나를 의미합니다.
- 검색 결과 vector는 새로 만들어지고 원본의 순서와 중복은 유지됩니다.
- ASCII 기반 식별자용입니다. UTF-8 한글 등에서 `?`는 유니코드 문자 한 개가 아니라 **바이트 한 개** 에 대응합니다.
- `*`와 `?`는 내부 정규식의 `.` 기반이므로 개행 문자는 매칭하지 않습니다.

---

### 📌 소스 코드
```cpp
#pragma once

#include <regex>
#include <string>
#include <vector>

// Simple wildcard pattern utility (C++17).
// ? : exactly one character; * : zero or more characters
// [ABC], [A-Z], [0-9], [!ABC] : character classes
// Backslash escapes a wildcard metacharacter outside character classes.
// Character classes support literal '-' at the beginning/end and literal ']' via \].
// Invalid patterns throw std::invalid_argument.
class RegularExpressionUtil {
public:
    static std::string ToRegex(const std::string& pattern);
    static bool Match(const std::string& text, const std::string& pattern, bool ignoreCase = false);
    static bool Contains(const std::string& text, const std::string& pattern, bool ignoreCase = false);
    static std::vector<std::string> Filter(const std::vector<std::string>& values,
        const std::string& pattern, bool ignoreCase = false);
    static std::vector<std::string> FilterContains(const std::vector<std::string>& values,
        const std::string& pattern, bool ignoreCase = false);
private:
    static std::regex Compile(const std::string& pattern, bool ignoreCase);
};
```

```cpp
#include "regexp_utils.h"
#include <stdexcept>

namespace {
bool regexMeta(char c) {
    return c == '.' || c == '^' || c == '$' || c == '|' || c == '(' || c == ')' ||
           c == '{' || c == '}' || c == '+' || c == '\\' || c == '[' || c == ']' ||
           c == '*' || c == '?';
}

void appendLiteral(std::string& out, char c) {
    if (regexMeta(c)) out += '\\';
    out += c;
}
}

std::string RegularExpressionUtil::ToRegex(const std::string& pattern) {
    std::string out;
    out.reserve(pattern.size() * 2);
    for (size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        if (c == '\\') {
            if (++i == pattern.size()) throw std::invalid_argument("Trailing escape in pattern");
            appendLiteral(out, pattern[i]);
        } else if (c == '?') {
            out += '.';
        } else if (c == '*') {
            out += ".*";
        } else if (c == '[') {
            size_t pos = i + 1;
            bool negated = pos < pattern.size() && pattern[pos] == '!';
            if (negated) ++pos;
            std::string cls;
            bool first = true;
            bool closed = false;
            for (; pos < pattern.size(); ++pos) {
                char x = pattern[pos];
                bool escaped = false;
                if (x == '\\') {
                    if (++pos == pattern.size()) throw std::invalid_argument("Trailing escape in character class");
                    x = pattern[pos];
                    escaped = true;
                } else if (x == ']') {
                    closed = true;
                    break;
                }
                if (x == '^' && first) cls += '\\';
                if (x == '[' || x == ']' || x == '\\') cls += '\\';
                if (escaped && x == '-') cls += '\\';
                cls += x;
                first = false;
            }
            if (!closed || cls.empty()) throw std::invalid_argument("Unclosed or empty character class");
            // A trailing '-' is literal in ECMAScript character classes.
            out += '[';
            if (negated) out += '^';
            out += cls;
            out += ']';
            i = pos;
        } else {
            appendLiteral(out, c);
        }
    }
    return out;
}

std::regex RegularExpressionUtil::Compile(const std::string& pattern, bool ignoreCase) {
    auto flags = std::regex_constants::ECMAScript;
    if (ignoreCase) flags |= std::regex_constants::icase;
    try {
        return std::regex(ToRegex(pattern), flags);
    } catch (const std::regex_error& e) {
        throw std::invalid_argument(std::string("Invalid pattern: ") + e.what());
    }
}

bool RegularExpressionUtil::Match(const std::string& text, const std::string& pattern, bool ignoreCase) {
    return std::regex_match(text, Compile(pattern, ignoreCase));
}

bool RegularExpressionUtil::Contains(const std::string& text, const std::string& pattern, bool ignoreCase) {
    return std::regex_search(text, Compile(pattern, ignoreCase));
}

std::vector<std::string> RegularExpressionUtil::Filter(const std::vector<std::string>& values,
    const std::string& pattern, bool ignoreCase) {
    auto re = Compile(pattern, ignoreCase);
    std::vector<std::string> result;
    for (const auto& value : values)
        if (std::regex_match(value, re)) result.push_back(value);
    return result;
}
std::vector<std::string> RegularExpressionUtil::FilterContains(const std::vector<std::string>& values,
    const std::string& pattern, bool ignoreCase) {
    auto re = Compile(pattern, ignoreCase);
    std::vector<std::string> result;
    for (const auto& value : values)
        if (std::regex_search(value, re)) result.push_back(value);
    return result;
}
```
### 📌 테스트 코드
```cpp
#include "regexp_utils.h"
#include <iostream>
#include "test_expreg_utils.h"


namespace {

int passed = 0, failed = 0;
void Check(bool actual, bool expected, const std::string& label) {
    bool ok = actual == expected;
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << label << " (expected=" << expected << ", actual=" << actual << ")\n";
    ok ? ++passed : ++failed;
}

void CheckList(const std::vector<std::string>& actual, const std::vector<std::string>& expected,
               const std::string& label) {
    bool ok = actual == expected;
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << label << " -> [";
    for (size_t i = 0; i < actual.size(); ++i) std::cout << (i ? ", " : "") << actual[i];
    std::cout << "]\n";
    ok ? ++passed : ++failed;
}

void CheckInvalid(const std::string& pattern) {
    bool caught = false;
    try { (void)RegularExpressionUtil::Match("test", pattern); }
    catch (const std::invalid_argument&) { caught = true; }
    Check(caught, true, "invalid pattern: " + pattern);
}

}

int test_expreg_utils::run_tests() {
    using R = RegularExpressionUtil;
    std::cout << std::boolalpha;

    std::cout << "=== 1. Single string: ? = exactly one character ===\n";
    Check(R::Match("00ABTH01ACZ", "00??TH01ACZ"), true, "two unknown characters");
    Check(R::Match("00ATH01ACZ", "00??TH01ACZ"), false, "one character is insufficient");
    Check(R::Match("00ABCTH01ACZ", "00??TH01ACZ"), false, "three characters is too many");
    Check(R::Match("ABCD", "AB?D"), true, "single ?");
    Check(R::Match("ABD", "AB?D"), false, "? cannot match empty");

    std::cout << "\n=== 2. * = zero or more characters ===\n";
    Check(R::Match("ABCD", "AB*D"), true, "* matches C");
    Check(R::Match("ABD", "AB*D"), true, "* matches zero characters");
    Check(R::Match("AB12345D", "AB*D"), true, "* matches many characters");
    Check(R::Match("AB12345X", "AB*D"), false, "suffix must match");
    Check(R::Match("", "*"), true, "* matches empty string");

    std::cout << "\n=== 3. Character sets and ranges ===\n";
    Check(R::Match("00ABTH01ACX", "00??TH01AC[XYZ]"), true, "X in [XYZ]");
    Check(R::Match("00ABTH01ACY", "00??TH01AC[XYZ]"), true, "Y in [XYZ]");
    Check(R::Match("00ABTH01ACZ", "00??TH01AC[XYZ]"), true, "Z in [XYZ]");
    Check(R::Match("00ABTH01ACQ", "00??TH01AC[XYZ]"), false, "Q not in [XYZ]");
    Check(R::Match("ABC7", "ABC[0-9]"), true, "digit range");
    Check(R::Match("ABCF", "ABC[A-Z]"), true, "letter range");
    Check(R::Match("ABCa", "ABC[A-Z]"), false, "case-sensitive range");
    Check(R::Match("ABCa", "ABC[A-Z]", true), true, "case-insensitive range");
    Check(R::Match("ABCQ", "ABC[!XYZ]"), true, "negated class accepts Q");
    Check(R::Match("ABCX", "ABC[!XYZ]"), false, "negated class rejects X");

    std::cout << "\n=== 4. Match (whole) versus Contains (substring) ===\n";
    Check(R::Match("prefix_00ABTH01ACZ_suffix", "00??TH01ACZ"), false, "Match requires whole text");
    Check(R::Contains("prefix_00ABTH01ACZ_suffix", "00??TH01ACZ"), true, "Contains finds substring");
    Check(R::Contains("prefix_00ABTH01ACQ_suffix", "00??TH01AC[XYZ]"), false, "Contains rejects wrong suffix");
    Check(R::Match("", ""), true, "empty text matches empty pattern");
    Check(R::Contains("anything", ""), true, "empty pattern matches empty substring");

    std::cout << "\n=== 5. Case sensitivity ===\n";
    Check(R::Match("abc", "ABC"), false, "default is case-sensitive");
    Check(R::Match("abc", "ABC", true), true, "ignoreCase=true");
    Check(R::Contains("part_abcz_end", "ABC[XYZ]", true), true, "Contains ignores case");

    std::cout << "\n=== 6. Literal punctuation and escaping ===\n";
    Check(R::Match("part.01+", "part.01+"), true, "regex punctuation is literal");
    Check(R::Match("file?.txt", R"(file\?.txt)"), true, "literal question mark with escape");
    Check(R::Match("file*.txt", R"(file\*.txt)"), true, "literal star with escape");
    Check(R::Match("[A]", R"(\[A\])"), true, "literal square brackets");
    Check(R::Match("A-C", "[A-C][A-C][A-C]"), false, "range does not accept dash");

    std::cout << "\n=== 7. Filter: return matching vector, preserve order/duplicates ===\n";
    const std::vector<std::string> channels = {
        "00ABTH01ACX", "00ABTH01ACY", "00ABTH01ACZ",
        "00CDTH01ACX", "00ABTH02ACX", "00ABTH01ACQ", "00ABTH01ACX"
    };
    CheckList(R::Filter(channels, "00??TH01AC[XYZ]"),
              {"00ABTH01ACX", "00ABTH01ACY", "00ABTH01ACZ", "00CDTH01ACX", "00ABTH01ACX"},
              "Filter channel names");
    CheckList(R::Filter(channels, "00ABTH02AC?"), {"00ABTH02ACX"}, "Filter one channel group");
    CheckList(R::Filter(channels, "NO_MATCH*"), {}, "no matches -> empty vector");
    CheckList(R::Filter({}, "*"), {}, "empty input vector");
    CheckList(R::Filter({"a", "A", "b"}, "A", true), {"a", "A"}, "case-insensitive Filter");

    std::cout << "\n=== 8. FilterContains: substring filtering ===\n";
    const std::vector<std::string> labels = {"PRE_00ABTH01ACX_END", "00CDTH01ACY", "TH02ACZ", "OTHER"};
    CheckList(R::Filter(labels, "00??TH01AC[XYZ]"), {"00CDTH01ACY"}, "Filter whole names only");
    CheckList(R::FilterContains(labels, "00??TH01AC[XYZ]"),
              {"PRE_00ABTH01ACX_END", "00CDTH01ACY"}, "FilterContains substring names");
    CheckList(R::FilterContains(labels, "TH02"), {"TH02ACZ"}, "search short substring");

    std::cout << "\n=== 9. ToRegex: inspect generated ECMAScript expression ===\n";
    std::cout << "  00??TH01AC[XYZ] => " << R::ToRegex("00??TH01AC[XYZ]") << '\n';
    Check(R::ToRegex("00??TH01AC[XYZ]") == "00..TH01AC[XYZ]", true, "conversion output");
    Check(R::ToRegex("file.*") == R"(file\..*)", true, "dot literal, star wildcard");

    std::cout << "\n=== 10. Invalid pattern diagnostics ===\n";
    CheckInvalid("ABC[");
    CheckInvalid("ABC[]");
    CheckInvalid("ABC[!]");
    CheckInvalid("ABC\\");
    CheckInvalid("ABC[Z-A]");

    std::cout << "\n=== Summary ===\nPASS: " << passed << "\nFAIL: " << failed << '\n';
    return failed ? 1 : 0;
}
```
---

