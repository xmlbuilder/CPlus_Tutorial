# 🧰 `ON_Closure` — Lightweight type-erased callable (GitHub README)

`ON_Closure<R, Args...>`는 임의의 **호출 가능 객체**(람다, 함수 포인터, 펑터, `std::bind` 결과 등)를 **타입 소거(type erasure)** 로 감싸 **하나의 공통 인터페이스**로 호출할 수 있게 하는 작고 간단한 유틸입니다.

- 어떤 호출 대상을 넘겨도 내부에서 **추상 인터페이스(Concept)** 로 감싸고
- 실제 타입은 **템플릿 모델(Model<F>)** 이 보관하며
- 외부에서는 `operator()` 로 **함수처럼 호출**합니다.

---

## ✨ 특징

- **간결한 타입 소거**: 추상 베이스 + 모델(템플릿) 구조
- **호출은 함수처럼**: `f(args...)` 형태
- **복사 저렴**: 내부는 `std::shared_ptr`로 공유(깊은 복사 아님)
- **move-only 캡처 람다**도 저장 가능(복사하면 *공유*됨)

> 이 버전은 템플릿 인자를 **`<R, Args...>`** 형태로 받습니다.  
> (즉, `ON_Closure<int, int, int>` 처럼 사용합니다. `ON_Closure<int(int,int)>` 문법은 **미지원**)

---

## 📦 단일 헤더 (원본 구현)

> 아래 코드는 질문에서 주신 구현을 가독성 있게 정리한 것입니다. 동작은 동일합니다.

```cpp
#pragma once
#include <memory>
#include <utility>

template<typename R, typename... Args>
class ON_Closure {
public:
    // 생성자: 어떤 함수 객체든 받아서 저장
    template<typename F>
    ON_Closure(F&& f)
        : func(std::make_shared<Model<F>>(std::forward<F>(f))) {}

    // 호출 연산자: 함수처럼 호출
    R operator()(Args... args) const {
        return func->invoke(std::forward<Args>(args)...);
    }

private:
    // 타입 소거를 위한 추상 인터페이스
    struct Concept {
        virtual ~Concept() = default;
        virtual R invoke(Args... args) const = 0;
    };

    // 실제 함수 객체를 담는 템플릿 모델
    template<typename F>
    struct Model : Concept {
        F f;
        explicit Model(F&& f) : f(std::forward<F>(f)) {}
        R invoke(Args... args) const override {
            return f(std::forward<Args>(args)...);
        }
    };

    std::shared_ptr<const Concept> func;
};
```

---

## 🧠 아키텍처 한눈에

```
ON_Closure<R, Args...>
└─ shared_ptr<Concept>
   ├─ virtual R invoke(Args...)
   └─ Model<F> : Concept
      └─ F f  // 실제 람다/함수객체/펑터
```

- `Concept`: 공통 호출 인터페이스
- `Model<F>`: 구체 타입 `F`를 보관하고 실제 호출을 수행

---

## 🛠 사용법

> **중요:** 이 버전은 **`<R, Args...>`** 템플릿 인자 방식을 사용합니다.

### 1) 기본 예제

```cpp
#include <iostream>
#include <string>
#include "on_closure.hpp"

int main() {
    // 두 정수 합
    ON_Closure<int, int, int> add = [](int a, int b) { return a + b; };
    std::cout << add(3, 4) << "\n"; // 7

    // 문자열 길이 반환
    ON_Closure<std::size_t, const std::string&> len =
        [](const std::string& s){ return s.size(); };
    std::cout << len(std::string("hello")) << "\n"; // 5

    // move-only 캡처
    auto p = std::make_unique<int>(42);
    ON_Closure<int, int> plusP{ [q = std::move(p)](int x){ return x + *q; } };
    std::cout << plusP(8) << "\n"; // 50

    // 멤버 함수 호출은 람다로 감싸서
    struct Greeter { int n=0; int hello(const std::string& who){ return ++n, (std::cout<<"hi "<<who<<"\n", n); } };
    Greeter g;
    ON_Closure<int, const std::string&> call = [&g](const std::string& w){ return g.hello(w); };
    std::cout << call("world") << "\n"; // 1
}
```

### 2) 빌드

```bash
g++ -std=c++17 -O2 main.cpp -o demo
# 또는
clang++ -std=c++17 -O2 main.cpp -o demo
```

---

## ⚠️ 제한 사항 & 주의점

이 원본 구현은 아주 간단한 대신, 다음과 같은 제약이 있습니다.

1. **`R = void` 미지원**
   - `operator()`가 `return func->invoke(...);` 형태라 `R=void`일 때 컴파일 에러가 납니다.
   - 필요하면 `if constexpr (std::is_void_v<R>)` 분기 추가가 필요합니다.

2. **완전 전달(Perfect Forwarding) 아님**
   - `operator()(Args... args)` 와 `Concept::invoke(Args... args)`가 **값 전달**이므로
     전달 인자의 참조성/값성/우측값 정보가 보존되지 않습니다.  
     (실전에서는 `Args&&...` + `std::forward<Args>(args)...` 권장)

3. **`const` 호출만 가능**
   - `invoke`와 `operator()`가 `const`이므로 **비-const 호출 연산자만 가진 functor**는 사용할 수 없습니다.
     (필요하면 해당 `const`를 제거/오버로드하거나 functor 측을 `mutable`로)

4. **CPO/std::invoke 미사용**
   - 멤버 함수 포인터, `std::reference_wrapper` 등의 폭넓은 호출은 직접 람다로 감싸야 합니다.
     (실전에서는 `std::invoke` 채택 권장)

5. **SBO(소형 버퍼 최적화) 없음**
   - 대상 보관 시 1회 힙 할당(`make_shared`)이 발생합니다.  
     소형 객체 성능이 중요하면 SBO가 있는 구현을 고려하세요.

6. **복사 의미는 “공유”**
   - `std::shared_ptr`를 복사하므로 **동일 대상**을 공유합니다.
   - 완전한 독립 복사(딥카피)가 필요하면 `clone()` 인터페이스를 도입하는 쪽을 고려하세요.

---

## 💡 개선 포인트(선택)

실전에서 쓰려면 아래와 같은 변경을 권장합니다.

- `R = void` 지원: `if constexpr (std::is_void_v<R>)` 분기
- **완전 전달**: `invoke(Args&&...)` + `std::forward`
- **`std::decay_t<F>`** 저장: 참조/cv/배열/함수 타입을 보관하기 좋은 값 타입으로 정규화
- **`std::invoke`** 사용: 함수/멤버 함수 포인터/참조 래퍼까지 호출 커버
- (선택) 비어 있음 체크: `explicit operator bool() const`
- (선택) SBO/clone/noexcept 전파 등 고급화

---

