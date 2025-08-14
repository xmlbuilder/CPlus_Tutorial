# ⏱️ C++ `std::chrono` 타이머 가이드

## 코드 예제와 설명

```cpp
#include <iostream>
#include <chrono>
#include <thread> // sleep_for 사용 시 필요

int main() {
    // 1) 현재 "달력 시간" (wall clock)
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::cout << tp.time_since_epoch().count() << "\n";

    // time_point + duration  : 2초 뒤 시각
    tp = tp + std::chrono::seconds(2);
    std::cout << tp.time_since_epoch().count() << "\n";

    // 2) 경과 시간 측정에 맞는 "단조 증가" 시계
    auto start = std::chrono::steady_clock::now();
    std::cout << "Start time " << start.time_since_epoch().count() << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto end = std::chrono::steady_clock::now();
    std::cout << "End time " << end.time_since_epoch().count() << "\n";

    // 두 시각의 차이 = duration
    auto timeStep = end - start;

    // 단위를 명시적으로 고정해서 출력(여기선 마이크로초)
    std::cout
        << std::chrono::duration_cast<std::chrono::microseconds>(timeStep).count()
        << "\n";

    return 0;
}
```

### 핵심 포인트
- **`system_clock`**: 실제 날짜/시간(“벽시계”). 시간 변경 시 점프 가능 → 경과 시간 측정엔 부적합.
- **`steady_clock`**: 단조증가(monotonic) 보장. 경과 시간/타임아웃 측정에 추천.
- `time_since_epoch().count()`는 시계의 기본 단위(tick) 값 → 사람이 읽을 단위로 변환하려면 `duration_cast` 사용.

---

## 리터럴 사용 (C++14)
```cpp
using namespace std::chrono_literals;

std::this_thread::sleep_for(750ms);
std::this_thread::sleep_for(2s);
```

---

## 사람이 읽을 수 있는 시간 출력
```cpp
auto now = std::chrono::system_clock::now();
std::time_t t = std::chrono::system_clock::to_time_t(now);
std::cout << std::put_time(std::localtime(&t), "%F %T") << "\n"; // 2025-08-12 10:03:45

// C++20 이상:
std::cout << std::format("{:%F %T}\n", now);
```

---

## 함수 실행 시간 측정 템플릿
```cpp
template <class F, class... Args>
auto measure_us(F&& f, Args&&... args) {
    auto start = std::chrono::steady_clock::now();
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

// 사용
auto us = measure_us([]{
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
});
std::cout << "Elapsed: " << us << " us\n";
```

---

## 스코프 타이머 (RAII)
```cpp
struct ScopeTimer {
    const char* name;
    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
    explicit ScopeTimer(const char* n) : name(n) {}
    ~ScopeTimer() {
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - t0).count();
        std::cout << name << " elapsed: " << us << " us\n";
    }
};

// 사용
{
    ScopeTimer t{"loading"};
    // ... 측정할 코드 ...
}
```

---

## 타임아웃/데드라인 패턴
```cpp
using namespace std::chrono;

auto deadline = steady_clock::now() + 2s;
while (true) {
    // ... 작업 ...
    if (steady_clock::now() >= deadline) break;
}
```

---

## 흔한 실수 체크리스트
- 경과 시간 측정에 `system_clock` 사용 → 시계 점프로 값 틀어짐
- `count()`를 그대로 출력 → 단위 모름 → 반드시 `duration_cast`
- `<thread>` 누락 → `sleep_for` 컴파일 에러
- `high_resolution_clock` 남용 → 의미 모호

---

## 요약
- 경과 시간/성능 측정 → `steady_clock` + `duration_cast`
- 현재 날짜/시간 → `system_clock` (+ `to_time_t` / `format`)
- 가독성 → `2s`, `5ms` 같은 리터럴 사용
- 반복 패턴은 헬퍼/RAII로 정리
