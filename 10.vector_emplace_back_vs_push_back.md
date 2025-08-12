
# `std::vector::emplace_back` vs `std::vector::push_back`

## 📌 Core Principle
- **Rvalue에 대해서만 `emplace_back`을 사용**하는 것이 권장된다.
- 다른 경우에는 `move` 연산자를 올바르게 적용해야 한다.

---

## 1. `push_back` 특징
`push_back` 함수는 **객체를 삽입하는 방식**으로, 객체 없이 삽입하려면 **임시 객체(rvalue)** 가 필요하다.
암시적 형변환이 가능하면 인자를 통해 임시 객체를 생성해 삽입할 수도 있다.

```cpp
std::vector<Item> vt;
Item a = {}; // 기본 생성자 호출

vt.push_back(Item("abc", 1, 234)); 
vt.push_back(std::move(a)); 
```

**작동 과정**
1. `Item("abc", 1, 234)`를 통해 **임시 객체** 생성
2. `push_back` 내부에서 **복사 생성자 또는 이동 생성자** 호출하여 임시 객체를 새로 생성
3. `vector` 끝에 삽입
4. 함수 종료 시 원래의 임시 객체 소멸

---

## 2. `emplace_back` 특징
- C++11에서 도입된 함수
- **가변 인자 템플릿**을 사용하여 **필요한 인자만 전달**받고, 함수 내부에서 객체를 직접 생성해 삽입
- 불필요한 임시 객체 생성이 없음
- `move` 생성자가 있다면 **최소 생성/이동 횟수**로 삽입 가능

```cpp
std::vector<Item> vt;
vt.emplace_back("abc", 1, 234);
```

**작동 과정**
1. `("abc", 1, 234)` 인자를 직접 전달
2. `emplace_back` 내부에서 **객체 직접 생성**
3. `vector`에 바로 삽입
4. main 종료 시 `vector` 안의 객체 소멸

---

## 3. 동작 비교 예제

```cpp
#include <iostream>
#include <vector>
using namespace std;

class Item {
public:
    Item(const int _n) : m_nx(_n) { cout << "일반 생성자 호출" << endl; }
    Item(const Item& rhs) : m_nx(rhs.m_nx) { cout << "복사 생성자 호출" << endl; }
    Item(Item&& rhs) : m_nx(std::move(rhs.m_nx)) { cout << "이동 생성자 호출" << endl; }
    ~Item() { cout << "소멸자 호출" << endl; }
private:
    int m_nx;
};

int main() {
    std::vector<Item> v;

    cout << "push_back 호출" << endl;
    v.push_back(Item(3));

    cout << "emplace_back 호출" << endl;
    v.emplace_back(3);
}
```

### push_back 결과
1. `Item(3)` → 임시 객체 생성
2. push_back 내부에서 이동 생성자 호출 → 새로운 객체 생성
3. vector에 삽입
4. 기존 임시 객체 소멸
5. main 종료 시 vector 내부 객체 소멸

### emplace_back 결과
1. `(3)` 인자 전달
2. 내부에서 객체 직접 생성
3. vector에 삽입
4. main 종료 시 vector 내부 객체 소멸

---

## 4. move와 emplace_back 주의점
`std::move`는 **소유권 이전**을 수행하므로, move 이후 원래 객체의 상태는 비워짐(유효하지만 값 없음).

```cpp
#include <iostream>
#include <string>
#include <vector>

void som_test(std::vector<std::string>& vecArgs){
    std::vector<std::string> in = std::move(vecArgs);
    for(auto& val : in){
        std::cout << val << std::endl;
    }
}

int main() {
    std::vector<std::string> vecArgs;
    vecArgs.emplace_back("Model1");
    vecArgs.emplace_back("Model2");

    som_test(vecArgs);

    std::cout << vecArgs.size() << std::endl; // 0

    for(auto& val : vecArgs){
        std::cout << "Outer Value " << val << std::endl;
    }
}
```

**실행 결과**
```
Model1
Model2
0
(출력 없음)
```

> `vecArgs`가 `move` 되어 내부 데이터가 다른 vector로 이전됨.

---

## ✅ 정리
| 함수 | 특징 | 장점 | 단점 |
|------|------|------|------|
| push_back | 완성된 객체를 삽입 | 단순 명료 | 불필요한 임시 객체 생성 가능 |
| emplace_back | 인자를 받아 내부에서 객체 생성 | 임시 객체 생략, 성능 최적화 | 잘못 사용 시 복잡성 증가 |

💡 **규칙**  
- Rvalue 삽입 → `emplace_back` 사용  
- 이미 생성된 객체 삽입 → `push_back(std::move(obj))` 사용  
- 불필요한 복사/이동 방지 목적 → `emplace_back` 선호
