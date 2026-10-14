## 📘 Intrusive Smart Pointer — `Ptr<T>`와 `ref_counted`


### 📌 1. 목적

- `Ptr<T>`는 **객체 내부에 참조 카운터를 두는 Intrusive Smart Pointer**이다. 
- 같은 CAD Geometry를 여러 곳에서 공유할 때, 마지막 참조가 사라지는 시점에 객체를 자동 삭제한다.
- `ref_counted`: 객체 내부의 참조 카운터 `refs_` 관리
- `Ptr<T>`: 객체 포인터 `p_` 보관, 복사/소멸 시 `acquire()`/`release()` 호출
- `make_ptr<T>()`: `new T(...)`로 생성한 객체를 `Ptr<T>`로 감싼다

```text
Ptr<TestCurve> p1 ───┐
Ptr<TestCurve> p2 ───┼──► TestCurve (heap)
Ptr<TestGeometry> p3 ┘      ├─ Geometry 데이터
                            └─ ref_counted::refs_ = 3
```

- `Ptr<T>` 자체는 일반적으로 포인터 하나 크기이며, `std::shared_ptr`처럼 별도의 control block을 사용하지 않는다.
- 대신 관리 객체가 `ref_counted`를 상속해야 한다.

### 📌 2. 핵심 구현

#### 🔹 `ref_counted` — 객체 내부 카운터

```cpp
class ref_counted {
public:
    ref_counted() noexcept : refs_(0) {}
    ref_counted(const ref_counted&) noexcept : refs_(0) {}
    ref_counted& operator=(const ref_counted&) noexcept { return *this; }
    virtual ~ref_counted() { assert(refs_.load() == 0); }

    void acquire() const noexcept {
        refs_.fetch_add(1, std::memory_order_relaxed);
    }

    void release() const noexcept {
        const int old = refs_.fetch_sub(1, std::memory_order_acq_rel);
        assert(old > 0);
        if (old == 1) delete this;
    }

    int ref_count() const noexcept {
        return refs_.load(std::memory_order_relaxed);
    }

private:
    mutable std::atomic<int> refs_;
};
```

- 생성 직후 `refs_ = 0`이며, `Ptr<T>`가 처음 소유할 때 1이 된다.
- `acquire()`는 1 증가, `release()`는 1 감소한다.
- `release()`의 **이전 값 `old`가 1이면** 감소 후 0이므로 `delete this`를 실행한다.
- `virtual` 소멸자 덕분에 파생 클래스부터 올바르게 소멸한다.


#### 🔹 `Ptr<T>` — 참조 획득과 해제

```cpp
Ptr(T* p) noexcept : p_(p) {
    if (p_) p_->acquire();
}

Ptr(const Ptr& rhs) noexcept : Ptr(rhs.p_) {}

~Ptr() {
    if (p_) p_->release();
}

void reset(T* p = nullptr) noexcept {
    if (p) p->acquire();   // 먼저 새 참조 확보
    T* old = p_;
    p_ = p;
    if (old) old->release();
}
```

- `reset(p_)`처럼 같은 객체로 재설정해도 먼저 증가한 뒤 감소하므로 객체가 중간에 삭제되지 않는다. 
- **이동(move)** 은 포인터를 넘기고 원본을 `nullptr`로 만들어, 참조 카운터를 증가시키지 않는다.

### 📌 3. 생성부터 파괴까지: 카운터 변화

```cpp
void example() {
    Ptr<TestCurve> p1 = make_ptr<TestCurve>(100, stats);
    {
        Ptr<TestCurve> p2 = p1;
        Ptr<TestGeometry> p3 = p1;
    }
    p1 = nullptr;
}
```

