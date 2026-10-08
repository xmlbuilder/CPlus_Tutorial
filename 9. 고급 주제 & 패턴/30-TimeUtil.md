## 📘 날짜 및 시간 처리 — TimeUtil

### 📌 1. 개요

- `TimeUtil`은 Java의 날짜·시간 API와 비슷한 편의성을 목적으로 작성한 C++17 날짜·시간 유틸리티입니다. 
- **범위:** 날짜 유효성 검사, 현재 로컬 시간, 소수 시간, Julian Day, 날짜 간 경과시간, 문자열 변환, 실행 시간 측정.
- Java `java.time`의 시간대·DST 기능 전체를 구현한 것은 아닙니다.

### 📌 2. 자료구조

| 이름 | 역할 | 주요 필드 |
|---|---|---|
| `LocalDateTime` | 날짜 및 시각 저장 | `year`, `month`, `day`, `hour`, `minute`, `second`, `decimal_hours` |
| `ElapsedTime` | Julian Day 기반 실수 경과시간 | `valid`, `negative`, `total_days` 등 |
| `TimeSpan` | 정수 초 기반 경과시간 | `valid`, `negative`, `total_seconds` 등 |
| `TimeUtil` | 정적 날짜·시간 함수 모음 | `Now`, `ElapsedLong`, `Format` 등 |
| `ScopeTimer` | 스코프 실행 시간 자동 출력 | `name`, `t0` |

```cpp
LocalDateTime dt;
dt.year = 2026;
dt.month = 4;
dt.day = 7;
dt.hour = 10;
dt.minute = 20;
dt.second = 30;
dt.decimal_hours = TimeUtil::DecimalHoursFromHMS(dt.hour, dt.minute, dt.second);
```
> `LocalDateTime`은 단순 구조체입니다. 시·분·초를 변경해도 `decimal_hours`는 자동으로 갱신되지 않습니다.

### 📌 3. 날짜 유효성 검사

```cpp
bool leap = TimeUtil::IsLeapYear(2024);        // true
int days = TimeUtil::DaysInMonth(2, 2024);     // 29
bool valid = TimeUtil::IsValidDate(2024, 2, 29); // true
bool timeOK = TimeUtil::IsValidTime(23, 59, 59);
```

- `IsLeapYear`: 4년·100년·400년 규칙으로 윤년 판정
- `DaysInMonth`: 해당 월의 일수 반환. **월이 1~12 범위 밖이면 경계값으로 제한(clamp)**
- `IsValidDate`, `IsValidTime`, `IsValidDateTime`: 입력 값 검증

### 📌 4. 현재 로컬 날짜·시간

```cpp
LocalDateTime now = TimeUtil::Now();
std::cout << TimeUtil::NowString() << '\n';
```

- `Now()`는 `GetCurrentLocalDateTime()`을 호출하며 시스템 로컬 시간을 읽습니다.
- `GetDefaultLocalDateTime()`은 **현재 연도의 3월 21일 12:00:00** 을 반환하도록 구현되어 있습니다.

### 📌 5. 시·분·초와 소수 시간

- 시·분·초를 소수 시간으로 변환하는 식:

```
H = h + (m / 60) + (s / 3600)
```

```cpp
double hours = TimeUtil::DecimalHoursFromHMS(9, 30, 0); // 9.5

int h = 0, m = 0, s = 0;
TimeUtil::DecimalHoursToHMS(14.5, h, m, s); // 14:30:00
```
- 날짜까지 전달하는 오버로드는 반올림으로 자정이 넘어갈 때 날짜를 다음 날로 보정합니다.

```cpp
int y = 2023, mo = 12, d = 31;
TimeUtil::DecimalHoursToHMS(23.99999, h, m, s, y, mo, d);
// 2024-01-01 00:00:00
```

- **구현 주의:** 날짜 없는 오버로드는 반올림으로 `second == 60`이 되면 59로 제한합니다.
- 날짜 포함 오버로드는 다음 분·시간·날짜로 넘깁니다.
- 시간의 24시간 순환은 내부에서 수행하지만, 전달된 날짜를 모든 입력 시간의 일수만큼 보정하는 일반적인 날짜 덧셈 함수는 아닙니다.

### 📌 6. Julian Day 변환

- Julian Day는 날짜와 시간을 연속적인 실수 값으로 표현합니다.

