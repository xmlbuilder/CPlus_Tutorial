# 🧩 1. Ranges: 뷰 파이프라인, 프로젝션, std::views::chunk
## 📌 개념 요약
- ranges는 STL 알고리즘과 컨테이너를 더 유연하게 연결해주는 기능
- **뷰(view)**는 원본 데이터를 복사하지 않고 지연 평가(lazy evaluation) 방식으로 처리
- 파이프라인() 문법으로 여러 뷰를 연결 가능
## 🧪 예제: 파이프라인 + 프로젝션
```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> nums = {1, 2, 3, 4, 5, 6};

    auto result = nums 
        | std::views::filter([](int x) { return x % 2 == 0; })  // 짝수 필터
        | std::views::transform([](int x) { return x * x; });   // 제곱

    for (int x : result)
        std::cout << x << " ";  // 출력: 4 16 36
}
```

## 🧪 예제: std::views::chunk
```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {1,2,3,4,5,6,7,8};

    auto chunks = data | std::views::chunk(3);

    for (auto&& group : chunks) {
        for (int x : group)
            std::cout << x << " ";
        std::cout << "\n";
    }
}
```

📌 출력:
```
1 2 3
4 5 6
7 8
```
---

# 🧠 2. Concepts & requires: 템플릿 제약
## 📌 개념 요약
- concepts는 템플릿 인자에 대해 제약 조건을 명시하는 기능
- requires 키워드로 조건을 직접 기술하거나, 표준 concept (std::integral, std::floating_point) 사용 가능
## 🧪 예제: 정수 타입만 허용하는 템플릿
```cpp
#include <concepts>
#include <iostream>

template<std::integral T>
T add(T a, T b) {
    return a + b;
}

int main() {
    std::cout << add(3, 4) << "\n";     // OK
    // std::cout << add(3.5, 4.2);      // 컴파일 오류: double은 integral 아님
}
```

## 🧪 예제: requires 표현식
```cpp
template<typename T>
requires requires(T x) {
    { x.size() } -> std::convertible_to<std::size_t>;
}
void print_size(const T& x) {
    std::cout << "Size: " << x.size() << "\n";
}
```
---

# 📦 3. std::span: 연속 메모리 뷰
## 📌 개념 요약
- std::span은 배열, 벡터, 포인터 등 연속된 메모리 블록을 참조하는 lightweight 뷰
- 복사 없이 안전하게 범위 접근 가능
- 크기 고정 (std::span<int, 5>) 또는 동적 (std::span<int>)
## 🧪 예제
```cpp
#include <span>
#include <vector>
#include <iostream>

void print(std::span<int> s) {
    for (int x : s)
        std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    std::vector<int> v = {10, 20, 30, 40};
    print(v); // vector도 span으로 전달 가능

    int arr[] = {1, 2, 3};
    print(arr); // 배열도 OK
}
```


# 🧵 4. std::string_view: 문자열 뷰
## 📌 개념 요약
- std::string_view는 문자열을 복사하지 않고 읽기 전용 참조로 처리
- const char*, std::string 모두에서 생성 가능
- 매우 빠르고 메모리 효율적
## 🧪 예제
```cpp
#include <string_view>
#include <iostream>

void greet(std::string_view name) {
    std::cout << "Hello, " << name << "!\n";
}

int main() {
    std::string s = "JungHwan";
    greet(s);               // string
    greet("C++ Developer"); // 문자열 리터럴
}
```


## ✨ 요약 테이블
| 기능 | 설명 | 장점 |
|------|-----|------| 
| ranges | 지연 평가 뷰 파이프라인 | 가독성, 성능 | 
| concepts | 템플릿 타입 제약 | 타입 안전, 명확한 오류 | 
| std::span | 연속 메모리 참조 | 복사 없음, 범용성 | 
| string_view | 문자열 참조 | 빠름, 메모리 절약 | 

-----



