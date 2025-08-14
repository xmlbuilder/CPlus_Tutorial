# C++11 `std::mutex` 심화 가이드

## 1. 개념
C++11부터 `<thread>`와 `<mutex>`를 통해 표준 라이브러리 차원에서 멀티스레드 동기화를 지원합니다.  
`std::mutex`는 **뮤텍스(Mutual Exclusion)**로, 공유 자원에 대한 **상호 배제**를 보장합니다.

---

## 2. 기본 사용법

```cpp
#include <thread>
#include <mutex>
#include <iostream>

std::mutex mtx;

void ThreadFunc(int nID, int& nVal)
{
    for (int i = 0 ; i < 5 ; ++i)
    {
        mtx.lock();
        std::cout << "Value of " << nID << " -> " << nVal << std::endl;
        ++nVal;
        mtx.unlock();
    }
}

int main()
{
    int nVal = 0;
    std::thread th1(ThreadFunc, 0, std::ref(nVal));
    std::thread th2(ThreadFunc, 1, std::ref(nVal));
    th1.join();
    th2.join();
}
```

- `lock()` : 잠금 요청 (다른 스레드가 잠금 중이면 대기)
- `unlock()` : 잠금 해제
- 공유 자원에 접근할 때 반드시 잠금/해제 쌍을 사용

---

## 3. `try_lock()`

```cpp
if (mtx.try_lock()) {
    // 잠금 성공
    mtx.unlock();
} else {
    // 잠금 실패 → 다른 작업
}
```

- 바로 리턴하며 성공 여부 반환
- 이미 동일 스레드가 잠근 상태에서 `lock()`/`try_lock()` 호출 시 **미정의 동작**

---

## 4. `std::recursive_mutex`

- 동일한 스레드가 같은 mutex를 여러 번 잠글 수 있음
- 잠근 횟수만큼 해제해야 완전히 풀림

```cpp
#include <thread>
#include <mutex>
#include <iostream>

std::recursive_mutex mtx;

void ThreadFunc(int nID, int& nVal)
{
    for (int i = 0; i < 5; ++i)
    {
        std::lock_guard<std::recursive_mutex> lg(mtx); // RAII
        std::cout << "Value of " << nID << " -> " << nVal << std::endl;
        ++nVal;
    }
}
```

---

## 5. RAII와 `std::lock_guard`

```cpp
{
    std::lock_guard<std::mutex> lg(mtx);
    // lock 상태
} // 범위를 벗어나면 자동 unlock
```

- 생성 시 lock, 소멸 시 unlock → 예외 안전성 보장

---

## 6. `std::unique_lock`

```cpp
std::unique_lock<std::mutex> ul(mtx, std::defer_lock);
ul.lock();
// ...
ul.unlock();
```

- `lock_guard`보다 유연 (나중에 lock, 중간에 unlock 가능)
- `std::condition_variable`과 함께 사용 가능

---

## 7. 시간 제한 Mutex

- `std::timed_mutex`, `std::recursive_timed_mutex`
- `try_lock_for(duration)` : 일정 시간 동안 재시도
- `try_lock_until(time_point)` : 특정 시각까지 재시도

```cpp
std::timed_mutex tmtx;
if (tmtx.try_lock_for(std::chrono::milliseconds(100))) {
    // 잠금 성공
    tmtx.unlock();
} else {
    // 시간 초과
}
```

---

## 8. Mutex 동작 원리

### 8.1 커널 수준 락 (Kernel Lock)
- 뮤텍스를 획득할 수 없는 경우 **운영체제 커널에 진입**하여 스레드를 **수면 상태(block)**로 전환
- 장점: CPU 낭비 없음
- 단점: 커널 모드 전환 비용 높음 (시스템 콜 오버헤드)

### 8.2 사용자 공간 스핀락 (User-space Spinlock)
- 잠금을 획득할 수 없으면 **바로 수면하지 않고** 잠금이 풀릴 때까지 루프를 돌며 대기
- 장점: 짧은 시간 안에 잠금이 풀릴 경우 커널 진입 없이 빠르게 처리
- 단점: 잠금이 오래 유지되면 CPU 자원 낭비

### 8.3 Futex(Fast Userspace Mutex)
- 리눅스에서 사용되는 하이브리드 방식
- 잠금 경쟁이 없을 때는 **사용자 공간에서 바로 획득/해제**
- 경쟁 발생 시에만 커널 진입 → 대기 스레드 관리
- 동작 흐름:
  1. try_lock 시도 → 성공 시 사용자 공간에서 처리
  2. 실패 시 `futex` 시스템 콜로 커널에 진입 → 스레드 블록
  3. 잠금 해제 시 대기 스레드를 깨움

### 8.4 실제 구현
- glibc `pthread_mutex`는 내부적으로 futex를 사용
- Windows `CRITICAL_SECTION`은 유사하게 사용자 모드 스핀 + 커널 오브젝트 대기 방식을 혼합

---

## 9. 요약
- **mutex 종류**: `std::mutex`, `std::recursive_mutex`, `std::timed_mutex`, `std::recursive_timed_mutex`
- **RAII**: `lock_guard`, `unique_lock`으로 예외 안전성 확보
- **성능**: 잠금 유지 시간이 짧으면 스핀락 유리, 길면 커널 락이 효율적
- **플랫폼 구현**: Linux는 futex, Windows는 SRWLock/CRITICAL_SECTION 기반