| 순서 | 실행 | `refs_` | Heap의 `TestCurve` |
|---:|---|---:|---|
| 1 | `new TestCurve(...)` | 0 | 생성됨 |
| 2 | `p1` 생성 → `acquire()` | 1 | 유지 |
| 3 | `p2 = p1` → `acquire()` | 2 | 유지 |
| 4 | `p3 = p1` → `acquire()` | 3 | 유지 |
| 5 | 내부 블록 종료: `p3` 소멸 → `release()` | 2 | 유지 |
| 6 | `p2` 소멸 → `release()` | 1 | 유지 |
| 7 | `p1 = nullptr` → `release()` | 0 | `delete this` 실행 |
| 8 | 파생→기반 소멸자, 메모리 해제 | — | 삭제 완료 |

```text
[생성]            [복사]               [블록 종료]         [마지막 해제]
 p1                 p1, p2, p3             p1                 (없음)
  │                  │  │  │               │
  ▼                  ▼  ▼  ▼               ▼
 TestCurve          TestCurve            TestCurve          delete this
 refs = 1           refs = 3             refs = 1              │
                                                             ▼
                                                         소멸 + 해제
```

#### 🔹 마지막 참조가 사라질 때의 실제 호출 순서

```text
p1 = nullptr
  └─ Ptr::reset(nullptr)
      └─ old->release()
          ├─ fetch_sub(1): old = 1, refs_ = 0
          └─ delete this
              ├─ TestCurve::~TestCurve()
              ├─ TestGeometry::~TestGeometry()
              ├─ ref_counted::~ref_counted()  // refs_ == 0 확인
              └─ operator delete              // Heap 메모리 반환
```

- **`Ptr`가 소멸했다고 항상 CAD 객체가 삭제되는 것은 아니다.** 마지막 참조를 해제한 `Ptr`에서만 실제 객체 삭제가 발생한다.

### 📌 4. CAD 유사 클래스 사용 예제

- 실제 테스트에서는 `TestGeometry`, `TestCurve`, `TestSurface`, `TestModel`을 사용한다.

```cpp
struct Counters {
    std::atomic<int> created{0};
    std::atomic<int> destroyed{0};
};

class TestGeometry : public ref_counted {
public:
    TestGeometry(int id, Counters& c) : id_(id), counters_(c) {
        ++counters_.created;
    }
    ~TestGeometry() override { ++counters_.destroyed; }
    int id() const { return id_; }
    virtual const char* type() const = 0;
private:
    int id_;
    Counters& counters_;
};

class TestCurve final : public TestGeometry {
public:
    TestCurve(int id, Counters& c) : TestGeometry(id, c) {}
    const char* type() const override { return "Curve"; }
};
```

```cpp
Counters stats;
{
    auto curve = make_ptr<TestCurve>(100, stats); // refs = 1
    Ptr<TestGeometry> geometry = curve;          // refs = 2 (업캐스팅)

    assert(curve->ref_count() == 2);
    assert(geometry->id() == 100);

    geometry = nullptr;                          // refs = 1
    assert(curve->ref_count() == 1);
} // curve 소멸 → refs = 0 → TestCurve 삭제

assert(stats.created == 1);
assert(stats.destroyed == 1);
```

- `Counters`는 참조 객체보다 오래 살아 있어야 한다. 테스트에서는 선언 순서와 스코프를 통해 이를 보장한다.

### 📌 5. 주요 사용 패턴

| 목적 | 코드 | 참조 카운터 효과 |
|---|---|---|
| 생성 | `auto a = make_ptr<TestCurve>(1, stats);` | 0 → 1 |
| 복사 | `auto b = a;` | 1 → 2 |
| 업캐스팅 | `Ptr<TestGeometry> g = a;` | 2 → 3 |
| 이동 | `auto c = std::move(b);` | 변화 없음 (`b`는 null) |
| 참조 해제 | `g = nullptr;` | 3 → 2 |
| 마지막 해제 | `a = nullptr;` 등 | 1 → 0, 삭제 |
| 원시 포인터 조회 | `a.get()` / `a.ptr()` | 변화 없음 |
| 동일 포인터 재설정 | `a.reset(a.get());` | +1 후 -1, 최종 변화 없음 |

`TestModel`은 `std::vector<Ptr<TestGeometry>>`에 포인터를 저장한다. `add()`에서 복사되어 참조 카운터가 증가하고, `clear()`에서 감소한다. 단, 다른 `Ptr`가 남아 있으면 Geometry는 삭제되지 않는다.

