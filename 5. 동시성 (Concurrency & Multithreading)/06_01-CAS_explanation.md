## 📘 C++ CAS 예제 상세 분석

- 다음 코드는 **구조체 `Pair` 전체를 하나의 원자 단위로 취급**하면서, 그 안의 **`a`만 +1** 하는 **CAS(compare-and-swap) 루프** 입니다.

```cpp
#include <atomic>

struct Pair {
    int a;
    int b;
};

// 초기값을 명시합니다.
std::atomic<Pair> g_pair{Pair{0, 0}};

void try_update() {
    Pair expected = g_pair.load(std::memory_order_acquire);
    Pair desired  = expected;
    desired.a += 1;

    while (!g_pair.compare_exchange_weak(expected, desired,
                                         std::memory_order_acq_rel,
                                         std::memory_order_acquire)) {
        desired = expected;
        desired.a += 1;
    }
}
```

---

### 📌 1. 코드가 하려는 일

- `g_pair`는 `Pair{a,b}`를 통째로 담고 있는 `std::atomic<Pair>`입니다.
- 여러 스레드가 동시에 증가를 시도해도, **CAS가 성공하는 순간의 `a`를 1 증가시키고 `b`는 유지** 합니다.
- 명시적인 mutex 없이 갱신하지만, **`std::atomic<Pair>`가 반드시 lock-free인 것은 아닙니다.** 구현에 따라 내부 잠금을 사용할 수 있습니다.

### 📌 2. 동작 흐름

```cpp
Pair expected = g_pair.load(std::memory_order_acquire);
Pair desired  = expected;
desired.a += 1;
```

- 공유 값을 `expected`로 읽어옵니다.
- 이를 복사해 `desired`를 만들고, `desired.a`만 +1 합니다.
- 두 변수는 지역 복사본이므로, 이 단계에서는 `g_pair`가 변경되지 않습니다.

```cpp
while (!g_pair.compare_exchange_weak(expected, desired,
                                     std::memory_order_acq_rel,
                                     std::memory_order_acquire)) {
    desired = expected;
    desired.a += 1;
}
```

- **CAS 시도:** 공유 값과 `expected`를 비교하고, 일치하면 `desired`로 교체합니다. 비교와 조건부 교체는 하나의 원자적 연산입니다.
- 성공하면 `true`를 반환하므로 반복문을 종료합니다.
- 실패 원인은 값의 불일치 또는 `weak`에서 허용되는 **허위 실패(spurious failure)** 입니다. 허위 실패는 값이 일치해도 발생할 수 있습니다.
- 실패하면 `expected`는 **해당 CAS에서 읽은 값** 으로 갱신됩니다. 이후 다른 스레드가 또 수정할 수 있으므로 “항상 최신값”은 아닙니다.
- `desired`는 자동 갱신되지 않으므로, 갱신된 `expected`를 바탕으로 다시 계산합니다.

- 예를 들어 `a=10`을 읽은 뒤 다른 스레드가 먼저 11로 바꾸면, 내 CAS는 실패하면서 `expected.a=11`을 받습니다. 이후 `desired.a=12`로 재계산하여 다시 시도합니다.

### 📌 3. 결과 보장

- **호출이 성공하여 반환하면**, 성공한 CAS 직전의 `a`에 1을 더한 값을 기록합니다. 그 시점의 `b`는 보존합니다.
- 다른 코드가 `a`를 재설정하지 않고 모든 증가를 이 방식으로 처리하면, 완료된 호출 수만큼 증가합니다.
- 전제는 **`int` 오버플로가 없다는 것** 입니다. 경쟁 상황에서 각 스레드가 일정 시간 안에 성공한다는 보장은 없습니다.
- 성공 직후 다른 스레드가 다시 수정할 수 있으므로, 반환 시에도 같은 값이 남아 있다고 보장하지는 않습니다.

### 📌 4. 메모리 오더 의미

- 최초 load의 `acquire`: 대응되는 release 연산이 공개한 값을 읽는 등 조건이 충족되면, 그 앞선 작업과 동기화합니다.
- 성공 시 `acq_rel`: 읽기에 acquire, 쓰기에 release 역할을 적용합니다.
- 실패 시 `acquire`: 공유 값에 쓰지 않고 읽기만 하며, 이 읽기에 acquire를 적용합니다.

- **acquire는 무조건 최신값을 읽게 하는 옵션이 아닙니다.**
- 동기화는 대응되는 release 연산 또는 그 release sequence의 값을 읽는 등 필요한 조건이 충족되어야 성립합니다.

- Pair 자체의 값만 관리하고 다른 데이터의 공개·동기화에는 사용하지 않는다면, 위 세 위치에 `relaxed`를 사용해도 원자적 비교·교체는 유지됩니다.

### 📌 5. atomic<Pair>를 쓰는 이유

- 구조 전체를 하나의 일관된 스냅샷으로 읽고, 그 값을 기준으로 조건부 교체할 수 있습니다.
- 다른 스레드가 `b`를 바꾸어 비교가 실패하면 새로 관찰한 `b`를 포함해 재계산하므로, 예전 `b`를 그대로 덮어쓰는 문제를 피합니다.
- 단, 다른 코드가 오래된 복사본을 단순 `store()`하면 그 코드에서 갱신 유실이 생길 수 있습니다. 공유 값을 변경하는 코드들이 같은 갱신 규칙을 따라야 합니다.

> `std::atomic<T>`의 일반 템플릿은 trivially copyable 등 타입 요구사항을 만족해야 합니다. 위 `Pair`는 만족합니다. CAS는 `operator==`가 아니라 타입의 표현을 기준으로 비교합니다.

---

### 📌 6. 의사코드로 표현

```text
cur = atomic_load(g_pair)              // 최초 한 번 읽음

loop:
  next = cur                          // desired 재계산
  next.a = cur.a + 1

  if atomic_CAS_weak(g_pair, cur, next):
      break                           // 성공: 공유 값 교체 완료
  else:
      // cur은 실패한 CAS가 읽은 값으로 갱신되어 있음
      goto loop                       // 다시 load하지 않고 재사용
```

실패 후 별도의 load가 필요하지 않은 이유는 **CAS가 `cur`(expected)에 관찰한 값을 돌려주기 때문**입니다.

---

### 📌 7. 대안 설계

- `a`만 독립적으로 증가시키면 `std::atomic<int>`의 `fetch_add(1)`을 사용할 수 있습니다.
- 여러 필드를 하나의 원자적 상태로 읽고 교체해야 하면 구조체 전체에 CAS를 적용할 수 있습니다.
- 복잡한 객체나 여러 작업을 함께 보호해야 하면 mutex를 사용하는 편이 단순할 수 있습니다.

---

### 📌 8. 요약

> **CAS가 성공하는 순간의 `Pair`를 기준으로 `a`를 +1 하고 `b`를 보존합니다.**  
> 실패하면 `expected`에 반환된 관찰값으로 `desired`를 다시 계산하여 재시도합니다. 원자적 갱신은 보장하지만, 항상 최신값을 유지하거나 반드시 lock-free로 동작하는 것은 아닙니다.

---
