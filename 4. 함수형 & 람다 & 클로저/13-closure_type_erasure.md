## 📘 `Closure` — Lightweight type-erased callable

- `Closure<R, Args...>`는 임의의 **호출 가능 객체**(람다, 함수 포인터, 펑터, `std::bind` 결과 등)를 **타입 소거(type erasure)** 로 감싸
- **하나의 공통 인터페이스**로 호출할 수 있게 하는 작고 간단한 유틸입니다.
- 어떤 호출 대상을 넘겨도 내부에서 **추상 인터페이스(Concept)** 로 감싸고
- 실제 타입은 **템플릿 모델(Model<F>)** 이 보관하며
- 외부에서는 `operator()` 로 **함수처럼 호출** 합니다.



### 📌 특징

- **간결한 타입 소거**: 추상 베이스 + 모델(템플릿) 구조
- **호출은 함수처럼**: `f(args...)` 형태
- **복사 저렴**: 내부는 `std::shared_ptr`로 공유(깊은 복사 아님)
- **move-only 캡처 람다** 도 저장 가능(복사하면 *공유*됨)

> 이 버전은 템플릿 인자를 **`<R, Args...>`** 형태로 받습니다.  


## 📦 단일 헤더 (원본 구현)


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

### 📌 아키텍처 한눈에

```
Closure<R, Args...>
└─ shared_ptr<Concept>
   ├─ virtual R invoke(Args...)
   └─ Model<F> : Concept
      └─ F f  // 실제 람다/함수객체/펑터
```

- `Concept`: 공통 호출 인터페이스
- `Model<F>`: 구체 타입 `F`를 보관하고 실제 호출을 수행

---

### 📌 사용법

> **중요:** 이 버전은 **`<R, Args...>`** 템플릿 인자 방식을 사용합니다.

#### 🔹 1) 기본 예제

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

#### 🔹 2) 빌드

```bash
g++ -std=c++17 -O2 main.cpp -o demo
# 또는
clang++ -std=c++17 -O2 main.cpp -o demo
```

---


