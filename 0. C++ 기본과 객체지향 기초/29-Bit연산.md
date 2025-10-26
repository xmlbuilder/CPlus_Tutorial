# Bit 연산
Integer.toBinaryString()은 Java에서 정수를 2진 문자열로 변환하는 메서드인데,  
Rust와 C++에서도 비슷한 기능을 직접 구현할 수 있음.  
아래에 두 언어로 각각 예제.  

## 🦀 Rust 버전
```rust
fn to_binary_string(n: u32) -> String {
    format!("{:b}", n)
}

fn main() {
    let value = 2147483648;
    let binary = to_binary_string(value);
    println!("Binary: {}", binary); // "10000000000000000000000000000000"
}
```

### ✅ 설명
- {:b}는 Rust의 포맷 지정자로, 2진수 출력
- u32, u64 등 부호 없는 타입을 쓰면 `>>>` 같은 논리 시프트도 자연스럽게 처리됨

## 💠 C++ 버전 (C++11 이상) - `std::bitset<32>(n).to_string()`
```cpp
#include <iostream>
#include <bitset>

std::string to_binary_string(uint32_t n) {
    return std::bitset<32>(n).to_string();
}

int main() {
    uint32_t value = 2147483648;
    std::string binary = to_binary_string(value);
    std::cout << "Binary: " << binary << std::endl;
}
```

### ✅ 설명
- std::bitset<32>은 32비트 정수를 2진 문자열로 변환
- to_string()으로 문자열 반환

## 📦 요약 비교: 정수 → 2진 문자열 변환

| 언어   | 메서드 또는 표현식                     | 결과 예시                              |
|--------|----------------------------------------|----------------------------------------|
| Java   | `Integer.toBinaryString(n)`            | `"10000000000000000000000000000000"`   |
| Rust   | `format!("{:b}", n)`                   | `"10000000000000000000000000000000"`   |
| C++    | `std::bitset<32>(n).to_string()`       | `"10000000000000000000000000000000"`   |

---


