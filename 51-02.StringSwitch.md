# 🧠 C++ 문자열 기반 switch 문 구현 (해시 기반)
C++에서는 switch 문이 int, enum, char 등 정수형 타입만 지원하고, std::string이나 문자열 리터럴은 사용할 수 없습니다. 이를 해결하기 위해 해시 함수를 활용한 문자열 switch 문 구현을 소개합니다.

## 🔧 구현 코드
### 1. constexpr 해시 함수 정의
```cpp
constexpr unsigned int HashCode(const char* str)
{
    return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * HashCode(str + 1) : 8603;
}

constexpr unsigned int HashCode(const wchar_t* str)
{
    return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * HashCode(str + 1) : 8603;
}
```
- constexpr로 정의되어 컴파일 타임에 해시값 계산 가능
- 재귀적으로 문자열을 순회하며 해시값 생성
- 0xEDB8832Full은 CRC32에서 사용되는 다항식 기반 상수

### 2. 문자열 switch 문 사용 예시
```cpp
void main() {
    const char* lastname = "Lee";
    switch (HashCode(lastname))
    {
        case HashCode("Kim"):
            std::cout << "Kim" << std::endl;
            break;
        case HashCode("Lee"):
            std::cout << "Lee" << std::endl;
            break;
    }
}
```

- HashCode("Kim"), HashCode("Lee")는 컴파일 타임에 계산됨
- 런타임 문자열도 동일한 방식으로 해시 처리하여 비교 가능

## 🧪 Boost 및 STL 해시 예시

### Boost 해시 사용
```cpp
#include <boost/functional/hash.hpp>

int hashCode()
{
    boost::hash<std::string> string_hash;
    return string_hash("Hash me");
}
```

- Boost의 hash는 std::string을 지원하며 충돌 가능성은 낮지만 switch에는 사용 불가 (런타임 해시)
STL 해시 사용
```cpp
std::hash<std::string>()("foo");
```

- std::hash도 마찬가지로 런타임 해시이며 switch에는 직접 사용 불가

## ✅ 장점과 주의점
| 항목 | 설명 |
|------|------| 
| ✅ 장점 | switch 문법을 유지하면서 문자열 비교 가능 | 
| ⚠️ 주의 | 해시 충돌 가능성 존재 → 고유한 문자열만 사용 권장 | 
| ⚠️ 주의 | 해시 함수는 반드시 constexpr이어야 컴파일 타임 비교 가능 | 
| ⚠️ 주의 | 문자열 변경 시 해시값도 변경되므로 유지보수 시 주의 필요 | 



## 💡 개선 아이디어
- 해시 충돌 방지를 위해 더 정교한 해시 알고리즘 사용 가능
- enum class와 매핑하여 가독성 향상
- constexpr std::string_view와 조합하면 더 유연한 구조 가능
