# C++ `std::atomic` 제대로 이해하기

`atomic`은 **원자적 연산(Atomic Operation)**을 의미합니다.  
원자적 연산은 **중간에 나눠질 수 없고**, 실행되는 동안 **어떤 스레드도 끼어들 수 없습니다**.  
따라서 결과는 **완료** 또는 **미실행** 둘 중 하나뿐입니다.

> C++에서는 `std::atomic<T>`가 원자성을 보장합니다. 이를 통해 락 없이(lock-free 또는 wait-free 가능성) 데이터 레이스 없이 공유 상태를 갱신할 수 있습니다.

---

## 1) 왜 `std::atomic`이 필요한가?

일반 정수에 대해 `++` 연산을 여러 스레드가 동시에 수행하면 **데이터 레이스**가 발생합니다.  
`std::atomic`은 내부적으로 CPU의 원자적 명령(예: `LOCK XADD`) 또는 컴파일러/라이브러리의 적절한 기법으로 이를 방지합니다.

- **장점**: 락 오버헤드가 없을 수 있음, 간결함
- **주의점**: **메모리 순서(memory order)**를 이해하고 적절히 사용해야 함

---

## 2) 기본 예제 (수정·보강 버전)

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

void worker(std::atomic<int>& counter) {
    for (int i = 0; i < 100'000; ++i) {
        ++counter;
    }
}

int main() {
    std::atomic<int> counter{0};

    std::vector<std::thread> workers;
    workers.reserve(100);

    for (int i = 0; i < 100; ++i) {
        workers.emplace_back(worker, std::ref(counter));
    }
    for (auto& t : workers) t.join();

    std::cout << "Counter result: " << counter.load() << '\n';
}
```

### 개선 포인트
- `<thread>` 포함
- 출력 시 `.load()` 사용
- 필요 시 `fetch_add(1, memory_order_relaxed)`로 최적화

---

## 3) 메모리 모델과 순서(memory order)

`std::atomic`의 핵심은 **원자성** + **메모리 가시성/순서**입니다. 기본은 `memory_order_seq_cst`.

- `memory_order_relaxed` : 원자성만 보장
- `memory_order_acquire` / `memory_order_release` : 한 방향 동기화
- `memory_order_seq_cst` : 가장 강력, 가장 느림

---

## 4) 성능 팁: 경쟁 줄이기

- 스레드 로컬 누적 후 합산
- 배치 업데이트
- false-sharing 방지

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::atomic<long long> global{0};
    const int N = 100;
    const int ITER = 1'000'000;

    std::vector<std::thread> ths;
    ths.reserve(N);

    for (int t = 0; t < N; ++t) {
        ths.emplace_back([&]{
            long long local = 0;
            for (int i = 0; i < ITER; ++i) ++local;
            global.fetch_add(local, std::memory_order_relaxed);
        });
    }
    for (auto& th : ths) th.join();

    std::cout << "global=" << global.load() << '\n';
}
```

---

## 5) CAS 예제

```cpp
#include <atomic>

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

## 6) atomic_flag로 스핀락 만들기

```cpp
#include <atomic>

class SpinLock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {}
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};
```

---

## 7) 요약 체크리스트

- 단순 카운터 → `fetch_add(1, relaxed)`
- 값 읽기 → `load()`
- 순서 필요 시 acquire/release
- 경쟁 심하면 로컬 누적
- 복합 상태 → CAS 또는 락
