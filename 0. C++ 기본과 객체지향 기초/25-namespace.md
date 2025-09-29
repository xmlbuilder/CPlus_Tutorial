# 🧠 namespace란?
- C++에서 **식별자(변수, 함수, 클래스 등)**를 그룹화하는 방법
- 같은 이름의 식별자가 여러 곳에서 정의되어도 충돌 없이 구분 가능
- 표준 라이브러리도 std라는 namespace 안에 정의되어 있음

## 📦 기본 사용법
```cpp
namespace MyMath {
    int add(int a, int b) {
        return a + b;
    }
}
```

### 사용할 때는:
```cpp
int result = MyMath::add(3, 4);
```


## 🧪 예제: 이름 충돌 방지
```cpp
namespace A {
    void print() { std::cout << "A::print()\n"; }
}

namespace B {
    void print() { std::cout << "B::print()\n"; }
}

int main() {
    A::print(); // A의 print 호출
    B::print(); // B의 print 호출
}
```


## 🔧 using 선언
### 1. 특정 식별자만 가져오기
```cpp
using A::print;
print(); // A::print() 호출
```

### 2. 전체 namespace 가져오기 (주의 필요)
```cpp
using namespace std;
cout << "Hello\n"; // std::cout 생략 가능
```

⚠️ using namespace std;는 작은 예제에서는 편하지만, 헤더 파일에서는 절대 사용하지 않는 게 좋습니다.  
이름 충돌 위험이 커짐.


## 🧬 중첩 namespace
```cpp
namespace Engine {
    namespace Physics {
        void simulate() { std::cout << "Simulating physics...\n"; }
    }
}

// C++17부터는 이렇게도 가능
namespace Engine::Graphics {
    void render() { std::cout << "Rendering graphics...\n"; }
}
```


## 🧵 익명 namespace
- 파일 내부에서만 접근 가능한 internal linkage를 만들고 싶을 때 사용
```cpp
namespace {
    void helper() { std::cout << "Internal helper\n"; }
}
```

익명 namespace 안의 함수는 외부에서 접근 불가 → static 함수보다 더 안전한 방식


✨ 요약
| 종류 | 설명 |
|------|-----| 
| 기본 namespace | 이름 충돌 방지, 코드 그룹화 | 
| using 선언 | 이름 생략 가능, 단 헤더에서는 주의 | 
| 중첩 namespace | 계층적 구조 표현 | 
| 익명 namespace | 파일 내부 전용 함수/변수 정의 | 


---




