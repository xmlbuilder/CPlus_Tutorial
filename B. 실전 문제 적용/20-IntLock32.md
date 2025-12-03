## IntLock32 사용 설명서
### 1. IntLock32 개요
- IntLock32는 32비트 정수 하나로 구현한 아주 가벼운 락입니다.
- 내부 구현: std::atomic<int>
- 크기: sizeof(IntLock32) == sizeof(int) (static_assert로 확인)
- 특징
  - try-lock 스타일 (GetLock, GetDefaultLock)
  - lock 값(토큰)을 직접 지정 가능 → 디버그/owner 표시 등에 활용 가능
  - 강제 해제(BreakLock) 제공
  - RAII 가드(IntLockGuard)로 편하게 사용 가능

### 2. 상태 값 / 상수
```cpp
enum : int
{
  UnlockedValue      = 0,  // 잠금이 풀린 상태
  DefaultLockedValue = 1,  // 기본 잠금 값
  InvalidLockValue   = -1  // 사용 금지 값
};
```

- 0 (UnlockedValue)
  - 락이 풀린 상태
- 1 (DefaultLockedValue)
  - 기본적으로 락을 잡을 때 사용하는 값
- -1 (InvalidLockValue)
  - 내부적으로 “유효하지 않은 값”을 의미.
    - 이 값으로는 락을 잡거나 풀 수 없음.
## 3. 함수별 설명
### 3.1 생성자 / 복사 금지
```cpp
IntLock32() noexcept
  : m_lock_value(UnlockedValue)
{}

IntLock32(const IntLock32&)            = delete;
IntLock32& operator=(const IntLock32&) = delete;
```

- 기본 생성: **항상 unlocked 상태(0)** 로 시작.
- 복사/대입은 금지 (락은 복사되는 순간 의미가 깨지므로).

### 3.2 int IsLocked() const
```cpp
int IsLocked() const noexcept
{
  return m_lock_value.load(std::memory_order_acquire);
}
```

- 현재 락의 raw 값(int) 을 반환.
  - 0 → unlocked
  - != 0 → 어떤 토큰으로든 잠긴 상태

- 디버깅/로그용으로 “누가 잡았는지”를 lock 값에 실어두고 싶을 때 유용.

### 3.3 bool IsLockedFlag() const
```cpp
bool IsLockedFlag() const noexcept
{
  return IsLocked() != UnlockedValue;
}
```

- 단순히 **잠겼는지 여부** 만 boolean으로 알고 싶을 때 사용.
  - true → 누군가 잡고 있음
  - false → unlocked

### 3.4 bool GetDefaultLock()
```cpp
bool GetDefaultLock() noexcept
{
  return GetLock(DefaultLockedValue);
}
```

- 기본 토큰(1) 로 try_lock을 시도합니다.
- 결과:
  - 성공 → true (0 → 1로 변경)
  - 실패 → false (이미 잠겨있거나, 내부 오류)

### 3.5 bool ReturnDefaultLock()
```cpp
bool ReturnDefaultLock() noexcept
{
  return ReturnLock(DefaultLockedValue);
}
```
- 현재 락이 DefaultLockedValue(1)로 잠겨있을 때만 unlock을 수행.
- 결과:
  - 현재 값이 1이면 0으로 돌리고 true
  - 그렇지 않으면 아무 것도 하지 않고 false

### 3.6 bool GetLock(int lock_value)
```cpp
bool GetLock(int lock_value) noexcept
{
  if (lock_value == UnlockedValue || lock_value == InvalidLockValue)
    return false;

  int expected = UnlockedValue;
  const bool ok = m_lock_value.compare_exchange_strong(
      expected,
      lock_value,
      std::memory_order_acq_rel,
      std::memory_order_acquire);

  return (ok && expected == UnlockedValue);
}
```
- 사용자 지정 토큰(lock_value)으로 try-lock.

#### 규칙
- lock_value 는
  - != 0(UnlockedValue) 이어야 하고
  - != -1(InvalidLockValue) 이어야 합니다.
- 현재 값이 0일 때만 lock_value로 바꾸고 성공(true).
- 이미 누군가 잡고 있으면 false.

#### 활용 예
- lock_value에 스레드 ID나, 객체 ID 같은 걸 넣어서
  - 디버그 시 “누가 잡고 있는지”를 추적할 수 있음.

### 3.7 bool ReturnLock(int lock_value)
```cpp
bool ReturnLock(int lock_value) noexcept
{
  if (lock_value == UnlockedValue || lock_value == InvalidLockValue)
    return false;

  int expected = lock_value;
  const bool ok = m_lock_value.compare_exchange_strong(
      expected,
      UnlockedValue,
      std::memory_order_acq_rel,
      std::memory_order_acquire);

  return (ok && expected == lock_value);
}
```
- 지정한 토큰(lock_value)으로 잠겨 있을 때만 unlock.