### 📌 6. 사용 시 주의사항

1. **스택 객체를 `Ptr`로 관리하지 않는다.** 마지막 `release()`가 `delete this`를 실행하므로 `new`로 할당된 객체를 사용한다.
2. **동일 원시 포인터를 독립적으로 `delete`하지 않는다.** `Ptr`가 소유한 객체의 수명은 `release()`로 관리한다.
3. **순환 참조를 피한다.** A가 B를, B가 A를 `Ptr`로 소유하면 카운터가 0이 되지 않을 수 있다. 현재 구현에는 weak pointer가 없다.
4. **참조 카운터만 원자적이다.** CAD 객체의 좌표·토폴로지 등 데이터 접근까지 스레드 안전해지는 것은 아니다.
5. **같은 `Ptr` 변수 자체에 대한 동시 읽기/쓰기에는 별도 동기화가 필요하다.** 서로 다른 `Ptr` 인스턴스가 동일 객체를 참조하며 증가·감소하는 경우와 구분한다.
6. **객체 수명과 원시 포인터 수명에 주의한다.** `get()`은 소유권을 늘리지 않으므로 마지막 `Ptr`가 해제되면 원시 포인터는 무효화된다.
7. **복사 대입은 참조 카운터를 복사하지 않는다.** `ref_counted` 복사 생성자는 0으로 초기화하고, 대입 연산자는 카운터를 유지한다.

### 📌 7. `std::shared_ptr`와 비교

| 항목 | `Ptr<T>` | `std::shared_ptr<T>` |
|---|---|---|
| 참조 카운터 | 객체 내부 | 별도 control block |
| 포인터 객체 | 보통 원시 포인터 하나 크기 | 보통 포인터 두 개 크기 |
| 대상 객체 요구사항 | `ref_counted` 상속 필요 | 일반 객체 가능 |
| Weak 참조 | 현재 구현 없음 | `std::weak_ptr` 지원 |
| 생성 편의 함수 | `make_ptr<T>()` | `std::make_shared<T>()` |
| 객체+관리정보 단일 할당 | 객체에 카운터 포함 | `make_shared`에서 가능 |

- 어느 쪽이 항상 빠르다고 단정할 수는 없다.
- 이 설계의 목적은 **기존 CAD 객체에 내장된 참조 카운팅을 간단한 API로 활용하는 것**이다.

---

