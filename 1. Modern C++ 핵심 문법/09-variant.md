## 📘 std::variant 완전 정리
### 1️⃣ 기본 개념
- std::variant는 여러 타입 중 하나만 저장할 수 있는 타입 안전한 유니언입니다.
```cpp
std::variant<int, std::string, double> v;
```

- v는 int, std::string, double 중 하나의 값을 가질 수 있음
- 런타임에 어떤 타입이 저장되어 있는지 확인 가능

### 2️⃣ 값 설정과 타입 확인
```cpp
std::variant<int, std::string, double> v = 1;
v = "abc";
v = 3.14;
```

- v.index() → 현재 저장된 타입의 인덱스 반환 (0부터 시작)
- std::get<T>(v) → 저장된 값을 지정한 타입으로 가져옴
- std::get<I>(v) → 인덱스로 접근도 가능
  
#### 🔍 출력 예시
```cpp
std::cout << v.index();           // 2 (double)
std::cout << std::get<double>(v); // 3.14
```

### 3️⃣ 안전한 접근 방법
- std::get<T>는 타입이 다르면 예외 발생
```cpp
try {
    auto i = std::get<int>(v); // 현재 타입이 int가 아니면 std::bad_variant_access 발생
} catch (const std::bad_variant_access& e) {
    std::cout << e.what(); // "bad_variant_access"
}
```

#### 🔹 std::holds_alternative<T>(v)로 타입 확인
```cpp
if (std::holds_alternative<std::string>(v)) {
    std::cout << std::get<std::string>(v);
}
```

#### 🔹 std::get_if<T>(&v)로 포인터 접근
```cpp
if (auto ptr = std::get_if<std::string>(&v)) {
    std::cout << *ptr;
}
```

- 타입이 맞지 않으면 nullptr 반환
- 예외 없이 안전하게 접근 가능

### 4️⃣ 인덱스 기반 접근
```cpp
std::variant<int, double, std::string> vunion = 3.5;
std::cout << std::get<1>(vunion); // double 타입이 1번째 인덱스
```

- 인덱스는 선언 순서 기준
- std::get<0>, std::get<1> 등으로 접근 가능

### 5️⃣ 문자열 리터럴과 std::string
```cpp
std::variant<int, double, std::string> v = "abc"; // const char[] → 암시적 변환 안 됨
```

- "abc"는 const char* → std::string으로 변환 필요
- "abc"s 또는 std::string("abc") 사용해야 정확히 매칭됨

### 6️⃣ 보강: 방문자 패턴 (std::visit)
```cpp
std::variant<int, std::string, double> v = 3.14;

std::visit([](auto&& arg) {
    std::cout << "Value: " << arg << std::endl;
}, v);
```

- std::visit은 현재 저장된 타입에 따라 람다를 호출
- 모든 타입을 처리하는 공통 인터페이스로 유용함

### 📌 요약
| 기능 | 설명 | 예시 | 
|------|-----|------|
| std::variant<Ts...> | 여러 타입 중 하나 저장 | std::variant<int, std::string> | 
| v.index() | 현재 저장된 타입의 인덱스 반환 | v.index() | 
| std::get<T>(v) | 지정 타입으로 값 가져오기 (예외 발생 가능) | std::get<int>(v) | 
| std::holds_alternative<T>(v) | 현재 타입인지 확인 | std::holds_alternative<double>(v) | 
| std::get_if<T>(&v) | 포인터로 안전하게 접근 | if (auto p = std::get_if<T>(&v)) | 
| std::visit | 방문자 패턴으로 타입별 처리 | std::visit([](auto&& x){}, v) | 

---
