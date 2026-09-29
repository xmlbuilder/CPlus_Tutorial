# 🧰 `ON_Closure<R(Args...)>` — 타입 소거 호출자 (함수 시그니처 문법 지원판)

이 버전은 클래스를 **`ON_Closure<R(Args...)>`** 형태로 **부분 특수화**하여,
`std::function`처럼 **함수 시그니처 문법**으로 선언/사용할 수 있게 수정했습니다.

- 예: `ON_Closure<int(int,int)> add = [](int a,int b){ return a+b; };`
- 내부는 **타입 소거(Type Erasure)**: 추상 인터페이스 + 모델 + `std::shared_ptr`
- **`std::decay_t`**, **`std::invoke`**, **perfect forwarding**, **`R=void` 지원**, **빈 상태 체크** 포함

---

## ✨ 변경/개선 요약

1. **함수 시그니처 문법** 지원  
   `ON_Closure<int(int,int)>` 처럼 직관적으로 선언 가능.

2. **`R=void` 정상 지원**  
   `if constexpr (std::is_void_v<R>)` 분기로 `void` 반환도 처리.

3. **완전 전달(Perfect Forwarding)**  
   `Concept::invoke(Args&&...)` + `std::forward` 로 값/참조/rvalue 모두 정확 전달.

4. **`std::invoke` 채택**  
   일반 함수/함수 객체/멤버 함수 포인터/`reference_wrapper`까지 호출 커버.

5. **저장 타입 정규화: `std::decay_t<F>`**  
   참조/cv/배열/함수 타입을 보관하기 좋은 **값 타입**으로 변환(수명 안전).

6. **유효성 체크**  
   `explicit operator bool()` 로 비어있는 상태인지 확인 가능.

> 복사 시에는 내부 `shared_ptr` 공유 의미입니다(값의 deep copy가 아님).

---

## 📦 싱글 헤더

```cpp
#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


// ============================================================
// Closure
//
// Stores any callable object (function, lambda, functor, etc.)
// behind a unified type‑erased interface of the form R(Args...).
//
// Similar to std::function, but the underlying callable is
// managed via shared_ptr, so copying a Closure shares the
// callable instead of duplicating it.
//
// Example:
//
//   Closure<double(double)> func =
//       [](double x)
//       {
//           return x * 2.0;
//       };
//
//   double result = func(10.0);
//
// ============================================================



// ------------------------------------------------------------
// Primary template
//
// Sig must be of the form R(Args...).
// ------------------------------------------------------------


template <class Sig>
class Closure;


// ============================================================
// Partial specialization
//
// Supports function signatures of the form:
//
//     Closure<R(Args...)>
//
// ============================================================
template <class R, class... Args>
class Closure<R(Args...)>
{
public:

    // ========================================================
    // Default Constructor
    //
    // Creates an empty Closure with no callable target.
    // ========================================================
    Closure() noexcept = default;


    // ========================================================
    // Callable Constructor
    //
    // Stores various callable types inside the Closure:
    //
    //   - lambda
    //   - function pointer
    //   - functor
    //   - captured lambda
    //
    // The condition DF != Closure prevents this constructor
    // from accepting another Closure instance directly.
    //
    // Copying a Closure is handled by the dedicated copy
    // constructor below.
    // ========================================================
    template <
        class F,
        class DF = std::decay_t<F>,
        std::enable_if_t<
            !std::is_same_v<DF, Closure>,
            int> = 0>
    Closure(F&& f)
        : func_(
            std::make_shared<Model<DF>>(
                std::forward<F>(f)
            )
        )
    {
    }


    // ========================================================
    // Copy Constructor
    //
    // Does not copy the underlying callable.
    //
    // Copies only the shared_ptr, so both Closures
    // share the same Model instance.
    // ========================================================
    Closure(const Closure&) noexcept = default;


    // ========================================================
    // Move Constructor
    // ========================================================

    Closure(Closure&&) noexcept = default;


    // ========================================================
    // Copy Assignment
    // ========================================================

    Closure& operator=(const Closure&) noexcept = default;


    // ========================================================
    // Move Assignment
    // ========================================================

    Closure& operator=(Closure&&) noexcept = default;


    // ========================================================
    // Destructor
    // ========================================================

    ~Closure() = default;


    // ========================================================
    // operator()
    //
    // Invokes the stored callable.
    //
    // Calling an empty Closure throws std::bad_function_call,
    // similar to std::function.
    //
    // Supports callables returning void as well.
    // ========================================================
    R operator()(Args... args) const
    {
        if (!func_)
        {
            throw std::bad_function_call();
        }

        if constexpr (std::is_void_v<R>)
        {
            func_->invoke(
                std::forward<Args>(args)...
            );
        }
        else
        {
            return func_->invoke(
                std::forward<Args>(args)...
            );
        }
        return {};
    }


    // ========================================================
    // operator bool
    //
    // Checks whether a callable object is stored.
    //
    // Example:
    //
    //   if (func)
    //       func(...);
    //
    // ========================================================
    explicit operator bool() const noexcept
    {
        return static_cast<bool>(func_);
    }


    // ========================================================
    // empty
    // ========================================================

    bool empty() const noexcept
    {
        return !func_;
    }

    // ========================================================
    // reset
    //
    // Releases the reference to the current callable.
    //
    // If other Closures still share the same callable,
    // the underlying Model object is not destroyed.
    // ========================================================
    void reset() noexcept
    {
        func_.reset();
    }


private:

    // ========================================================
    // Concept
    //
    // Type‑erasure base class that provides a unified interface
    // for invoking any callable object.
    // ========================================================
    struct Concept
    {
        virtual ~Concept() = default;

        virtual R invoke(Args&&... args) const = 0;
    };


    // ========================================================
    // Model
    //
    // Stores the actual callable object.
    //
    // F may be a lambda, functor, or function pointer.
    // ========================================================
    template <class F>
    struct Model final : Concept
    {
        F f;


        // ----------------------------------------------------
        // Model Constructor
        //
        // Uses a forwarding constructor to store the callable,
        // copying or moving it as appropriate.
        // ----------------------------------------------------
        template <class Fn>
        explicit Model(Fn&& fn)
            : f(std::forward<Fn>(fn))
        {
        }


        // ----------------------------------------------------
        // invoke
        //
        // Invokes the stored callable.
        //
        // Because it uses std::invoke, all callable types are
        // handled uniformly — regular lambdas, functors,
        // function pointers, and more.
        // ----------------------------------------------------
        R invoke(Args&&... args) const override
        {
            if constexpr (std::is_void_v<R>)
            {
                std::invoke(
                    f,
                    std::forward<Args>(args)...
                );
            }
            else
            {
                return std::invoke(
                    f,
                    std::forward<Args>(args)...
                );
            }
            return {};
        }
    };


private:

    // ========================================================
    // Callable Storage
    //
    // The actual callable (Model) is shared via shared_ptr.
    //
    // Closure A ─┐
    //             ├──> Model<F> ──> callable
    // Closure B ─┤
    // Closure C ─┘
    //
    // Therefore, copying a Closure does not duplicate the
    // Model<F> or the callable object; it simply shares them.
    // ========================================================
    std::shared_ptr<const Concept> func_;
};
```

