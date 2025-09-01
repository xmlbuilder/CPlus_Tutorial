# C# 코드에서는 정규식 수량자
C# 코드에서는 정규식 수량자 {n,m}를 사용해 특정 범위의 반복을 매칭하고 있죠. 

## ✅ 정규식 수량자 개요

| 수량자     | 설명                     | 예시 표현 | 매칭 가능한 문자열 예시         |
|------------|--------------------------|------------|----------------------------------|
| `*`        | 0회 이상 반복            | `a*`       | `""`, `"a"`, `"aa"`              |
| `+`        | 1회 이상 반복            | `a+`       | `"a"`, `"aa"`                    |
| `?`        | 0회 또는 1회             | `a?`       | `""`, `"a"`                      |
| `{n}`      | 정확히 n회 반복          | `a{3}`     | `"aaa"`                          |
| `{n,}`     | 최소 n회 이상 반복       | `a{2,}`    | `"aa"`, `"aaa"` 등               |
| `{n,m}`    | 최소 n회, 최대 m회 반복 | `a{2,4}`   | `"aa"`, `"aaa"`, `"aaaa"`        |


# 🧠 C#
```csharp
using System.Text.RegularExpressions;

namespace GrammarTest
{
    public class Program
    {
        public static void Main(String[] args)
        {
            var text1 = "a123456 b123 z12345 AX98765";
            var pattern1 = @"\b[a-zA-Z][0-9]{5,}\b"; 

            var matches1 = Regex.Matches(text1, pattern1);

            foreach (Match match in matches1)
            {
                Console.WriteLine(match.Value);
            }
            /*
             a123456
             z12345
             */


            var text2 = "삼겹살-84-58433, 상추-95838-488";
            var pattern2 = @"\p{IsHangulSyllables}+-[0-9]{2,3}-[0-9]+";

            var matches2 = Regex.Matches(text2, pattern2);
            foreach (Match match in matches2)
            {
                Console.WriteLine(match.Value);
            }
            /*
             삼겹살-84-58433
             */
        }
    }
}

```

## 🧠 C++ (with <regex> from C++11)
```cpp
#include <iostream>
#include <regex>
#include <string>

int main() {
    std::string text = "a123456 b123 z12345 AX98765";
    std::regex pattern(R"(\b[a-zA-Z][0-9]{5,}\b)");

    std::sregex_iterator begin(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    for (auto it = begin; it != end; ++it) {
        std::cout << it->str() << std::endl;
    }
}
```

- std::regex 사용
- R"()"는 raw string literal로 백슬래시 이스케이프를 피할 수 있음

## ☕ Java (with java.util.regex)
```java
import java.util.regex.*;
public class Main {
    public static void main(String[] args) {
        String text = "a123456 b123 z12345 AX98765";
        Pattern pattern = Pattern.compile("\\b[a-zA-Z][0-9]{5,}\\b");
        Matcher matcher = pattern.matcher(text);

        while (matcher.find()) {
            System.out.println(matcher.group());
        }
    }
}
```

- Pattern.compile()로 정규식 생성
- Matcher.find()로 반복 탐색

## 🐍 Python (with re module)
```python
import re

text = "a123456 b123 z12345 AX98765"
pattern = r"\b[a-zA-Z][0-9]{5,}\b"

matches = re.findall(pattern, text)
for match in matches:
    print(match)

```

- re.findall()은 매칭된 모든 결과를 리스트로 반환
- r""은 raw string literal

## 🈶 한글 정규식 매칭 방법 비교

C#에서는 \p{IsHangulSyllables} 같은 유니코드 속성으로 한글을 매칭했는데, 다른 언어에서는 다음과 같이 처리할 수 있어요:


| 언어     | 한글 매칭 정규식 예시                |
|----------|--------------------------------------|
| C#       | `\p{IsHangulSyllables}+`             |
| C++      | `[가-힣]+`                            |
| Java     | `[가-힣]+`                            |
| Python   | `[가-힣]+` 또는 `[\uAC00-\uD7A3]+`     |


---


