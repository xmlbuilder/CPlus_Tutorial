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
    //            ├──> Model<F> ──> callable
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
    Closure<int, int, int> add = [](int a, int b) { return a + b; };
    std::cout << add(3, 4) << "\n"; // 7

    // 문자열 길이 반환
    Closure<std::size_t, const std::string&> len =
        [](const std::string& s){ return s.size(); };
    std::cout << len(std::string("hello")) << "\n"; // 5

    // move-only 캡처
    auto p = std::make_unique<int>(42);
    Closure<int, int> plusP{ [q = std::move(p)](int x){ return x + *q; } };
    std::cout << plusP(8) << "\n"; // 50

    // 멤버 함수 호출은 람다로 감싸서
    struct Greeter { int n=0; int hello(const std::string& who){ return ++n, (std::cout<<"hi "<<who<<"\n", n); } };
    Greeter g;
    Closure<int, const std::string&> call = [&g](const std::string& w){ return g.hello(w); };
    std::cout << call("world") << "\n"; // 1
}
```
```cpp
#include "closure.hpp"

#include "closure_tests.h"

#include <iostream>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <ranges>

static int g_pass = 0;
static int g_fail = 0;

static void Check(bool condition, const char* message)
{
    if (condition)
    {
        ++g_pass;
        std::cout << "[PASS] " << message << '\n';
    }
    else
    {
        ++g_fail;
        std::cout << "[FAIL] " << message << '\n';
    }
}

// ============================================================
// Normal function
// ============================================================
static double Square(double x)
{
    return x * x;
}

// ============================================================
// Functor
// ============================================================
struct Multiply
{
    double scale;

    double operator()(double x) const
    {
        return x * scale;
    }
};

// ============================================================
// Test 1
// Empty Closure
// ============================================================
static void TestEmptyClosure()
{
    std::cout
        << "\n========================================\n"
        << "Test 1 - Empty Closure\n"
        << "========================================\n";

    Closure<double(double)> func;

    Check(!func, "default Closure is invalid");
    Check(func.empty(), "empty() returns true");

    bool exceptionThrown = false;

    try
    {
        func(10.0);
    }
    catch (const std::bad_function_call&)
    {
        exceptionThrown = true;
    }

    Check(
        exceptionThrown,
        "empty Closure throws bad_function_call"
    );
}


// ============================================================
// Test 2
// Lambda
// ============================================================
static void TestLambda()
{
    std::cout
        << "\n========================================\n"
        << "Test 2 - Lambda\n"
        << "========================================\n";

    Closure<double(double)> func =
        [](double x)
        {
            return x * 2.0;
        };

    Check(!func.empty(), "lambda Closure is valid");

    Check(
        std::abs(func(10.0) - 20.0) < 1e-12,
        "lambda invocation works"
    );
}


// ============================================================
// Test 3
// Captured Lambda
// ============================================================
static void TestCapturedLambda()
{
    std::cout
        << "\n========================================\n"
        << "Test 3 - Captured Lambda\n"
        << "========================================\n";

    double scale = 3.5;

    Closure<double(double)> func =
        [scale](double x)
        {
            return x * scale;
        };

    Check(
        std::abs(func(2.0) - 7.0) < 1e-12,
        "captured value preserved"
    );
}


// ============================================================
// Test 4
// Function pointer
// ============================================================
static void TestFunctionPointer()
{
    std::cout
        << "\n========================================\n"
        << "Test 4 - Function Pointer\n"
        << "========================================\n";

    Closure<double(double)> func = &Square;

    Check(
        std::abs(func(5.0) - 25.0) < 1e-12,
        "normal function invocation works"
    );
}


// ============================================================
// Test 5
// Functor
// ============================================================
static void TestFunctor()
{
    std::cout
        << "\n========================================\n"
        << "Test 5 - Functor\n"
        << "========================================\n";

    Multiply multiply{4.0};

    Closure<double(double)> func = multiply;

    Check(
        std::abs(func(3.0) - 12.0) < 1e-12,
        "functor invocation works"
    );
}

// ============================================================
// Test 6
// Multiple arguments
// ============================================================
static void TestMultipleArguments()
{
    std::cout
        << "\n========================================\n"
        << "Test 6 - Multiple Arguments\n"
        << "========================================\n";

    Closure<double(double, double, double)> func =
        [](double x, double y, double z)
        {
            return x + y + z;
        };

    Check(
        std::abs(func(1.0, 2.0, 3.0) - 6.0) < 1e-12,
        "multiple arguments work"
    );
}


// ============================================================
// Test 7
// std::string
// ============================================================
static void TestString()
{
    std::cout
        << "\n========================================\n"
        << "Test 7 - std::string\n"
        << "========================================\n";

    Closure<std::string(const std::string&)> func =
        [](const std::string& s)
        {
            return "[CAD] " + s;
        };

    Check(
        func("NURBS Surface") ==
            "[CAD] NURBS Surface",
        "std::string argument/result works"
    );
}

// ============================================================
// Test 8
// void return
// ============================================================
static void TestVoidReturn()
{
    std::cout
        << "\n========================================\n"
        << "Test 8 - void Return\n"
        << "========================================\n";

    int value = 0;

    Closure<void(int)> func =
        [&value](int x)
        {
            value += x;
        };

    func(10);
    func(20);

    Check(
        value == 30,
        "void Closure invocation works"
    );
}