### 📌 8. 소스 코드
```cpp
#pragma once

#include <atomic>
#include <cassert>

class ref_counted
{
public:
    ref_counted() noexcept
        : _refs(0)
    {
    }

    // 복사된 객체는 새로운 객체이므로 참조 카운터는 0
    ref_counted(const ref_counted&) noexcept
        : _refs(0)
    {
    }

    virtual ~ref_counted()
    {
        assert(_refs.load() == 0);
    }

    // 참조 카운터 증가
    void acquire() const noexcept
    {
        _refs.fetch_add(1, std::memory_order_relaxed);
    }

    // 참조 카운터 감소
    void release() const noexcept
    {
        const int old =
            _refs.fetch_sub(1, std::memory_order_acq_rel);

        assert(old > 0);

        if (old == 1)
        {
            delete this;
        }
    }

    // 현재 참조 카운터
    int ref_count() const noexcept
    {
        return _refs.load(std::memory_order_relaxed);
    }

    ref_counted& operator=(const ref_counted&) noexcept
    {
        // 대입 시 기존 객체의 참조 카운터 유지
        return *this;
    }

private:
    mutable std::atomic<int> _refs;
};
```
```cpp
#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "ref_counted.hpp"

template<typename T>
class Ptr
{
public:
    using pointer_type = T;

    template<typename U>
    friend class Ptr;

    // 기본 생성자
    Ptr() noexcept
        : _p(nullptr)
    {
    }

    Ptr(std::nullptr_t) noexcept
        : _p(nullptr)
    {
    }

    // Raw pointer 생성자
    Ptr(T* p) noexcept
        : _p(p)
    {
        if (_p)
            _p->acquire();
    }

    // 복사 생성자
    Ptr(const Ptr& other) noexcept
        : Ptr(other._p)
    {
    }

    // 상속 관계의 Ptr 복사
    template<
        typename U,
        typename = std::enable_if_t<
            std::is_convertible_v<U*, T*>
        >
    >
    Ptr(const Ptr<U>& other) noexcept
        : Ptr(other._p)
    {
    }

    // 이동 생성자
    Ptr(Ptr&& other) noexcept
        : _p(other._p)
    {
        other._p = nullptr;
    }

    // 소멸자
    ~Ptr()
    {
        if (_p)
            _p->release();
    }

    // 복사 대입
    Ptr& operator=(const Ptr& other) noexcept
    {
        reset(other._p);
        return *this;
    }

    // Raw pointer 대입
    Ptr& operator=(T* p) noexcept
    {
        reset(p);
        return *this;
    }

    // 이동 대입
    Ptr& operator=(Ptr&& other) noexcept
    {
        if (this != &other)
        {
            T* old = _p;

            _p = other._p;
            other._p = nullptr;

            if (old)
                old->release();
        }

        return *this;
    }

    Ptr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    // 참조 대상 변경
    void reset(T* p = nullptr) noexcept
    {
        // 먼저 acquire해야 reset(ptr())도 안전
        if (p)
            p->acquire();

        T* old = _p;
        _p = p;

        if (old)
            old->release();
    }

    T* ptr() const noexcept
    {
        return _p;
    }

    T* get() const noexcept
    {
        return _p;
    }

    T* operator->() const noexcept
    {
        assert(_p);
        return _p;
    }

    T& operator*() const noexcept
    {
        assert(_p);
        return *_p;
    }

    explicit operator bool() const noexcept
    {
        return _p != nullptr;
    }

    bool is_valid() const noexcept
    {
        return _p != nullptr;
    }

private:
    T* _p;
};

// 생성 편의 함수
template<typename T, typename... Args>
Ptr<T> make_ptr(Args&&... args)
{
    static_assert(
        std::is_base_of_v<ref_counted, T>,
        "T must derive from ref_counted"
    );

    return Ptr<T>(
        new T(std::forward<Args>(args)...)
    );
}
```