```cpp
double jd = 0.0;
bool ok = TimeUtil::ToJulianDay(2000, 1, 1, 12.0, jd);
// jd = 2451545.0

int year = 0, month = 0, day = 0;
double hours = 0.0;
TimeUtil::FromJulianDay(jd, year, month, day, hours);
```

- `ToJulianDay(const LocalDateTime&, double&)` 오버로드도 있습니다.
- 이 변환은 **시간대 정보 없이 전달된 날짜·시간 필드**를 계산합니다.

### 📌 7. 날짜 간 경과시간

#### 🔹 7.1 Julian Day 기반 — `Elapsed()`

```cpp
LocalDateTime a{2026, 4, 7, 10, 20, 30};
LocalDateTime b{2026, 4, 8, 12, 25, 40};

ElapsedTime e = TimeUtil::Elapsed(a, b);
if (e.valid) {
    std::cout << e.total_seconds << '\n';
}
```

- Julian Day의 차이를 `double`로 계산하므로 부동소수점 오차가 있을 수 있습니다.
- 개별 `days`, `hours`, `minutes`, `seconds`는 반올림한 전체 초를 분해한 값입니다.

#### 🔹 7.2 정수 초 기반 — `ElapsedLong()`

```cpp
TimeSpan e = TimeUtil::ElapsedLong(a, b);
if (e.valid) {
    std::cout << e.total_seconds << '\n'; // 93910
}
```

| 함수 | 내부 계산 | 특징 |
|---|---|---|
| `Elapsed()` | Julian Day 차이 (`double`) | 실수 오차 가능 |
| `ElapsedLong()` | 정수 초 차이 (`long long`) | 초 단위 정수 계산 |

- 두 결과 모두 `negative`로 역방향 여부를 나타내며, `total_*` 및 개별 구성 값은 **차이의 절댓값** 입니다.
- 입력이 유효하지 않으면 `valid == false`입니다.
- `ToLongSeconds()`는 1970-01-01을 기준으로 날짜 필드를 초 단위 숫자로 변환하지만 **시간대 변환을 하지 않습니다**.


### 📌 8. 문자열 포맷

```cpp
LocalDateTime dt{2026, 4, 7, 19, 42, 15};

TimeUtil::Format(dt);            // "2026-04-07 19:42:15"
TimeUtil::FormatForFileName(dt); // "2026-04-07_19-42-15"
TimeUtil::FormatForDb(dt);       // "2026-04-07 19:42:15"
TimeUtil::FormatCompact(dt);     // "20260407_194215"
```

- 현재 `FormatForDb()`는 `Format()`을 그대로 호출합니다.
- 각각 현재 시간을 바로 문자열로 얻는 `NowString()`, `NowFileNameString()`, `NowDbString()`, `NowCompactString()`도 제공합니다.

### 📌 9. 실행 시간 측정 — `std::chrono`

### `ScopeTimer`: RAII 방식

```cpp
void Calculate()
{
    ScopeTimer timer("Calculate");
    // 측정할 코드
} // 소멸자에서 마이크로초 출력
```

- `std::chrono::steady_clock`으로 시작 시간을 저장하고, 스코프 종료 시 소멸자에서 실행 시간을 출력합니다.

#### 🔹 `on_measure_elapsed`: 함수 실행 시간 반환

```cpp
auto microseconds = on_measure_elapsed([] {
    // 측정할 코드
});
```

- 템플릿과 `std::invoke`, `std::forward`를 사용해 전달된 함수를 호출합니다.
- 현재 구현은 **함수의 반환값이 아니라 경과 마이크로초만 반환** 합니다.

> 헤더의 `on_measure_elapsed`는 `std::invoke`와 `std::forward`를 사용하므로, 단독 헤더로 컴파일할 때는 `<functional>` 및 `<utility>` 포함 여부를 확인해야 합니다.


### 📌 10. 기억할 핵심

1. `LocalDateTime`은 시간대 없는 단순 날짜·시간 구조체입니다.
2. `TimeUtil`은 날짜 검사, 변환, 계산, 출력용 **정적 함수 모음** 입니다.
3. 소수 시간은 계산에 편리하지만 반올림 처리가 필요합니다.
4. Julian Day는 실수 기반, `ElapsedLong()`은 정수 초 기반입니다.
5. `ToLongSeconds()`는 시간대 처리가 없으므로 UTC Unix timestamp와 구분합니다.
6. `ScopeTimer`는 RAII, `on_measure_elapsed`는 템플릿 기반 함수 실행 시간 측정입니다.

---