// ============================================================
// Test 9
// Copy
// ============================================================
static void TestCopy()
{
    std::cout
        << "\n========================================\n"
        << "Test 9 - Copy\n"
        << "========================================\n";

    Closure<double(double)> func1 =
        [](double x)
        {
            return x * 10.0;
        };

    Closure<double(double)> func2 = func1;

    Check(!func1.empty(), "original Closure valid");
    Check(!func2.empty(), "copied Closure valid");

    Check(
        std::abs(func1(3.0) - 30.0) < 1e-12,
        "original Closure works"
    );

    Check(
        std::abs(func2(3.0) - 30.0) < 1e-12,
        "copied Closure works"
    );
}


// ============================================================
// Test 10
// Shared captured state
//
// Closure 복사 시 Model이 공유되는 것을 검증한다.
// ============================================================
static void TestSharedState()
{
    std::cout
        << "\n========================================\n"
        << "Test 10 - Shared Callable\n"
        << "========================================\n";

    auto counter =
        std::make_shared<int>(0);

    Closure<void()> func1 =
        [counter]()
        {
            ++(*counter);
        };

    Closure<void()> func2 = func1;
    Closure<void()> func3 = func2;

    func1();
    func2();
    func3();

    Check(
        *counter == 3,
        "copied Closures share captured state"
    );
}


// ============================================================
// Test 11
// Reset
// ============================================================
static void TestReset()
{
    std::cout
        << "\n========================================\n"
        << "Test 11 - Reset\n"
        << "========================================\n";

    Closure<int(int)> func =
        [](int x)
        {
            return x + 1;
        };

    Check(!func.empty(), "Closure valid before reset");

    func.reset();

    Check(!func, "Closure invalid after reset");
    Check(func.empty(), "empty after reset");
}

// ============================================================
// Test 12
// Move
// ============================================================
static void TestMove()
{
    std::cout
        << "\n========================================\n"
        << "Test 12 - Move\n"
        << "========================================\n";

    Closure<int(int)> func1 =
        [](int x)
        {
            return x * 2;
        };

    Closure<int(int)> func2 =
        std::move(func1);

    Check(!func1, "source empty after move");
    Check(!func2.empty(), "destination valid after move");

    Check(
        func2(10) == 20,
        "moved Closure works"
    );
}


// ============================================================
// Test 13
// CAD style - curve evaluator
// ============================================================
static void TestCadEvaluator()
{
    std::cout
        << "\n========================================\n"
        << "Test 13 - CAD Style Evaluator\n"
        << "========================================\n";

    //
    // 실제 CAD에서는:
    //
    // [&curve](double t)
    // {
    //     return curve.PointAt(t);
    // }
    //
    // 같은 형태로 사용할 수 있다.
    //
    Closure<double(double)> curveEvaluator =
        [](double t)
        {
            // 테스트용 curve:
            // y = t^2
            return t * t;
        };

    Check(
        std::abs(curveEvaluator(0.5) - 0.25)
            < 1e-12,
        "CAD style curve evaluator works"
    );
}


// ============================================================
// Test 14
// CAD style predicate
// ============================================================
static void TestCadPredicate()
{
    std::cout
        << "\n========================================\n"
        << "Test 14 - CAD Style Predicate\n"
        << "========================================\n";

    const double tolerance = 0.01;

    Closure<bool(double)> needRefine =
        [tolerance](double error)
        {
            return error > tolerance;
        };

    Check(
        !needRefine(0.001),
        "small error does not refine"
    );

    Check(
        needRefine(0.1),
        "large error requests refinement"
    );
}

int closure_tests::run_tests()
{
    // 1) 일반 람다
    Closure<int(int,int)> add = [](int a, int b){ return a + b; };
    std::cout << add(3, 4) << "\n";  // 7

    // 2) void 반환
    Closure<void(const std::string&)> print = [](const std::string& s){
        std::cout << s << "\n";
    };
    print("hello");

    // 3) 멤버 함수 포인터 (std::invoke로 지원)
    struct Greeter { void hello(const std::string& who){ std::cout << "hi " << who << "\n"; } };
    Greeter g;
    Closure<void(Greeter&, const std::string&)> call = &Greeter::hello;
    call(g, "world");

    // 4) move-only 캡처
    auto p = std::make_unique<int>(42);
    Closure<int(int)> plusP{ [q = std::move(p)](int x){ return x + *q; } };
    std::cout << plusP(8) << "\n";  // 50

    // 5) 빈 상태 체크
    Closure<int(int,int)> op;     // empty
    if (!op) { /* 아직 대상 미설정 */ }
    op = Closure<int(int,int)>{ [](int a,int b){ return a*b; } };
    std::cout << op(3, 4) << "\n";   // 12

    g_pass = 0;
    g_fail = 0;

    std::cout
        << "========================================\n"
        << "Closure Tests\n"
        << "C++17 Type-Erased Callable\n"
        << "========================================\n";

    TestEmptyClosure();
    TestLambda();
    TestCapturedLambda();
    TestFunctionPointer();
    TestFunctor();
    TestMultipleArguments();
    TestString();
    TestVoidReturn();
    TestCopy();
    TestSharedState();
    TestReset();
    TestMove();

    // CAD usage
    TestCadEvaluator();
    TestCadPredicate();

    std::cout
        << "\n========================================\n"
        << "Closure Test Summary\n"
        << "========================================\n"
        << "PASS   : " << g_pass << '\n'
        << "FAILED : " << g_fail << '\n'
        << "RESULT : "
        << (g_fail == 0 ? "PASS" : "FAILED")
        << '\n'
        << "========================================\n";

    return g_fail == 0 ? 0 : 1;
}
```

#### 🔹 2) 빌드

```bash
g++ -std=c++17 -O2 main.cpp -o demo
# 또는
clang++ -std=c++17 -O2 main.cpp -o demo
```

---