#### 규칙
- 현재 락 값이 lock_value일 때만
  - lock_value → 0 으로 변경
  - true 반환

- 현재 값이 다르면:
  - 아무 것도 바꾸지 않고 false 반환

#### 용도
**내가 잡은 락만 풀고 싶다**  라는 의도를 명확하게 표현할 수 있음.

### 3.8 int BreakLock()
```cpp
int BreakLock() noexcept
{
  return m_lock_value.exchange(UnlockedValue, std::memory_order_acq_rel);
}
```
- 무조건 강제 해제합니다.

- 반환값:
  - 잠그기 전의 값 (0이면 원래 unlocked 상태였던 것)
  - 0이 아니면 **누군가/뭔가가 잡고 있었다** 는 의미

#### 사용 예
- 타임아웃 / 비상 복구:
  - 너무 오래 잠겨 있는 경우, 관리자 코드에서 강제로 해제하고 싶을 때
  - 이전 값으로 **누가 잡았었는지** 를 기록해서 디버깅에 사용 가능

## 4. RAII 가드: IntLockGuard
```cpp
class IntLockGuard
{
public:
  explicit IntLockGuard(
      IntLock32& lock,
      int token = IntLock32::DefaultLockedValue) noexcept
    : m_lock(lock)
    , m_token(token)
    , m_owns(false)
  {
    m_owns = m_lock.GetLock(m_token);
  }

  ~IntLockGuard()
  {
    if (m_owns)
      (void)m_lock.ReturnLock(m_token);
  }

  bool owns_lock() const noexcept { return m_owns; }

  // 복사 금지
  IntLockGuard(const IntLockGuard&)            = delete;
  IntLockGuard& operator=(const IntLockGuard&) = delete;

private:
  IntLock32& m_lock;
  int        m_token;
  bool       m_owns;
};
```

#### 동작 방식
- 생성자:
  - lock.GetLock(token)을 호출해서 try-lock
  - 성공 시 m_owns = true

- 소멸자:
  - m_owns == true이면 lock.ReturnLock(token) 호출 → 자동 해제

#### owns_lock() 사용
- IntLockGuard g(lock);
  - 이때 락을 못 얻을 수도 있으므로,
  - if (!g.owns_lock()) { /*실패 처리*/ } 식으로 체크 가능.

## 5. 사용 예제 – 단계별
### 5.1 기본적인 단일 스레드 사용
```cpp
IntLock32 lock;

// 1) 기본 토큰으로 잠금 시도
if (lock.GetDefaultLock())
{
    // 여기부터 락 구간
    // ...

    // 2) 반드시 해제
    bool ok = lock.ReturnDefaultLock();
    // ok == true 여야 정상
}
else
{
    // 이미 누가 잡고 있어서 실패
}
```

### 5.2 커스텀 토큰 사용 (디버깅 / owner 표시)
```cpp
IntLock32 lock;
int my_token = 1234; // 예: 스레드 ID, 객체 ID 등

if (lock.GetLock(my_token))
{
    // 내가 락을 잡음
    // 디버깅 시 lock.IsLocked() 값이 1234인지 확인 가능

    // 해제할 때도 같은 토큰을 사용
    bool ok = lock.ReturnLock(my_token);
    // ok == true 이어야 함
}
else
{
    // 이미 다른 값으로 잠겨 있음
    int current = lock.IsLocked();
    // current != 0 이라면 누군가 잡고 있는 상태
}
```

### 5.3 RAII 방식으로 사용하기
```cpp
IntLock32 lock;

void critical_section()
{
    IntLockGuard guard(lock); // 기본 토큰으로 try-lock

    if (!guard.owns_lock())
    {
        // 락 못 잡았으면 그냥 리턴하거나 예외 처리
        return;
    }

    // 여기서부터 스코프 끝날 때까지, 락이 유지됨
    // 스코프를 벗어나면 guard 소멸자에서 자동으로 ReturnLock 호출
}
```

- 예외/조기 return이 있어도 자동으로 unlock되기 때문에, 락 깜빡 잊고 안 푸는 버그를 줄일 수 있음.

### 5.4 멀티 스레드에서 spin lock으로 사용
```cpp
IntLock32 lock;
int shared_counter = 0;

void worker()
{
    for (int i = 0; i < 10000; ++i)
    {
        // 아주 단순한 spin lock
        while (!lock.GetDefaultLock())
        {
            // 필요하다면 std::this_thread::yield(); 등을 넣어서 CPU 낭비 줄이기
        }

        // 크리티컬 섹션
        ++shared_counter;

        bool ok = lock.ReturnDefaultLock();
        (void)ok;
    }
}
```

