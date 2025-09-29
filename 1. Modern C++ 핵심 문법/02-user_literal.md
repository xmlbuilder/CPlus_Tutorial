# 🧮 C++ 사용자 정의 리터럴 예제

이 프로젝트는 C++11 이상의 기능인 **사용자 정의 리터럴(User-defined literals)** 을 활용하여 단위 변환 및 이진 문자열 파싱을 구현한 예제입니다.

## ✨ 기능 소개

- `3.4_cm` → 센티미터를 밀리미터로 변환 (`3.4 * 10`)
- `13.0_m` → 미터를 밀리미터로 변환 (`13.0 * 1000`)
- `130.0_mm` → 밀리미터 그대로 사용
- `"110"_bin` → 이진 문자열을 정수로 변환 (`6`)
- `"1100110"_bin` → 이진 문자열을 정수로 변환 (`102`)

## 📦 요구 사항

- C++11 이상
- 표준 라이브러리 `<iostream>`

## 🚀 실행 방법

```bash
g++ -std=c++11 user_literals.cpp -o user_literals
./user_literals
```

## 📌 출력 예시
34
13034
0.01
6
102
1712753


## 📚 참고 소스

```cpp
#include <iostream>

// 사용자 정의 리터럴: 길이 단위 변환
constexpr long double operator"" _cm(long double x) { return x * 10; }     // cm → mm
constexpr long double operator"" _m(long double x)  { return x * 1000; }   // m → mm
constexpr long double operator"" _mm(long double x) { return x; }          // mm 그대로

// 사용자 정의 리터럴: 이진 문자열 → 정수
int operator"" _bin(const char* str, size_t l) {
    int ret = 0;
    for (size_t i = 0; i < l; ++i) {
        ret <<= 1;
        if (str[i] == '1') ret += 1;
    }
    return ret;
}

int main() {
    long double height = 3.4_cm;
    std::cout << height << std::endl;                      // 34
    std::cout << height + 13.0_m << std::endl;             // 13034
    std::cout << (130.0_mm / 13.0_m) << std::endl;         // 0.01

    std::cout << "110"_bin << std::endl;                   // 6
    std::cout << "1100110"_bin << std::endl;               // 102
    std::cout << "110100010001001110001"_bin << std::endl; // 1712753

    return 0;
}
```


