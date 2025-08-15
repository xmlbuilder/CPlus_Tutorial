# 📘 함수 포인터 사용 방식 비교: 고전 vs 현대 C++
## 🔹 개요
C++에서는 함수를 변수처럼 다룰 수 있도록 함수 포인터를 제공합니다. 고전적인 방식은 C 스타일의 함수 포인터를 사용하고, 현대적인 방식은 std::function과 람다를 활용합니다.

### 🧪 Case 1: typedef를 이용한 고전적 함수 포인터
```cpp
typedef int (*funCall)();

int func1() {
    std::cout << "Func1 called" << std::endl;
    return 0;
}

int main() {
    funCall sample = func1;
    sample();
}
```

### ✅ 특징
- typedef로 함수 포인터 타입을 정의
- 가독성 향상
### ⚠️ 단점
- 타입이 고정되어 있어 유연성이 떨어짐
- 람다나 캡처 기능을 지원하지 않음

## 🧪 Case 2: 직접 함수 포인터 선언
```cpp
int (*funCall)() = func1;
funCall();
```

### ✅ 특징
- 가장 기본적인 C 스타일 함수 포인터
- 직관적이지만 복잡한 함수 시그니처에서는 가독성이 떨어짐
### ⚠️ 단점
- 재사용성이 낮고, 타입 정의가 반복됨

## 🧪 Case 3: using을 활용한 함수 포인터 타입 정의
```cpp
using NewProcess = int (*)(void);

void func2(NewProcess func) {
    func();
}
```

### ✅ 특징
- using을 통해 타입 정의가 더 현대적이고 깔끔함
- 함수 인자로 전달 가능
### ⚠️ 단점
- 여전히 C 스타일 함수 포인터의 한계를 가짐

## 🧪 Case 4: std::function을 활용한 현대적 방식
```cpp
typedef std::function<int()> funType;

int func2(std::function<int()> & fun) {
    fun();
    return 0;
}
```

### ✅ 특징
- 람다, 일반 함수, 멤버 함수, 바인딩된 함수 모두 지원
- 타입 추론과 캡처 기능 가능
- 안전하고 유연한 함수 호출 방식
### ⚠️ 단점
- 약간의 성능 오버헤드 (함수 포인터보다 느릴 수 있음)
- 템플릿 기반이므로 디버깅이 복잡할 수 있음

## 📊 비교 요약
| 케이스 | 방식 | 유연성 | 가독성 | 람다 지원 | 함수 인자 전달 | 성능 | 
| Case 1 | typedef 함수 포인터 | 낮음 | 보통 | ❌ | 가능 | 빠름 | 
| Case 2 | 직접 선언 | 낮음 | 낮음 | ❌ | 가능 | 빠름 | 
| Case 3 | using 타입 정의 | 보통 | 좋음 | ❌ | 가능 | 빠름 | 
| Case 4 | std::function | 높음 | 좋음 | ✅ | 가능 | 느림 | 



## 🧠 결론
- 간단한 함수 호출에는 고전적인 함수 포인터도 충분히 유용합니다.
- 복잡한 함수 전달, 람다, 바인딩이 필요한 경우에는 std::function이 훨씬 더 강력하고 안전한 선택입니다.
- 최신 C++에서는 std::function과 auto, 람다를 함께 사용하는 방식이 가장 일반적입니다.