---

## 🧪 사용 예제

```cpp
#include <iostream>
#include <memory>
#include <string>

int main() {
  // 1) 일반 람다
  ON_Closure<int(int,int)> add = [](int a, int b){ return a + b; };
  std::cout << add(3, 4) << "\n";  // 7

  // 2) void 반환
  ON_Closure<void(const std::string&)> print = [](const std::string& s){
    std::cout << s << "\n";
  };
  print("hello");

  // 3) 멤버 함수 포인터 (std::invoke로 지원)
  struct Greeter { void hello(const std::string& who){ std::cout << "hi " << who << "\n"; } };
  Greeter g;
  ON_Closure<void(Greeter&, const std::string&)> call = &Greeter::hello;
  call(g, "world");

  // 4) move-only 캡처
  auto p = std::make_unique<int>(42);
  ON_Closure<int(int)> plusP{ [q = std::move(p)](int x){ return x + *q; } };
  std::cout << plusP(8) << "\n";  // 50

  // 5) 빈 상태 체크
  ON_Closure<int(int,int)> op;     // empty
  if (!op) { /* 아직 대상 미설정 */ }
  op = ON_Closure<int(int,int)>{ [](int a,int b){ return a*b; } };
  std::cout << op(3, 4) << "\n";   // 12
}
```

### 빌드
```bash
g++ -std=c++17 -O2 demo.cpp -o demo
# or
clang++ -std=c++17 -O2 demo.cpp -o demo
```

---

## 🔁 마이그레이션 가이드 (이전 `<R, Args...>` 문법에서)

이전:  
```cpp
ON_Closure<int, int, int> add = [](int a, int b){ return a + b; };
```

변경 후(권장):  
```cpp
ON_Closure<int(int,int)> add = [](int a, int b){ return a + b; };
```

`void` 반환/참조 매개변수 등도 **함수 시그니처**로 표현합니다.
```cpp
ON_Closure<void(const std::string&)> print = ...;
ON_Closure<void(Greeter&, const std::string&)> call = ...;
```

---

## 🧠 왜 `std::decay_t`가 중요한가? (요약)
- 참조/const 제거 + 함수→함수포인터, 배열→포인터 변환 → **보관하기 좋은 값 타입**으로 정규화
- lvalue를 넘겨도 내부에 **참조 멤버가 남지 않음**(수명 안전)
- 자세한 설명은 README의 부록/이전 답변 참고

---

## ⚠️ 주의/한계

- `operator()`가 `const` → 비-`const` functor는 `mutable` 캡처/멤버로 설계하거나 필요시 비-`const` 오버로드를 추가하세요.
- 복사하면 내부 대상은 **공유**됩니다(`shared_ptr`). Deep copy 의미가 필요하면 `clone()`을 도입한 설계를 고려.
- 이 구현은 **SBO(소형 버퍼 최적화)** 를 제공하지 않습니다(힙 할당 1회). 소형 객체 성능이 중요하면 SBO 확장을 고려하세요.

---

