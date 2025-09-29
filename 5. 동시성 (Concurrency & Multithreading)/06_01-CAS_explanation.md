# C++ CAS 예제 상세 분석

다음 코드는 **구조체 `Pair` 전체를 하나의 원자 단위로 취급**하면서, 그 안의 **`a`만 +1** 하려는 **CAS(compare-and-swap) 루프**입니다.

```cpp
struct Pair {
    int a;
    int b;
};

std::atomic<Pair> g_pair{};

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

## 1. 코드가 하려는 일 (요약)

- `g_pair`는 `Pair{a,b}`를 통째로 담고 있는 `std::atomic<Pair>`입니다.  
- 여러 스레드가 동시에 `a`를 증가시키더라도,
  - **`b`는 그대로 유지**하고,
  - **`a`만 정확히 +1** 되도록
- **락 없이(CAS 루프)** 안전하게 갱신합니다.

---

## 2. 동작 흐름

```cpp
Pair expected = g_pair.load(std::memory_order_acquire);
Pair desired  = expected;
desired.a += 1;
```
- 현재 메모리 값을 `expected`로 읽어옵니다.
- 이를 복사해 `desired`를 만들고, `desired.a`만 +1.

```cpp
while (!g_pair.compare_exchange_weak(expected, desired,
                                     std::memory_order_acq_rel,
                                     std::memory_order_acquire)) {
    desired = expected;
    desired.a += 1;
}
```
- **CAS 시도**: `g_pair`가 `expected`와 같으면 `desired`로 교체.
- 성공하면 종료.
- 실패하면:
  1) 다른 스레드가 그 사이에 `g_pair`를 변경했거나,
  2) `weak`에서 발생 가능한 **spurious failure**.
- 실패 시 `expected`는 최신 실제 값으로 자동 갱신 → 이를 기반으로 다시 `desired`를 만들고 재시도.

---

## 3. 결과 보장

- `b`는 항상 최신값이 유지됨(다른 스레드 변경 덮어쓰지 않음).
- `a`는 경쟁이 있어도 정확히 1씩 증가.

---

## 4. 메모리 오더 의미

- 성공 시 `std::memory_order_acq_rel`: 변경 전후로 acquire/release 동기화 보장.
- 실패 시 `std::memory_order_acquire`: `expected`를 최신 값으로 읽을 때 동기화 보장.

---

## 5. atomic<Pair>를 쓰는 이유

- `a`만 바꾸더라도 `b`를 동시에 일관되게 유지해야 할 때.
- 구조 전체의 원자적 스냅샷을 기준으로 수정 → 다른 스레드 변경을 덮어쓰지 않음.

> 전제: `std::atomic<T>`는 **`T`가 trivially copyable**일 때만 가능. (`Pair`는 해당)

---

## 6. 의사코드로 표현

```text
loop:
  cur = atomic_load(g_pair)            // expected
  next = cur                           // desired
  next.a = cur.a + 1                   // b는 그대로
  if atomic_CAS(g_pair, cur, next):    // 같으면 교체
      break
  else:
      // cur이 최신값으로 갱신됨 → 다시 시도
      goto loop
```

---

## 7. 대안 설계

- `a`를 독립 `std::atomic<int>`로 두면 `fetch_add(1)` 가능 → 단, `b`와의 일관성은 별도 보장 필요.
- 여러 필드 동시 원자성 필요 시 → 구조체 통째로 CAS.

---

## 8. 요약

> **CAS 루프를 통해 `Pair`의 `a`를 +1 하되, `b`를 그대로 유지**  
> 실패 시 최신 값으로 다시 계산해 재시도 → 경쟁 속에서도 안전하게 일관성 있는 변경 보장.
