# 🚗 auto란?
auto는 컴파일러가 변수의 타입을 자동으로 추론하도록 하는 키워드예요. C11부터 도입되었고, 이후 C14, C17, C20에서 점점 더 강력해졌습니다.
```cpp
auto x = 10;        // int
auto y = 3.14;      // double
auto name = "Jung"; // const char*
```


## 🔍 언제 쓰면 좋을까?
- 복잡한 타입을 간결하게 표현할 때
- 템플릿 코드에서 유연하게 타입을 처리할 때
- STL 반복자(iterator)나 람다 표현식에서 자주 사용
```cpp
std::vector<int> v = {1, 2, 3};
for (auto it = v.begin(); it != v.end(); ++it) {
    std::cout << *it << " ";
}
```


## 🧠 타입 추론 규칙
- auto는 값의 타입을 기준으로 추론합니다.
- 참조나 const는 제거되므로, 필요하면 명시적으로 붙여야 함.
```cpp
int a = 42;
const int& ref = a;

auto x = ref;        // int (const와 참조 제거됨)
auto& y = ref;       // const int& (참조 유지)
const auto z = ref;  // const int
```



## 🧪 함수에서의 auto
### 1. 반환 타입 추론 (C++14 이상)
```cpp
auto add(int a, int b) {
    return a + b; // 반환 타입 자동 추론
}
```

### 2. 람다 표현식
```cpp
auto f = [](auto x, auto y) {
    return x + y;
};

std::cout << f(3, 4.5); // 7.5
```


## 🧬 템플릿에서의 auto
### ✅ Non-type 템플릿 파라미터 (C++17)
```cpp
template<auto N>
void print() {
    std::cout << N << std::endl;
}

print<100>();     // int
print<'A'>();     // char
```

### ✅ decltype(auto) (C++14)
decltype(auto)는 표현식의 정확한 타입을 유지하면서 추론합니다.
```cpp
int x = 10;
decltype(auto) y = (x); // int&
```


### ⚠️ 주의할 점
| 상황 | 설명 | 
|------|-----|
| auto는 값 기반 추론 | 참조나 const는 제거됨 | 
| 초기화 없이 사용 불가 | auto x; → 오류 | 
| 명확한 타입이 필요한 경우 피해야 함 | 예: API 설계, 인터페이스 정의 | 



## ✨ 요약
| 버전 | 기능 | 
|------|-----|
| C++11 | 기본 타입 추론, 반복자, 람다 | 
| C++14 | 반환 타입 추론, decltype(auto) | 
| C++17 | auto 템플릿 파라미터 | 
| C++20 | auto 매개변수, 콘셉트와 함께 사용 가능 | 

---
# 🧠 Non-Type Template Parameter란?
템플릿 인자에는 타입뿐 아니라 **값(value)**도 전달할 수 있어요. 이를 non-type template parameter라고 함.
template<typename T, int BufSize>
class buffer_t { ... };


여기서 BufSize는 타입이 아닌 정수 값을 템플릿 인자로 받음.

## ⚙️ C++17 이후: auto를 이용한 Non-Type Template Parameter
C++17부터는 non-type 템플릿 파라미터에 **auto**를 사용할 수 있게 됨.  
이건 매우 유용한 기능, 템플릿 인자의 타입을 자동으로 추론해줍니다.
```cpp
template <auto value>
void foo() {
    std::cout << value << std::endl;
}
```

이렇게 하면 value의 타입을 명시하지 않아도 컴파일러가 알아서 추론.
## ✅ 예시
```cpp
foo<10>();        // value는 int로 추론됨
foo<'A'>();       // value는 char
foo<true>();      // value는 bool
foo<3.14>();      // value는 double
```


##📌 장점 요약
| 기능 | 설명 |
|------|-----| 
| auto 추론 | 템플릿 인자의 타입을 자동으로 추론 | 
| 코드 간결화 | 타입 명시 없이 다양한 타입 지원 | 
| 범용성 증가 | 하나의 템플릿으로 여러 타입 처리 가능 | 



##🧪 constexpr과 함께 쓰기
```cpp
constexpr int a = 30;
buffer_t<int, a> buf1{};
```

이처럼 non-type 템플릿 파라미터는 컴파일 타임 상수여야 하므로 constexpr과 함께 자주 사용됨.

## ✨ 보너스: auto 템플릿 인자와 decltype
템플릿 내부에서 value의 타입을 알고 싶다면 decltype(value)를 사용할 수 있음.
```cpp
template <auto value>
void foo() {
    std::cout << "Type: " << typeid(decltype(value)).name() << "\n";
}
```


## 💡 정리
- template<auto value>는 C++17 이후 도입된 기능
- 타입을 명시하지 않아도 컴파일러가 자동 추론
- 다양한 타입의 상수를 템플릿 인자로 받을 수 있음
- constexpr과 함께 쓰면 컴파일 타임 최적화에 유리