- while (!GetDefaultLock()) {} 패턴으로 spin-lock 구현.
- 실제 코드에서는
  - 일정 횟수 이상 실패 시 yield()나 sleep_for로 back-off 넣어주는 걸 권장.

### 5.5 강제 해제로 비상 처리 (BreakLock)
```cpp
IntLock32 lock;

// 어딘가에서 너무 오래 잡혀 있다고 판단되는 경우:

int prev = lock.BreakLock();
if (prev != IntLock32::UnlockedValue)
{
    // 원래 누군가가 lock을 잡고 있었다는 뜻
    // prev 값에 기반해 디버그 로그 남기거나, 복구 로직 수행 가능
}
```
- 주의:
  - 강제 해제를 하면 원래 락을 잡고 있던 스레드는 “자기 락이 풀린 줄 모르고” 실행을 계속할 수 있습니다.
  - 그래서 보통은 디버그/비상 상황에서만 사용하고,
  - 정상 로직에서는 ReturnLock / ReturnDefaultLock만 사용하는 것이 좋습니다.

## 6. 요약

- IntLock32는:
  - 4바이트짜리 very-lightweight lock
  - GetLock / ReturnLock / GetDefaultLock / ReturnDefaultLock / BreakLock 제공
  - lock 값에 토큰을 실을 수 있어 디버그/owner 추적에 유리

- IntLockGuard로 RAII 스타일 사용 가능:
  - scope 기반으로 자동 해제
- 사용 패턴은 크게 세 가지:
  - 단순 try-lock / unlock
  - 커스텀 토큰을 사용한 owner 표시
  - 멀티스레드에서 spin-lock으로 보호 + RAII

---

### 소스 코드 
```cpp
// IntLock32.h
#pragma once

#include <atomic>
#include <ostream>

class IntLock32
{
public:
  // 잠금 상태 값 정의
  enum : int
  {
    UnlockedValue      = 0,
    DefaultLockedValue = 1,
    InvalidLockValue   = -1
  };

  IntLock32() noexcept
    : m_lock_value(UnlockedValue)
  {}

  ~IntLock32() = default;

  IntLock32(const IntLock32&)            = delete;
  IntLock32& operator=(const IntLock32&) = delete;

  // 현재 상태: 0 이면 unlocked
  int IsLocked() const noexcept
  {
    return m_lock_value.load(std::memory_order_acquire);
  }

  bool IsLockedFlag() const noexcept
  {
    return IsLocked() != UnlockedValue;
  }

  // 기본 토큰으로 try-lock
  bool GetDefaultLock() noexcept
  {
    return GetLock(DefaultLockedValue);
  }

  // 기본 토큰으로 unlock
  bool ReturnDefaultLock() noexcept
  {
    return ReturnLock(DefaultLockedValue);
  }

  // 주어진 lock_value로 try-lock
  bool GetLock(int lock_value) noexcept
  {
    if (lock_value == UnlockedValue || lock_value == InvalidLockValue)
      return false;

    int expected = UnlockedValue;
    const bool ok = m_lock_value.compare_exchange_strong(
        expected,
        lock_value,
        std::memory_order_acq_rel,
        std::memory_order_acquire);

    return (ok && expected == UnlockedValue);
  }

  // 주어진 lock_value로 unlock
  bool ReturnLock(int lock_value) noexcept
  {
    if (lock_value == UnlockedValue || lock_value == InvalidLockValue)
      return false;

    int expected = lock_value;
    const bool ok = m_lock_value.compare_exchange_strong(
        expected,
        UnlockedValue,
        std::memory_order_acq_rel,
        std::memory_order_acquire);

    return (ok && expected == lock_value);
  }

  // 강제 해제: 이전 값을 돌려줌
  int BreakLock() noexcept
  {
    return m_lock_value.exchange(UnlockedValue, std::memory_order_acq_rel);
  }

private:
  std::atomic<int> m_lock_value;
};

inline std::ostream& operator<<(std::ostream& os, const IntLock32& lock)
{
  os << "IntLock32(" << lock.IsLocked() << ')';
  return os;
}

// RAII 가드
class IntLockGuard
{
public:
  explicit IntLockGuard(
      IntLock32& lock,
      int token = IntLock32::DefaultLockedValue) noexcept
    : m_lock(lock)
    , m_token(token)
    , m_owns(false)
  {
    m_owns = m_lock.GetLock(m_token);
  }

  ~IntLockGuard()
  {
    if (m_owns)
      (void)m_lock.ReturnLock(m_token);
  }

  IntLockGuard(const IntLockGuard&)            = delete;
  IntLockGuard& operator=(const IntLockGuard&) = delete;

  bool owns_lock() const noexcept { return m_owns; }

private:
  IntLock32& m_lock;
  int        m_token;
  bool       m_owns;
};

// with_int_lock 헬퍼는 잠깐 빼두는 게 안전합니다.
// (이 안에서 다시 GetLock을 부르면 double-lock 문제가 생기기 쉬워서
//  필요하면 나중에 별도로 정리해서 넣는 게 깔끔합니다.)

static_assert(sizeof(IntLock32) == sizeof(int),
              "IntLock32 is intended to be the same size as int");
```