### 📌 9. 테스트 코드
```cpp
#include "test_IntrusivePtr.h"
#include "ptr.hpp"
#include <atomic>
#include <iostream>
#include <string>
#include <exception>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

static int passed = 0;
static int failed = 0;
static void check(bool ok, const char* label) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << label << '\n';
    (ok ? passed : failed)++;
}

// Test-only CAD-like objects; no dependencies on the real CAD Core.
struct Counters {
    std::atomic<int> created{0};
    std::atomic<int> destroyed{0};
};

class TestGeometry : public ref_counted {
public:
    TestGeometry(int id, Counters& counters) : id_(id), counters_(counters) { ++counters_.created; }
    ~TestGeometry() override { ++counters_.destroyed; }
    int id() const { return id_; }
    virtual const char* type() const = 0;
private:
    int id_;
    Counters& counters_;
};

class TestCurve final : public TestGeometry {
public:
    TestCurve(int id, Counters& counters) : TestGeometry(id, counters) {}
    const char* type() const override { return "Curve"; }
};

class TestSurface final : public TestGeometry {
public:
    TestSurface(int id, Counters& counters) : TestGeometry(id, counters) {}
    const char* type() const override { return "Surface"; }
};

class TestModel {
public:
    void add(const Ptr<TestGeometry>& geometry) { items_.push_back(geometry); }
    std::size_t count() const { return items_.size(); }
    Ptr<TestGeometry> get(std::size_t index) const { return items_.at(index); }
    void clear() { items_.clear(); }
private:
    std::vector<Ptr<TestGeometry>> items_;
};

static void test_creation_and_copy() {
    Counters stats;
    {
        auto curve = make_ptr<TestCurve>(100, stats);
        check(curve && curve->id() == 100 && curve->ref_count() == 1, "create / initial reference = 1");
        {
            Ptr<TestCurve> second = curve;
            check(curve->ref_count() == 2 && second.get() == curve.get(), "copy construction / share object");
            second = second;
            check(curve->ref_count() == 2, "self-copy assignment");
            second.reset(second.get());
            check(curve->ref_count() == 2, "reset with same raw pointer");
        }
        check(curve->ref_count() == 1, "scope exit decrements reference");
    }
    check(stats.created == 1 && stats.destroyed == 1, "destroy exactly once");
}

static void test_assignment_and_move() {
    Counters stats;
    {
        auto first = make_ptr<TestCurve>(1, stats);
        auto second = make_ptr<TestCurve>(2, stats);
        second = first;
        check(stats.destroyed == 1 && first->ref_count() == 2, "copy assignment releases previous object");
        Ptr<TestCurve> moved(std::move(second));
        check(!second && moved->ref_count() == 2, "move construction does not increase reference");
        Ptr<TestCurve> target;
        target = std::move(moved);
        check(!moved && target->ref_count() == 2, "move assignment transfers ownership");
        target = nullptr;
        check(first->ref_count() == 1, "nullptr assignment releases reference");
        first = first.get();
        check(first->ref_count() == 1, "raw pointer self-assignment");
    }
    check(stats.created == 2 && stats.destroyed == 2, "assignment/move: no leak or double destruction");
}

static void test_polymorphism_and_model() {
    Counters stats;
    {
        auto curve = make_ptr<TestCurve>(10, stats);
        auto surface = make_ptr<TestSurface>(20, stats);
        Ptr<TestGeometry> base = curve;
        check(base->id() == 10 && curve->ref_count() == 2, "derived -> base conversion");
        TestModel model;
        model.add(curve);
        model.add(surface);
        check(model.count() == 2 && curve->ref_count() == 3 && surface->ref_count() == 2,
              "model shares CAD-like geometry");
        {
            auto item = model.get(1);
            check(item->id() == 20 && item->type() == std::string("Surface") && surface->ref_count() == 3,
                  "model lookup returns shared pointer");
        }
        model.clear();
        check(curve->ref_count() == 2 && surface->ref_count() == 1, "model clear releases references");
    }
    check(stats.created == 2 && stats.destroyed == 2, "polymorphic destruction exactly once");
}

static void test_threaded_reference_count() {
    Counters stats;
    {
        auto shared = make_ptr<TestCurve>(77, stats);
        constexpr int threadCount = 8;
        constexpr int iterations = 10000;
        std::vector<std::thread> threads;
        threads.reserve(threadCount);
        for (int t = 0; t < threadCount; ++t) {
            // Capture a copy; worker threads only copy Ptr and read immutable data.
            threads.emplace_back([shared]() {
                for (int i = 0; i < iterations; ++i) {
                    Ptr<TestCurve> copy = shared;
                    if (copy->id() != 77) std::terminate();
                }
            });
        }
        for (auto& thread : threads) thread.join();
        check(shared->ref_count() == 1, "concurrent copies/releases preserve reference count");
        check(stats.destroyed == 0, "shared object remains alive during test");
    }
    check(stats.created == 1 && stats.destroyed == 1, "threaded test destroys object once");
}

int main() {
    static_assert(sizeof(Ptr<TestCurve>) == sizeof(TestCurve*), "Ptr should store one pointer");
    static_assert(std::is_convertible_v<Ptr<TestCurve>, Ptr<TestGeometry>>, "upcast must be supported");
    static_assert(!std::is_convertible_v<Ptr<TestGeometry>, Ptr<TestCurve>>, "implicit downcast must be rejected");
    test_creation_and_copy();
    test_assignment_and_move();
    test_polymorphism_and_model();
    test_threaded_reference_count();
    std::cout << "----------------------------------------\nPASS   : " << passed
              << "\nFAILED : " << failed << "\nRESULT : " << (failed ? "FAIL" : "PASS") << '\n';
    return failed ? 1 : 0;
}
```

---




