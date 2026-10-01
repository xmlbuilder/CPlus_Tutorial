## 📘 `std::call_once()` --- Thread-Safe 1회 실행

### 📌 1. 개요

- `std::call_once()`는 특정 작업을 **성공적으로 한 번만 실행** 하도록 보장하는 C++11 기능입니다.
- 여러 Thread가 동시에 호출해도 동일한 `std::once_flag`에 대해 한 번만 실행됩니다.
- 초기화, Lookup Table 생성, Library 초기화 등 **한 번만 필요한 작업** 에 사용합니다.

``` cpp
#include <mutex>

std::once_flag initFlag;

std::call_once(initFlag, []() {
    Initialize();
});
```

### 📌 2. 기본 형태

``` cpp
std::call_once(flag, callable, args...);
```

  | 항목 | 역할 |
  |------|-------|
  | `std::once_flag` | 1회 실행 상태 관리|
  | `std::call_once()` | callable을 한 번만 실행|
  | `callable` | 함수, Lambda, 함수 객체|

- 핵심은 **같은 `once_flag`를 사용하는 호출들이 하나의 실행 상태를 공유**.

``` cpp
std::once_flag flag;

std::call_once(flag, Initialize); // 실행
std::call_once(flag, Initialize); // 실행 안 함
std::call_once(flag, Initialize); // 실행 안 함
```

### 📌 3. Multi-Thread 동작

- 여러 Thread가 동시에 호출해도:

``` text
Thread A ─┐
Thread B ─┼──→ call_once(flag, Initialize)
Thread C ─┘
```

- 실제 초기화는 한 번만 성공적으로 수행됩니다.

``` text
             call_once(flag)
                   │
          ┌────────┴────────┐
          │                 │
     최초 성공 전       이미 성공함
          │                 │
          ▼                 ▼
     Initialize()        실행 안 함
          │
          ▼
       정상 완료
```

- 초기화를 수행하는 동안 경쟁하는 다른 Thread들은 필요한 동기화를 거치므로,
- 초기화가 끝난 결과를 안전하게 사용할 수 있습니다.

### 📌 4. `bool`과의 차이

- 다음 코드는 Thread-Safe하지 않습니다.

``` cpp
bool initialized = false;

if (!initialized) {
    Initialize();
    initialized = true;
}
```

- 두 Thread가 동시에 `initialized == false`를 확인할 수 있어 `Initialize()`가 여러 번 실행될 수 있습니다.

- `call_once()`를 사용하면:

``` cpp
std::once_flag flag;
std::call_once(flag, Initialize);
```

- 1회 실행과 Thread 동기화를 표준 라이브러리가 처리합니다.


### 📌 5. 예외 발생 시

`call_once()` 내부의 함수가 예외를 발생시키면 **실행 완료로 처리되지 않습니다.**

``` cpp
std::call_once(flag, []() {
    Initialize();  // 여기서 예외 발생
});
```

- 동작:

``` text
call_once()
    ↓
Initialize()
    ↓
예외 발생
    ↓
완료되지 않음
    ↓
다음 call_once()에서 재시도 가능
```

- 즉, **호출 시도가 한 번**이라는 뜻이 아니라 **정상적으로 완료되는 실행이 한 번** 이라는 의미입니다.


### 📌 6. 서로 다른 작업에는 다른 flag 사용

``` cpp
std::once_flag flagA;
std::once_flag flagB;

std::call_once(flagA, InitializeA);
std::call_once(flagB, InitializeB);
```
- 같은 flag를 사용하면 두 함수가 같은 "한 번"을 공유하므로 주의해야 합니다.

``` cpp
std::once_flag flag;

std::call_once(flag, InitializeA); // 실행
std::call_once(flag, InitializeB); // 실행되지 않음
```

### 📌 7. Singleton 사용 예

- Singleton은 대표적인 활용 예 중 하나입니다.

``` cpp
static std::once_flag initFlag;

std::call_once(initFlag, []() {
    instance.reset(new Singleton());
});
```

- 여러 Thread가 동시에 객체 생성을 요청해도 생성 코드는 한 번만 성공적으로 실행됩니다.


### 📌 8. 주의 사항

- 동일 작업은 **동일한 `once_flag`** 를 사용해야 함
- 서로 다른 1회 작업에는 각각 별도의 `once_flag` 사용
- 예외 발생 시 이후 호출에서 다시 실행될 수 있음
- `once_flag`에는 일반적인 `reset()` 기능이 없음
- `Initialize → Shutdown → Initialize`처럼 반복 초기화가 필요하면 다른 상태 관리 방식이 적합
- `call_once()`는 외부의 모든 공유 데이터를 자동으로 Thread-Safe하게 만드는 기능은 아님

### 📌 9. 핵심 정리

``` cpp
std::once_flag flag;

std::call_once(flag, []() {
    Initialize();
});
```

- 기억할 것은 세 가지입니다.

``` text
같은 once_flag
      ↓
여러 Thread가 호출해도
      ↓
정상 완료되는 실행은 한 번
```

> **`std::call_once()` = 여러 Thread 환경에서 특정 초기화 작업을 안전하게 한 번만 수행할 때 사용**

---