### 샘플 소스

```cpp
// test_IntLock32.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include "int_lock32.h"

bool test_single_thread_basic()
{
  std::cout << "[TEST] single-thread basic\n";

  IntLock32 lock;
  // 초기 상태는 unlock
  assert(lock.IsLocked() == IntLock32::UnlockedValue);
  assert(!lock.IsLockedFlag());

  // 기본 토큰으로 lock
  bool ok = lock.GetDefaultLock();
  assert(ok);
  assert(lock.IsLockedFlag());

  // 이미 잠긴 상태에서 다시 lock 시도 -> 실패
  ok = lock.GetDefaultLock();
  assert(!ok);

  // unlock
  ok = lock.ReturnDefaultLock();
  assert(ok);
  assert(!lock.IsLockedFlag());

  // 이미 unlock 상태에서 unlock -> 실패
  ok = lock.ReturnDefaultLock();
  assert(!ok);

  std::cout << "  OK\n";
  return true;
}
```
```cpp
bool test_custom_token_and_break()
{
  std::cout << "[TEST] custom token & BreakLock\n";

  IntLock32 lock;
  const int token = 1234;

  // 커스텀 토큰으로 잠금
  bool ok = lock.GetLock(token);
  assert(ok);
  assert(lock.IsLocked() == token);

  // 잘못된 토큰으로 ReturnLock -> 실패
  ok = lock.ReturnLock(token + 1);
  assert(!ok);
  assert(lock.IsLocked() == token);

  // BreakLock: 강제 해제 + 이전 값 리턴
  int prev = lock.BreakLock();
  assert(prev == token);
  assert(lock.IsLocked() == IntLock32::UnlockedValue);

  // BreakLock 다시 호출해도 그냥 0 리턴
  prev = lock.BreakLock();
  assert(prev == IntLock32::UnlockedValue);

  std::cout << "  OK\n";
  return true;
}
```
```cpp
bool test_guard()
{
  std::cout << "[TEST] IntLockGuard\n";

  IntLock32 lock;

  {
    IntLockGuard g(lock);
    assert(g.owns_lock());
    assert(lock.IsLockedFlag());


    IntLockGuard g2(lock);
    assert(!g2.owns_lock());
    assert(lock.IsLockedFlag());
  }

  // 첫 번째 가드가 scope를 빠져나가면서 unlock 되어야 함
  assert(!lock.IsLockedFlag());

  std::cout << "  OK\n";
  return true;
}
```
```cpp
bool test_multithread_counter()
{
  std::cout << "[TEST] multi-thread counter\n";

  IntLock32 lock;
  int counter = 0;

  constexpr int kNumThreads    = 8;
  constexpr int kIncrementsPer = 10000;

  auto worker = [&]()
  {
    for (int i = 0; i < kIncrementsPer; ++i)
    {
      // 간단한 spin-lock
      while (!lock.GetDefaultLock())
      {
        // 실 코드에서는 std::this_thread::yield() 정도 넣는 걸 권장
      }

      // 크리티컬 섹션
      ++counter;

      bool ok = lock.ReturnDefaultLock();
      assert(ok);
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(kNumThreads);
  for (int i = 0; i < kNumThreads; ++i)
    threads.emplace_back(worker);

  for (auto& t : threads)
    t.join();

  const int expected = kNumThreads * kIncrementsPer;
  std::cout << "  counter = " << counter
            << " (expected " << expected << ")\n";
  assert(counter == expected);

  std::cout << "  OK\n";
  return true;
}
```
```cpp
int main()
{
  bool ok = true;

  ok = ok && test_single_thread_basic();
  ok = ok && test_custom_token_and_break();
  ok = ok && test_guard();
  ok = ok && test_multithread_counter();

  if (ok)
  {
    std::cout << "\n=== ALL IntLock32 TESTS PASSED ===\n";
    return 0;
  }
  else
  {
    std::cerr << "\n=== SOME TESTS FAILED ===\n";
    return 1;
  }
}
```
## 출력 코드
```
[TEST] single-thread basic
  OK
[TEST] custom token & BreakLock
  OK
[TEST] IntLockGuard
  OK
[TEST] multi-thread counter
  counter = 80000 (expected 80000)
  OK

=== ALL IntLock32 TESTS PASSED ===
```
---
