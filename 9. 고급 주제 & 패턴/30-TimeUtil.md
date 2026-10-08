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

### 📌 소스 코드
```cpp
#pragma once
#include <ctime>
#include <cmath>
#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

struct LocalDateTime
{
    int year = 1970;
    int month = 1;
    int day = 1;

    int hour = 0;
    int minute = 0;
    int second = 0;

    double decimal_hours = 0.0;
};

struct ElapsedTime
{
    bool valid = false;
    bool negative = false;

    double total_days = 0.0;
    double total_hours = 0.0;
    double total_minutes = 0.0;
    double total_seconds = 0.0;

    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
};

struct TimeSpan
{
    bool valid = false;
    bool negative = false;

    long long total_seconds = 0;
    long long total_minutes = 0;
    long long total_hours = 0;
    long long total_days = 0;

    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
};

class TimeUtil
{
public:
    static bool IsLeapYear(int year);

    static int DaysInMonth(int month, int year);

    static bool IsValidDate(int year, int month, int day);
    static bool IsValidTime(int hour, int minute, int second);
    static bool IsValidDateTime(int year, int month, int day, int hour, int minute, int second);

    // HMS -> decimal hours
    static double DecimalHoursFromHMS(int hour, int minute, int second);

    // decimal hours -> HMS
    static void DecimalHoursToHMS(double hours, int& hour, int& minute, int& second);

    static void DecimalHoursToHMS(
        double hours,
        int& hour,
        int& minute,
        int& second,
        int& year,
        int& month,
        int& day);

    static LocalDateTime GetCurrentLocalDateTime();

    static LocalDateTime GetDefaultLocalDateTime();

    static bool ToJulianDay(int year, int month, int day, double hours, double& julian_day);
    static bool FromJulianDay(double julian_day, int& year, int& month, int& day, double& hours);

    static bool ToJulianDay(const LocalDateTime& dt, double& julian_day);

    static ElapsedTime Elapsed(const LocalDateTime& start, const LocalDateTime& end);

    static bool ToLongSeconds(const LocalDateTime& dt, long long& out_seconds);
    static TimeSpan ElapsedLong(const LocalDateTime& start, const LocalDateTime& end);

    static LocalDateTime Now();
    static std::string NowString();
    static std::string Format(const LocalDateTime& dt);
    static std::string FormatForFileName(const LocalDateTime& dt);
    static std::string FormatForDb(const LocalDateTime& dt);
    static std::string FormatCompact(const LocalDateTime& dt);
    static std::string NowFileNameString();
    static std::string NowDbString();
    static std::string NowCompactString();

private:
    static double IntPart(double x);
    static double FracPart(double x);
    static void NormalizeDateForward(int& year, int& month, int& day);
};

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

template <class F, class... Args>
auto on_measure_elapsed(F&& f, Args&&... args) {
    auto start = std::chrono::steady_clock::now();
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}
```
```cpp
#include "utils_headers.h"


namespace
{
    double on_int_part(double x)
    {
        return x < 0.0 ? std::ceil(x) : std::floor(x);
    }

    double on_frac_part(double x)
    {
        return x - on_int_part(x);
    }

    void on_decimal_hours_to_hms_raw(double hours, int& hour, int& minute, int& second)
    {
        while (hours >= 24.0)
            hours -= 24.0;

        while (hours < 0.0)
            hours += 24.0;

        hour = static_cast<int>(hours);

        const double mins = (hours - hour) * 60.0;
        minute = static_cast<int>(mins);

        const double secs = (mins - minute) * 60.0;
        second = static_cast<int>(secs + 0.5); // 반올림
    }
}

double TimeUtil::IntPart(const double x)
{
    return on_int_part(x);
}

double TimeUtil::FracPart(const double x)
{
    return on_frac_part(x);
}

bool TimeUtil::IsLeapYear(const int year)
{
    if (0 != year % 4)
        return false;

    if (0 == year % 100 && 0 != year % 400)
        return false;

    return true;
}

int TimeUtil::DaysInMonth(int month, const int year)
{
    month = (std::max)(1, (std::min)(12, month));

    if (month == 2 && IsLeapYear(year))
        return 29;

    static const int days[13] =
    {
        0,
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    return days[month];
}

bool TimeUtil::IsValidDate(const int year, const int month, const int day)
{
    if (month < 1 || month > 12)
        return false;

    if (day < 1 || day > DaysInMonth(month, year))
        return false;

    return true;
}

bool TimeUtil::IsValidTime(int hour, int minute, int second)
{
    if (hour < 0 || hour > 23)
        return false;

    if (minute < 0 || minute > 59)
        return false;

    if (second < 0 || second > 59)
        return false;

    return true;
}

bool TimeUtil::IsValidDateTime(int year, int month, int day, int hour, int minute, int second)
{
    return IsValidDate(year, month, day) && IsValidTime(hour, minute, second);
}

double TimeUtil::DecimalHoursFromHMS(int hour, int minute, int second)
{
    return static_cast<double>(hour)
         + static_cast<double>(minute) / 60.0
         + static_cast<double>(second) / 3600.0;
}

void TimeUtil::DecimalHoursToHMS(double hours, int& hour, int& minute, int& second)
{
    on_decimal_hours_to_hms_raw(hours, hour, minute, second);

    if (second > 59)
        second = 59;
}

void TimeUtil::NormalizeDateForward(int& year, int& month, int& day)
{
    while (day > DaysInMonth(month, year))
    {
        day = 1;
        month++;

        if (month > 12)
        {
            month = 1;
            year++;
        }
    }
}

void TimeUtil::DecimalHoursToHMS(
    double hours,
    int& hour,
    int& minute,
    int& second,
    int& year,
    int& month,
    int& day)
{
    on_decimal_hours_to_hms_raw(hours, hour, minute, second);

    if (second > 59)
    {
        second = 0;
        minute++;

        if (minute > 59)
        {
            minute = 0;
            hour++;

            if (hour > 23)
            {
                hour = 0;
                day++;
                NormalizeDateForward(year, month, day);
            }
        }
    }
}

LocalDateTime TimeUtil::GetCurrentLocalDateTime()
{
    LocalDateTime dt;

    const std::time_t now = std::time(nullptr);
    std::tm local_tm{};

#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    local_tm = *std::localtime(&now);
#endif

    dt.year = local_tm.tm_year + 1900;
    dt.month = local_tm.tm_mon + 1;
    dt.day = local_tm.tm_mday;

    dt.hour = local_tm.tm_hour;
    dt.minute = local_tm.tm_min;
    dt.second = local_tm.tm_sec;

    dt.decimal_hours = DecimalHoursFromHMS(dt.hour, dt.minute, dt.second);
    return dt;
}

LocalDateTime TimeUtil::GetDefaultLocalDateTime()
{
    LocalDateTime now = GetCurrentLocalDateTime();

    LocalDateTime dt;
    dt.year = now.year;
    dt.month = 3;
    dt.day = 21;
    dt.hour = 12;
    dt.minute = 0;
    dt.second = 0;
    dt.decimal_hours = 12.0;

    return dt;
}

bool TimeUtil::ToJulianDay(int year, int month, int day, double hours, double& julian_day)
{
    if (!IsValidDate(year, month, day))
        return false;

    if (hours < 0.0 || hours > 24.0)
        return false;

    int y = year;
    int m = month;

    if (m < 3)
    {
        m += 12;
        y--;
    }

    const int a = y / 100;
    const int b = 2 - a + (a / 4);

    const int julian_day_int =
        36525 * (y + 4716) / 100 +
        306 * (m + 1) / 10 +
        day + b - 1524;

    julian_day = static_cast<double>(julian_day_int) + hours / 24.0 - 0.5;
    return true;
}

bool TimeUtil::FromJulianDay(double julian_day, int& year, int& month, int& day, double& hours)
{
    const double jd = julian_day + 0.5;

    int b = static_cast<int>(IntPart(jd));
    const int a = (b * 100 - 186721625) / 3652425;

    b += 1 + a - (a / 4) + 1524;

    const int c = (b * 100 - 12210) / 36525;
    const int d = 365 * c + c / 4;
    const int e = (10000 * (b - d)) / 306001;

    day = b - d - 306001 * e / 10000;
    month = e < 14 ? e - 1 : e - 13;
    year = month > 2 ? c - 4716 : c - 4715;

    hours = FracPart(jd) * 24.0 + 1e-8;

    return IsValidDate(year, month, day);
}

bool TimeUtil::ToJulianDay(const LocalDateTime& dt, double& julian_day)
{
    if (!IsValidDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second))
        return false;

    const double hours = DecimalHoursFromHMS(dt.hour, dt.minute, dt.second);
    return ToJulianDay(dt.year, dt.month, dt.day, hours, julian_day);
}

ElapsedTime TimeUtil::Elapsed(const LocalDateTime& start, const LocalDateTime& end)
{
    ElapsedTime out;

    double jd_start = 0.0;
    double jd_end = 0.0;

    if (!ToJulianDay(start, jd_start))
        return out;

    if (!ToJulianDay(end, jd_end))
        return out;

    double delta_days = jd_end - jd_start;
    out.valid = true;

    if (delta_days < 0.0)
    {
        out.negative = true;
        delta_days = -delta_days;
    }

    out.total_days = delta_days;
    out.total_hours = delta_days * 24.0;
    out.total_minutes = out.total_hours * 60.0;
    out.total_seconds = out.total_minutes * 60.0;

    auto total_seconds_rounded = std::llround(out.total_seconds);

    out.days = static_cast<int>(total_seconds_rounded / 86400);
    total_seconds_rounded %= 86400;

    out.hours = static_cast<int>(total_seconds_rounded / 3600);
    total_seconds_rounded %= 3600;

    out.minutes = static_cast<int>(total_seconds_rounded / 60);
    total_seconds_rounded %= 60;

    out.seconds = static_cast<int>(total_seconds_rounded);

    return out;
}

static int DaysFromCivil(int y, unsigned m, unsigned d)
{
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
}

bool TimeUtil::ToLongSeconds(const LocalDateTime& dt, long long& out_seconds)
{
    if (!IsValidDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second))
        return false;

    const int days = DaysFromCivil(dt.year,
       static_cast<unsigned>(dt.month),
       static_cast<unsigned>(dt.day));

    out_seconds =
        static_cast<long long>(days) * 86400LL +
        static_cast<long long>(dt.hour) * 3600LL +
        static_cast<long long>(dt.minute) * 60LL +
        static_cast<long long>(dt.second);

    return true;
}

TimeSpan TimeUtil::ElapsedLong(const LocalDateTime& start, const LocalDateTime& end)
{
    TimeSpan out;

    long long s0 = 0;
    long long s1 = 0;

    if (!ToLongSeconds(start, s0))
        return out;

    if (!ToLongSeconds(end, s1))
        return out;

    long long delta = s1 - s0;

    out.valid = true;
    if (delta < 0)
    {
        out.negative = true;
        delta = -delta;
    }

    out.total_seconds = delta;
    out.total_minutes = delta / 60;
    out.total_hours   = delta / 3600;
    out.total_days    = delta / 86400;

    out.days = static_cast<int>(delta / 86400);
    delta %= 86400;

    out.hours = static_cast<int>(delta / 3600);
    delta %= 3600;

    out.minutes = static_cast<int>(delta / 60);
    delta %= 60;

    out.seconds = static_cast<int>(delta);

    return out;
}

LocalDateTime TimeUtil::Now()
{
    return GetCurrentLocalDateTime();
}

std::string TimeUtil::Format(const LocalDateTime& dt)
{
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << dt.year << "-"
        << std::setw(2) << dt.month << "-"
        << std::setw(2) << dt.day << " "
        << std::setw(2) << dt.hour << ":"
        << std::setw(2) << dt.minute << ":"
        << std::setw(2) << dt.second;
    return oss.str();
}

std::string TimeUtil::FormatForFileName(const LocalDateTime& dt)
{
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << dt.year << "-"
        << std::setw(2) << dt.month << "-"
        << std::setw(2) << dt.day << "_"
        << std::setw(2) << dt.hour << "-"
        << std::setw(2) << dt.minute << "-"
        << std::setw(2) << dt.second;
    return oss.str();
}

std::string TimeUtil::FormatForDb(const LocalDateTime& dt)
{
    return Format(dt);
}

std::string TimeUtil::FormatCompact(const LocalDateTime& dt)
{
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << dt.year
        << std::setw(2) << dt.month
        << std::setw(2) << dt.day << "_"
        << std::setw(2) << dt.hour
        << std::setw(2) << dt.minute
        << std::setw(2) << dt.second;
    return oss.str();
}

std::string TimeUtil::NowString()
{
    return Format(Now());
}

std::string TimeUtil::NowFileNameString()
{
    return FormatForFileName(Now());
}

std::string TimeUtil::NowDbString()
{
    return FormatForDb(Now());
}

std::string TimeUtil::NowCompactString()
{
    return FormatCompact(Now());
}
```
### 📌 테스트 코드

```cpp
#include "utils_headers.h"
#include "test_time_utils_code.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#if defined(_MSC_VER)
#include <windows.h>
#endif


namespace
{
    static int g_test_pass = 0;
    static int g_test_fail = 0;

    static bool NearlyEqual(double a, double b, double eps = 1e-9)
    {
        return std::fabs(a - b) <= eps;
    }

    static std::string BoolToString(bool b)
    {
        return b ? "true" : "false";
    }

    static void PrintHeader(const std::string& title)
    {
        std::cout << "\n============================================================\n";
        std::cout << title << "\n";
        std::cout << "============================================================\n";
    }

    static void PrintSubHeader(const std::string& title)
    {
        std::cout << "\n------------------------------------------------------------\n";
        std::cout << title << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    static void Pass(const std::string& name)
    {
        ++g_test_pass;
        std::cout << "[PASS] " << name << "\n";
    }

    static void Fail(const std::string& name, const std::string& detail = "")
    {
        ++g_test_fail;
        std::cout << "[FAIL] " << name;
        if (!detail.empty())
            std::cout << " : " << detail;
        std::cout << "\n";
    }

    static void ExpectTrue(const std::string& name, bool actual)
    {
        if (actual)
            Pass(name);
        else
            Fail(name, "expected=true actual=false");
    }

    static void ExpectFalse(const std::string& name, bool actual)
    {
        if (!actual)
            Pass(name);
        else
            Fail(name, "expected=false actual=true");
    }

    static void ExpectInt(const std::string& name, int actual, int expected)
    {
        if (actual == expected)
        {
            Pass(name);
        }
        else
        {
            std::ostringstream oss;
            oss << "expected=" << expected << " actual=" << actual;
            Fail(name, oss.str());
        }
    }

    static void ExpectDouble(const std::string& name, double actual, double expected, double eps = 1e-9)
    {
        if (NearlyEqual(actual, expected, eps))
        {
            Pass(name);
        }
        else
        {
            std::ostringstream oss;
            oss << std::setprecision(17)
                << "expected=" << expected << " actual=" << actual
                << " eps=" << eps;
            Fail(name, oss.str());
        }
    }

    static void PrintDateTime(
        const std::string& prefix,
        int year, int month, int day,
        int hour, int minute, int second)
    {
        std::cout << prefix
                  << year << "-"
                  << std::setw(2) << std::setfill('0') << month << "-"
                  << std::setw(2) << std::setfill('0') << day << " "
                  << std::setw(2) << std::setfill('0') << hour << ":"
                  << std::setw(2) << std::setfill('0') << minute << ":"
                  << std::setw(2) << std::setfill('0') << second
                  << std::setfill(' ')
                  << "\n";
    }

    static void PrintDateTimeHours(
        const std::string& prefix,
        int year, int month, int day,
        double hours)
    {
        std::cout << prefix
                  << year << "-"
                  << std::setw(2) << std::setfill('0') << month << "-"
                  << std::setw(2) << std::setfill('0') << day
                  << "  decimal_hours=" << std::setprecision(15) << hours
                  << std::setfill(' ')
                  << "\n";
    }
}

static void Test_IsLeapYear()
{
    PrintSubHeader("Test_IsLeapYear");

    ExpectTrue ("Leap year 2000", TimeUtil::IsLeapYear(2000));
    ExpectFalse("Non-leap century 1900", TimeUtil::IsLeapYear(1900));
    ExpectTrue ("Leap year 2024", TimeUtil::IsLeapYear(2024));
    ExpectFalse("Non-leap year 2023", TimeUtil::IsLeapYear(2023));
}

static void Test_DaysInMonth()
{
    PrintSubHeader("Test_DaysInMonth");

    ExpectInt("Jan 2023", TimeUtil::DaysInMonth(1, 2023), 31);
    ExpectInt("Feb 2023", TimeUtil::DaysInMonth(2, 2023), 28);
    ExpectInt("Feb 2024", TimeUtil::DaysInMonth(2, 2024), 29);
    ExpectInt("Apr 2023", TimeUtil::DaysInMonth(4, 2023), 30);
    ExpectInt("Dec 2023", TimeUtil::DaysInMonth(12, 2023), 31);

    // clamp 테스트
    ExpectInt("Clamp month 0 -> Jan",  TimeUtil::DaysInMonth(0, 2023), 31);
    ExpectInt("Clamp month 13 -> Dec", TimeUtil::DaysInMonth(13, 2023), 31);
}

static void Test_IsValidDate()
{
    PrintSubHeader("Test_IsValidDate");

    ExpectTrue ("Valid date 2024-02-29", TimeUtil::IsValidDate(2024, 2, 29));
    ExpectFalse("Invalid date 2023-02-29", TimeUtil::IsValidDate(2023, 2, 29));
    ExpectTrue ("Valid date 2023-04-30", TimeUtil::IsValidDate(2023, 4, 30));
    ExpectFalse("Invalid date 2023-04-31", TimeUtil::IsValidDate(2023, 4, 31));
    ExpectFalse("Invalid month 0", TimeUtil::IsValidDate(2023, 0, 10));
    ExpectFalse("Invalid month 13", TimeUtil::IsValidDate(2023, 13, 10));
    ExpectFalse("Invalid day 0", TimeUtil::IsValidDate(2023, 1, 0));
}

static void Test_IsValidTime()
{
    PrintSubHeader("Test_IsValidTime");

    ExpectTrue ("Valid time 00:00:00", TimeUtil::IsValidTime(0, 0, 0));
    ExpectTrue ("Valid time 23:59:59", TimeUtil::IsValidTime(23, 59, 59));
    ExpectFalse("Invalid hour -1", TimeUtil::IsValidTime(-1, 0, 0));
    ExpectFalse("Invalid hour 24", TimeUtil::IsValidTime(24, 0, 0));
    ExpectFalse("Invalid minute 60", TimeUtil::IsValidTime(12, 60, 0));
    ExpectFalse("Invalid second 60", TimeUtil::IsValidTime(12, 0, 60));
}

static void Test_IsValidDateTime()
{
    PrintSubHeader("Test_IsValidDateTime");

    ExpectTrue ("Valid datetime", TimeUtil::IsValidDateTime(2024, 2, 29, 23, 59, 59));
    ExpectFalse("Invalid date in datetime", TimeUtil::IsValidDateTime(2023, 2, 29, 23, 59, 59));
    ExpectFalse("Invalid time in datetime", TimeUtil::IsValidDateTime(2024, 2, 29, 24, 0, 0));
}

static void Test_DecimalHoursFromHMS()
{
    PrintSubHeader("Test_DecimalHoursFromHMS");

    ExpectDouble("0:0:0 -> 0.0", TimeUtil::DecimalHoursFromHMS(0, 0, 0), 0.0);
    ExpectDouble("9:30:0 -> 9.5", TimeUtil::DecimalHoursFromHMS(9, 30, 0), 9.5);
    ExpectDouble("1:15:30", TimeUtil::DecimalHoursFromHMS(1, 15, 30), 1.2583333333333333, 1e-12);
    ExpectDouble("23:59:59", TimeUtil::DecimalHoursFromHMS(23, 59, 59), 23.999722222222221, 1e-12);
}

static void Test_DecimalHoursToHMS_Basic()
{
    PrintSubHeader("Test_DecimalHoursToHMS_Basic");

    int h = 0, m = 0, s = 0;

    TimeUtil::DecimalHoursToHMS(0.0, h, m, s);
    ExpectInt("0.0 -> hour", h, 0);
    ExpectInt("0.0 -> minute", m, 0);
    ExpectInt("0.0 -> second", s, 0);

    TimeUtil::DecimalHoursToHMS(9.5, h, m, s);
    ExpectInt("9.5 -> hour", h, 9);
    ExpectInt("9.5 -> minute", m, 30);
    ExpectInt("9.5 -> second", s, 0);

    TimeUtil::DecimalHoursToHMS(1.2583333333333333, h, m, s);
    ExpectInt("1.258333.. -> hour", h, 1);
    ExpectInt("1.258333.. -> minute", m, 15);
    ExpectInt("1.258333.. -> second", s, 30);

    // 24시간 넘는 값은 순환
    TimeUtil::DecimalHoursToHMS(25.5, h, m, s);
    ExpectInt("25.5 -> hour", h, 1);
    ExpectInt("25.5 -> minute", m, 30);
    ExpectInt("25.5 -> second", s, 0);

    // 음수도 순환
    TimeUtil::DecimalHoursToHMS(-0.5, h, m, s);
    ExpectInt("-0.5 -> hour", h, 23);
    ExpectInt("-0.5 -> minute", m, 30);
    ExpectInt("-0.5 -> second", s, 0);
}

static void Test_DecimalHoursToHMS_Rounding()
{
    PrintSubHeader("Test_DecimalHoursToHMS_Rounding");

    int h = 0, m = 0, s = 0;

    // second 가 60으로 튈 수 있는 케이스를 막는 단순 버전
    TimeUtil::DecimalHoursToHMS(23.99999, h, m, s);

    std::cout << "Input 23.99999 -> " << h << ":" << m << ":" << s << "\n";

    ExpectTrue("Basic overload second valid range", s >= 0 && s <= 59);
    ExpectTrue("Basic overload minute valid range", m >= 0 && m <= 59);
    ExpectTrue("Basic overload hour valid range", h >= 0 && h <= 23);
}

static void Test_DecimalHoursToHMS_WithDateAdjust()
{
    PrintSubHeader("Test_DecimalHoursToHMS_WithDateAdjust");

    {
        int y = 2026, mo = 4, d = 7;
        int h = 0, mi = 0, s = 0;

        TimeUtil::DecimalHoursToHMS(23.99999, h, mi, s, y, mo, d);

        PrintDateTime("23.99999 adjusted -> ", y, mo, d, h, mi, s);

        ExpectInt("Adjusted hour", h, 0);
        ExpectInt("Adjusted minute", mi, 0);
        ExpectInt("Adjusted second", s, 0);
        ExpectInt("Adjusted year same", y, 2026);
        ExpectInt("Adjusted month same", mo, 4);
        ExpectInt("Adjusted day +1", d, 8);
    }

    {
        int y = 2024, mo = 2, d = 28;
        int h = 0, mi = 0, s = 0;

        TimeUtil::DecimalHoursToHMS(23.99999, h, mi, s, y, mo, d);

        PrintDateTime("Leap day rollover -> ", y, mo, d, h, mi, s);

        ExpectInt("Leap rollover year", y, 2024);
        ExpectInt("Leap rollover month", mo, 2);
        ExpectInt("Leap rollover day", d, 29);
    }

    {
        int y = 2023, mo = 12, d = 31;
        int h = 0, mi = 0, s = 0;

        TimeUtil::DecimalHoursToHMS(23.99999, h, mi, s, y, mo, d);

        PrintDateTime("Year rollover -> ", y, mo, d, h, mi, s);

        ExpectInt("Year rollover year", y, 2024);
        ExpectInt("Year rollover month", mo, 1);
        ExpectInt("Year rollover day", d, 1);
    }
}

static void Test_RoundTrip_HMS_DecimalHours()
{
    PrintSubHeader("Test_RoundTrip_HMS_DecimalHours");

    struct Case
    {
        int h, m, s;
    };

    const std::vector<Case> cases =
    {
        {0, 0, 0},
        {1, 2, 3},
        {9, 30, 0},
        {12, 34, 56},
        {23, 59, 59}
    };

    for (const auto& c : cases)
    {
        const double dh = TimeUtil::DecimalHoursFromHMS(c.h, c.m, c.s);

        int hh = 0, mm = 0, ss = 0;
        TimeUtil::DecimalHoursToHMS(dh, hh, mm, ss);

        std::ostringstream name;
        name << "RoundTrip " << c.h << ":" << c.m << ":" << c.s;

        if (hh == c.h && mm == c.m && ss == c.s)
        {
            Pass(name.str());
        }
        else
        {
            std::ostringstream oss;
            oss << "expected="
                << c.h << ":" << c.m << ":" << c.s
                << " actual="
                << hh << ":" << mm << ":" << ss
                << " decimal=" << std::setprecision(15) << dh;
            Fail(name.str(), oss.str());
        }
    }
}

static void Test_GetCurrentLocalDateTime()
{
    PrintSubHeader("Test_GetCurrentLocalDateTime");

    const LocalDateTime now = TimeUtil::GetCurrentLocalDateTime();

    PrintDateTime("Current local time -> ",
                  now.year, now.month, now.day,
                  now.hour, now.minute, now.second);

    std::cout << "decimal_hours = " << std::setprecision(15) << now.decimal_hours << "\n";

    ExpectTrue("Current date valid", TimeUtil::IsValidDate(now.year, now.month, now.day));
    ExpectTrue("Current time valid", TimeUtil::IsValidTime(now.hour, now.minute, now.second));

    const double expected = TimeUtil::DecimalHoursFromHMS(now.hour, now.minute, now.second);
    ExpectDouble("Current decimal_hours consistency", now.decimal_hours, expected, 1e-9);
}

static void Test_GetDefaultLocalDateTime()
{
    PrintSubHeader("Test_GetDefaultLocalDateTime");

    const LocalDateTime dt = TimeUtil::GetDefaultLocalDateTime();

    PrintDateTime("Default local time -> ",
                  dt.year, dt.month, dt.day,
                  dt.hour, dt.minute, dt.second);

    std::cout << "decimal_hours = " << std::setprecision(15) << dt.decimal_hours << "\n";

    ExpectInt("Default month", dt.month, 3);
    ExpectInt("Default day", dt.day, 21);
    ExpectInt("Default hour", dt.hour, 12);
    ExpectInt("Default minute", dt.minute, 0);
    ExpectInt("Default second", dt.second, 0);
    ExpectDouble("Default decimal_hours", dt.decimal_hours, 12.0);
}

static void Test_ToJulianDay_Basic()
{
    PrintSubHeader("Test_ToJulianDay_Basic");

    double jd = 0.0;

    ExpectTrue("ToJulianDay valid date", TimeUtil::ToJulianDay(2026, 4, 7, 19.5, jd));
    std::cout << "JD(2026-04-07 19.5h) = " << std::setprecision(15) << jd << "\n";

    ExpectFalse("ToJulianDay invalid date", TimeUtil::ToJulianDay(2023, 2, 29, 12.0, jd));
    ExpectFalse("ToJulianDay invalid hours negative", TimeUtil::ToJulianDay(2026, 4, 7, -1.0, jd));
    ExpectFalse("ToJulianDay invalid hours >24", TimeUtil::ToJulianDay(2026, 4, 7, 24.5, jd));
}

static void Test_JulianDay_RoundTrip()
{
    PrintSubHeader("Test_JulianDay_RoundTrip");

    struct Case
    {
        int y, m, d;
        double hours;
    };

    const std::vector<Case> cases =
    {
        {2026, 4, 7, 0.0},
        {2026, 4, 7, 12.0},
        {2026, 4, 7, 19.5},
        {2024, 2, 29, 23.75},
        {2000, 1, 1, 6.123456789}
    };

    for (const auto& c : cases)
    {
        double jd = 0.0;
        const bool ok1 = TimeUtil::ToJulianDay(c.y, c.m, c.d, c.hours, jd);

        if (!ok1)
        {
            std::ostringstream name;
            name << "Julian round-trip prepare " << c.y << "-" << c.m << "-" << c.d;
            Fail(name.str(), "ToJulianDay failed unexpectedly");
            continue;
        }

        int y2 = 0, m2 = 0, d2 = 0;
        double h2 = 0.0;
        const bool ok2 = TimeUtil::FromJulianDay(jd, y2, m2, d2, h2);

        std::ostringstream label;
        label << "RoundTrip JD " << c.y << "-" << c.m << "-" << c.d << " h=" << c.hours;

        if (!ok2)
        {
            Fail(label.str(), "FromJulianDay failed unexpectedly");
            continue;
        }

        PrintDateTimeHours("Input  -> ", c.y, c.m, c.d, c.hours);
        std::cout << "JD      -> " << std::setprecision(15) << jd << "\n";
        PrintDateTimeHours("Output -> ", y2, m2, d2, h2);

        bool same = true;
        if (y2 != c.y) same = false;
        if (m2 != c.m) same = false;
        if (d2 != c.d) same = false;
        if (!NearlyEqual(h2, c.hours, 1e-7)) same = false;

        if (same)
            Pass(label.str());
        else
        {
            std::ostringstream oss;
            oss << "expected="
                << c.y << "-" << c.m << "-" << c.d << " h=" << std::setprecision(15) << c.hours
                << " actual="
                << y2 << "-" << m2 << "-" << d2 << " h=" << std::setprecision(15) << h2;
            Fail(label.str(), oss.str());
        }
    }
}

static void Test_KnownJulianDayReference()
{
    PrintSubHeader("Test_KnownJulianDayReference");

    // 천문학에서 잘 알려진 기준:
    // 2000-01-01 12:00:00 UT == JD 2451545.0
    double jd = 0.0;
    const bool ok = TimeUtil::ToJulianDay(2000, 1, 1, 12.0, jd);

    ExpectTrue("Reference JD conversion success", ok);
    if (ok)
    {
        std::cout << "Reference JD = " << std::setprecision(15) << jd << "\n";
        ExpectDouble("2000-01-01 12:00 -> JD 2451545.0", jd, 2451545.0, 1e-9);
    }

    int y = 0, m = 0, d = 0;
    double h = 0.0;
    const bool ok2 = TimeUtil::FromJulianDay(2451545.0, y, m, d, h);

    ExpectTrue("Reference JD reverse success", ok2);
    if (ok2)
    {
        PrintDateTimeHours("Reference reverse -> ", y, m, d, h);

        ExpectInt("Reference reverse year", y, 2000);
        ExpectInt("Reference reverse month", m, 1);
        ExpectInt("Reference reverse day", d, 1);
        ExpectDouble("Reference reverse hours", h, 12.0, 1e-7);
    }
}

static void Tutorial_UsageExamples()
{
    PrintSubHeader("Tutorial_UsageExamples");

    std::cout << "[Example 1] UI 입력 9:30:15 를 내부 실수 시간으로 바꾸기\n";
    {
        const double dh = TimeUtil::DecimalHoursFromHMS(9, 30, 15);
        std::cout << "9:30:15 -> decimal hours = " << std::setprecision(15) << dh << "\n";
    }

    std::cout << "\n[Example 2] 내부 decimal hours 를 다시 UI 표시용 시:분:초로 바꾸기\n";
    {
        int h = 0, m = 0, s = 0;
        TimeUtil::DecimalHoursToHMS(14.7569444444444, h, m, s);
        std::cout << "14.7569444444444 -> " << h << ":" << m << ":" << s << "\n";
    }

    std::cout << "\n[Example 3] 날짜 + decimal hours 를 Julian Day 로 바꿔 보관하기\n";
    {
        double jd = 0.0;
        if (TimeUtil::ToJulianDay(2026, 4, 7, 19.5, jd))
        {
            std::cout << "2026-04-07 19.5h -> JD = " << std::setprecision(15) << jd << "\n";
        }
        else
        {
            std::cout << "Julian Day conversion failed\n";
        }
    }

    std::cout << "\n[Example 4] Julian Day 를 다시 날짜/시간으로 복원하기\n";
    {
        int y = 0, m = 0, d = 0;
        double hours = 0.0;

        if (TimeUtil::FromJulianDay(2451545.0, y, m, d, hours))
        {
            std::cout << "JD 2451545.0 -> "
                      << y << "-" << m << "-" << d
                      << " decimal hours = " << std::setprecision(15) << hours << "\n";

            int hh = 0, mm = 0, ss = 0;
            TimeUtil::DecimalHoursToHMS(hours, hh, mm, ss);
            std::cout << "Converted HMS -> " << hh << ":" << mm << ":" << ss << "\n";
        }
    }

    std::cout << "\n[Example 5] 현재 로컬 시간 얻기\n";
    {
        const LocalDateTime now = TimeUtil::GetCurrentLocalDateTime();
        PrintDateTime("Now -> ",
                      now.year, now.month, now.day,
                      now.hour, now.minute, now.second);
        std::cout << "decimal_hours = " << std::setprecision(15) << now.decimal_hours << "\n";
    }

    std::cout << "\n[Example 6] 23.99999 처럼 반올림으로 다음 날이 될 수 있는 값 처리\n";
    {
        int y = 2026, mo = 4, d = 7;
        int h = 0, mi = 0, s = 0;

        TimeUtil::DecimalHoursToHMS(23.99999, h, mi, s, y, mo, d);

        PrintDateTime("Adjusted -> ", y, mo, d, h, mi, s);
        std::cout << "이 overload 는 날짜까지 같이 보정할 때 사용합니다.\n";
    }
}


static void Test_Elapsed_Basic()
{
    PrintSubHeader("Test_Elapsed_Basic");

    LocalDateTime a;
    a.year = 2026; a.month = 4; a.day = 7;
    a.hour = 10; a.minute = 20; a.second = 30;
    a.decimal_hours = TimeUtil::DecimalHoursFromHMS(a.hour, a.minute, a.second);

    LocalDateTime b;
    b.year = 2026; b.month = 4; b.day = 7;
    b.hour = 11; b.minute = 20; b.second = 30;
    b.decimal_hours = TimeUtil::DecimalHoursFromHMS(b.hour, b.minute, b.second);

    const ElapsedTime e = TimeUtil::Elapsed(a, b);

    ExpectTrue("Elapsed basic valid", e.valid);
    ExpectFalse("Elapsed basic negative", e.negative);
    ExpectDouble("Elapsed basic total hours", e.total_hours, 1.0, 1e-5);
    ExpectInt("Elapsed basic days", e.days, 0);
    ExpectInt("Elapsed basic hours", e.hours, 1);
    ExpectInt("Elapsed basic minutes", e.minutes, 0);
    ExpectInt("Elapsed basic seconds", e.seconds, 0);
}

static void Test_Elapsed_AcrossDay()
{
    PrintSubHeader("Test_Elapsed_AcrossDay");

    LocalDateTime a;
    a.year = 2026; a.month = 4; a.day = 7;
    a.hour = 23; a.minute = 0; a.second = 0;
    a.decimal_hours = TimeUtil::DecimalHoursFromHMS(a.hour, a.minute, a.second);

    LocalDateTime b;
    b.year = 2026; b.month = 4; b.day = 8;
    b.hour = 1; b.minute = 30; b.second = 0;
    b.decimal_hours = TimeUtil::DecimalHoursFromHMS(b.hour, b.minute, b.second);

    const ElapsedTime e = TimeUtil::Elapsed(a, b);

    ExpectTrue("Elapsed across day valid", e.valid);
    ExpectFalse("Elapsed across day negative", e.negative);
    ExpectDouble("Elapsed across day total hours", e.total_hours, 2.5, 1e-5);
    ExpectInt("Elapsed across day days", e.days, 0);
    ExpectInt("Elapsed across day hours", e.hours, 2);
    ExpectInt("Elapsed across day minutes", e.minutes, 30);
    ExpectInt("Elapsed across day seconds", e.seconds, 0);
}

static void Test_Elapsed_AcrossYear()
{
    PrintSubHeader("Test_Elapsed_AcrossYear");

    LocalDateTime a;
    a.year = 2023; a.month = 12; a.day = 31;
    a.hour = 23; a.minute = 59; a.second = 30;
    a.decimal_hours = TimeUtil::DecimalHoursFromHMS(a.hour, a.minute, a.second);

    LocalDateTime b;
    b.year = 2024; b.month = 1; b.day = 1;
    b.hour = 0; b.minute = 0; b.second = 45;
    b.decimal_hours = TimeUtil::DecimalHoursFromHMS(b.hour, b.minute, b.second);

    const ElapsedTime e = TimeUtil::Elapsed(a, b);

    ExpectTrue("Elapsed across year valid", e.valid);
    ExpectFalse("Elapsed across year negative", e.negative);
    ExpectDouble("Elapsed across year total seconds", e.total_seconds, 75.0, 1e-5);
    ExpectInt("Elapsed across year days", e.days, 0);
    ExpectInt("Elapsed across year hours", e.hours, 0);
    ExpectInt("Elapsed across year minutes", e.minutes, 1);
    ExpectInt("Elapsed across year seconds", e.seconds, 15);
}

static void Test_Elapsed_Negative()
{
    PrintSubHeader("Test_Elapsed_Negative");

    LocalDateTime a;
    a.year = 2026; a.month = 4; a.day = 8;
    a.hour = 12; a.minute = 0; a.second = 0;
    a.decimal_hours = TimeUtil::DecimalHoursFromHMS(a.hour, a.minute, a.second);

    LocalDateTime b;
    b.year = 2026; b.month = 4; b.day = 7;
    b.hour = 12; b.minute = 0; b.second = 0;
    b.decimal_hours = TimeUtil::DecimalHoursFromHMS(b.hour, b.minute, b.second);

    const ElapsedTime e = TimeUtil::Elapsed(a, b);

    ExpectTrue("Elapsed negative valid", e.valid);
    ExpectTrue("Elapsed negative flag", e.negative);
    ExpectDouble("Elapsed negative total days abs", e.total_days, 1.0, 1e-9);
    ExpectInt("Elapsed negative days", e.days, 1);
    ExpectInt("Elapsed negative hours", e.hours, 0);
    ExpectInt("Elapsed negative minutes", e.minutes, 0);
    ExpectInt("Elapsed negative seconds", e.seconds, 0);
}

static void Test_Elapsed_InvalidInput()
{
    PrintSubHeader("Test_Elapsed_InvalidInput");

    LocalDateTime a;
    a.year = 2026; a.month = 2; a.day = 30; // invalid
    a.hour = 10; a.minute = 0; a.second = 0;
    a.decimal_hours = TimeUtil::DecimalHoursFromHMS(a.hour, a.minute, a.second);

    LocalDateTime b;
    b.year = 2026; b.month = 3; b.day = 1;
    b.hour = 10; b.minute = 0; b.second = 0;
    b.decimal_hours = TimeUtil::DecimalHoursFromHMS(b.hour, b.minute, b.second);

    const ElapsedTime e = TimeUtil::Elapsed(a, b);

    ExpectFalse("Elapsed invalid input valid flag", e.valid);
}

static void ExpectInt64(const std::string& name, long long actual, long long expected)
{
    if (actual == expected)
    {
        Pass(name);
    }
    else
    {
        std::ostringstream oss;
        oss << "expected=" << expected << " actual=" << actual;
        Fail(name, oss.str());
    }
}

static void PrintElapsedExact(const std::string& prefix, const TimeSpan& e)
{
    std::cout << prefix
              << "valid=" << BoolToString(e.valid)
              << ", negative=" << BoolToString(e.negative)
              << ", total_days=" << e.total_days
              << ", total_hours=" << e.total_hours
              << ", total_minutes=" << e.total_minutes
              << ", total_seconds=" << e.total_seconds
              << ", breakdown="
              << e.days << "d "
              << e.hours << "h "
              << e.minutes << "m "
              << e.seconds << "s\n";
}

static LocalDateTime MakeLocalDateTime(
    int year, int month, int day,
    int hour, int minute, int second)
{
    LocalDateTime dt;
    dt.year = year;
    dt.month = month;
    dt.day = day;
    dt.hour = hour;
    dt.minute = minute;
    dt.second = second;
    dt.decimal_hours = TimeUtil::DecimalHoursFromHMS(hour, minute, second);
    return dt;
}

static void Test_ElapsedExact_Basic()
{
    PrintSubHeader("Test_ElapsedExact_Basic");

    const LocalDateTime start = MakeLocalDateTime(2026, 4, 7, 10, 20, 30);
    const LocalDateTime end   = MakeLocalDateTime(2026, 4, 7, 11, 20, 30);

    const TimeSpan e = TimeUtil::ElapsedLong(start, end);

    PrintElapsedExact("ElapsedExact basic -> ", e);

    ExpectTrue("ElapsedExact basic valid", e.valid);
    ExpectFalse("ElapsedExact basic negative", e.negative);
    ExpectInt64("ElapsedExact basic total_seconds", e.total_seconds, 3600);
    ExpectInt64("ElapsedExact basic total_minutes", e.total_minutes, 60);
    ExpectInt64("ElapsedExact basic total_hours", e.total_hours, 1);
    ExpectInt64("ElapsedExact basic total_days", e.total_days, 0);
    ExpectInt("ElapsedExact basic days", e.days, 0);
    ExpectInt("ElapsedExact basic hours", e.hours, 1);
    ExpectInt("ElapsedExact basic minutes", e.minutes, 0);
    ExpectInt("ElapsedExact basic seconds", e.seconds, 0);
}

static void Test_ElapsedExact_AcrossDay()
{
    PrintSubHeader("Test_ElapsedExact_AcrossDay");

    const LocalDateTime start = MakeLocalDateTime(2026, 4, 7, 23, 0, 0);
    const LocalDateTime end   = MakeLocalDateTime(2026, 4, 8,  1, 30, 0);

    const TimeSpan e = TimeUtil::ElapsedLong(start, end);

    PrintElapsedExact("ElapsedExact across day -> ", e);

    ExpectTrue("ElapsedExact across day valid", e.valid);
    ExpectFalse("ElapsedExact across day negative", e.negative);
    ExpectInt64("ElapsedExact across day total_seconds", e.total_seconds, 9000);
    ExpectInt("ElapsedExact across day days", e.days, 0);
    ExpectInt("ElapsedExact across day hours", e.hours, 2);
    ExpectInt("ElapsedExact across day minutes", e.minutes, 30);
    ExpectInt("ElapsedExact across day seconds", e.seconds, 0);
}

static void Test_ElapsedExact_AcrossYear()
{
    PrintSubHeader("Test_ElapsedExact_AcrossYear");

    const LocalDateTime start = MakeLocalDateTime(2023, 12, 31, 23, 59, 30);
    const LocalDateTime end   = MakeLocalDateTime(2024,  1,  1,  0,  0, 45);

    const TimeSpan e = TimeUtil::ElapsedLong(start, end);

    PrintElapsedExact("ElapsedExact across year -> ", e);

    ExpectTrue("ElapsedExact across year valid", e.valid);
    ExpectFalse("ElapsedExact across year negative", e.negative);
    ExpectInt64("ElapsedExact across year total_seconds", e.total_seconds, 75);
    ExpectInt("ElapsedExact across year days", e.days, 0);
    ExpectInt("ElapsedExact across year hours", e.hours, 0);
    ExpectInt("ElapsedExact across year minutes", e.minutes, 1);
    ExpectInt("ElapsedExact across year seconds", e.seconds, 15);
}

static void Test_ElapsedExact_Negative()
{
    PrintSubHeader("Test_ElapsedExact_Negative");

    const LocalDateTime start = MakeLocalDateTime(2026, 4, 8, 12, 0, 0);
    const LocalDateTime end   = MakeLocalDateTime(2026, 4, 7, 12, 0, 0);

    const TimeSpan e = TimeUtil::ElapsedLong(start, end);

    PrintElapsedExact("ElapsedExact negative -> ", e);

    ExpectTrue("ElapsedExact negative valid", e.valid);
    ExpectTrue("ElapsedExact negative flag", e.negative);
    ExpectInt64("ElapsedExact negative total_seconds", e.total_seconds, 86400);
    ExpectInt("ElapsedExact negative days", e.days, 1);
    ExpectInt("ElapsedExact negative hours", e.hours, 0);
    ExpectInt("ElapsedExact negative minutes", e.minutes, 0);
    ExpectInt("ElapsedExact negative seconds", e.seconds, 0);
}

static void Test_ElapsedExact_InvalidInput()
{
    PrintSubHeader("Test_ElapsedExact_InvalidInput");

    const LocalDateTime invalid_dt = MakeLocalDateTime(2026, 2, 30, 10, 0, 0);
    const LocalDateTime valid_dt   = MakeLocalDateTime(2026, 3, 1, 10, 0, 0);

    const TimeSpan e = TimeUtil::ElapsedLong(invalid_dt, valid_dt);

    PrintElapsedExact("ElapsedExact invalid input -> ", e);

    ExpectFalse("ElapsedExact invalid input valid flag", e.valid);
}

static void Test_ToUnixLikeSeconds_Basic()
{
    PrintSubHeader("Test_ToUnixLikeSeconds_Basic");

    long long s0 = 0;
    long long s1 = 0;

    const LocalDateTime a = MakeLocalDateTime(1970, 1, 1, 0, 0, 0);
    const LocalDateTime b = MakeLocalDateTime(1970, 1, 1, 0, 0, 1);

    ExpectTrue("ToUnixLikeSeconds epoch", TimeUtil::ToLongSeconds(a, s0));
    ExpectTrue("ToUnixLikeSeconds epoch+1s", TimeUtil::ToLongSeconds(b, s1));

    std::cout << "epoch seconds     = " << s0 << "\n";
    std::cout << "epoch+1s seconds  = " << s1 << "\n";

    ExpectInt64("epoch should be 0", s0, 0);
    ExpectInt64("epoch+1s should be 1", s1, 1);
}

static void Test_ToUnixLikeSeconds_DeltaConsistency()
{
    PrintSubHeader("Test_ToUnixLikeSeconds_DeltaConsistency");

    long long s0 = 0;
    long long s1 = 0;

    const LocalDateTime a = MakeLocalDateTime(2026, 4, 7, 10, 20, 30);
    const LocalDateTime b = MakeLocalDateTime(2026, 4, 8, 12, 25, 40);

    ExpectTrue("ToUnixLikeSeconds A", TimeUtil::ToLongSeconds(a, s0));
    ExpectTrue("ToUnixLikeSeconds B", TimeUtil::ToLongSeconds(b, s1));

    const long long delta = s1 - s0;

    std::cout << "A seconds = " << s0 << "\n";
    std::cout << "B seconds = " << s1 << "\n";
    std::cout << "delta     = " << delta << "\n";

    // 1일 = 86400
    // 10:20:30 -> 다음날 12:25:40
    // = 1일 + 2시간 5분 10초
    // = 86400 + 7200 + 300 + 10 = 93910
    ExpectInt64("Delta consistency exact seconds", delta, 93910);
}

static void Test_Elapsed_FloatingVsExact_Comparison()
{
    PrintSubHeader("Test_Elapsed_FloatingVsExact_Comparison");

    const LocalDateTime start = MakeLocalDateTime(2023, 12, 31, 23, 59, 30);
    const LocalDateTime end   = MakeLocalDateTime(2024,  1,  1,  0,  0, 45);

    const ElapsedTime e_float = TimeUtil::Elapsed(start, end);
    const TimeSpan    e_exact = TimeUtil::ElapsedLong(start, end);

    std::cout << std::setprecision(17);
    std::cout << "Floating elapsed total_seconds = " << e_float.total_seconds << "\n";
    std::cout << "Exact    elapsed total_seconds = " << e_exact.total_seconds << "\n";

    ExpectTrue("Floating elapsed valid", e_float.valid);
    ExpectTrue("Exact elapsed valid", e_exact.valid);

    // exact는 반드시 정확히 75
    ExpectInt64("Exact elapsed 75 sec", e_exact.total_seconds, 75);

    // floating은 허용 오차 비교
    ExpectDouble("Floating elapsed near 75 sec", e_float.total_seconds, 75.0, 1e-5);
}

static void Test_FloatingTimeTolerance_Demonstration()
{
    PrintSubHeader("Test_FloatingTimeTolerance_Demonstration");

    const double h1 = TimeUtil::DecimalHoursFromHMS(1, 15, 30);
    int hh = 0, mm = 0, ss = 0;
    TimeUtil::DecimalHoursToHMS(h1, hh, mm, ss);

    std::cout << std::setprecision(17);
    std::cout << "decimal hours = " << h1 << "\n";
    std::cout << "roundtrip HMS = " << hh << ":" << mm << ":" << ss << "\n";

    ExpectInt("Tolerance demo hour", hh, 1);
    ExpectInt("Tolerance demo minute", mm, 15);
    ExpectInt("Tolerance demo second", ss, 30);

    const double total_sec_from_double = h1 * 3600.0;
    std::cout << "double * 3600 = " << total_sec_from_double << "\n";

    // 여기서는 exact equality를 일부러 검사하지 않고,
    // 근접 비교만 한다는 점을 보여주는 데 목적이 있음
    ExpectDouble("double*3600 near exact seconds", total_sec_from_double, 4530.0, 1e-9);
}

static void Tutorial_ElapsedExact_Usage()
{
    PrintSubHeader("Tutorial_ElapsedExact_Usage");

    const LocalDateTime start = MakeLocalDateTime(2026, 4, 7, 10, 20, 30);
    const LocalDateTime end   = MakeLocalDateTime(2026, 4, 8, 12, 25, 40);

    std::cout << "[Start]\n";
    PrintDateTime("  ", start.year, start.month, start.day, start.hour, start.minute, start.second);

    std::cout << "[End]\n";
    PrintDateTime("  ", end.year, end.month, end.day, end.hour, end.minute, end.second);

    const TimeSpan e = TimeUtil::ElapsedLong(start, end);

    std::cout << "[ElapsedExact result]\n";
    PrintElapsedExact("  ", e);

    ExpectTrue("Tutorial elapsed valid", e.valid);
    ExpectFalse("Tutorial elapsed negative", e.negative);
    ExpectInt64("Tutorial elapsed total_seconds", e.total_seconds, 93910);
}


static void ExpectString(const std::string& name, const std::string& actual, const std::string& expected)
{
    if (actual == expected)
    {
        Pass(name);
    }
    else
    {
        std::ostringstream oss;
        oss << "expected=[" << expected << "] actual=[" << actual << "]";
        Fail(name, oss.str());
    }
}

static void ExpectSizeT(const std::string& name, std::size_t actual, std::size_t expected)
{
    if (actual == expected)
    {
        Pass(name);
    }
    else
    {
        std::ostringstream oss;
        oss << "expected=" << expected << " actual=" << actual;
        Fail(name, oss.str());
    }
}

static void ExpectChar(const std::string& name, char actual, char expected)
{
    if (actual == expected)
    {
        Pass(name);
    }
    else
    {
        std::ostringstream oss;
        oss << "expected='" << expected << "' actual='" << actual << "'";
        Fail(name, oss.str());
    }
}

static bool IsAllDigits(const std::string& s)
{
    for (char c : s)
    {
        if (c < '0' || c > '9')
            return false;
    }
    return true;
}

static bool IsValidDisplayFormat(const std::string& s)
{
    // yyyy-mm-dd hh:mm:ss
    if (s.size() != 19)
        return false;

    if (s[4] != '-') return false;
    if (s[7] != '-') return false;
    if (s[10] != ' ') return false;
    if (s[13] != ':') return false;
    if (s[16] != ':') return false;

    std::string digits;
    digits.reserve(14);

    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16)
            continue;
        digits.push_back(s[i]);
    }

    return IsAllDigits(digits);
}

static bool IsValidFileNameFormat(const std::string& s)
{
    // yyyy-mm-dd_hh-mm-ss
    if (s.size() != 19)
        return false;

    if (s[4] != '-') return false;
    if (s[7] != '-') return false;
    if (s[10] != '_') return false;
    if (s[13] != '-') return false;
    if (s[16] != '-') return false;

    std::string digits;
    digits.reserve(14);

    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16)
            continue;
        digits.push_back(s[i]);
    }

    return IsAllDigits(digits);
}

static bool IsValidCompactFormat(const std::string& s)
{
    // yyyymmdd_hhmmss
    if (s.size() != 15)
        return false;

    if (s[8] != '_')
        return false;

    std::string left = s.substr(0, 8);
    std::string right = s.substr(9, 6);

    return IsAllDigits(left) && IsAllDigits(right);
}

static void PrintStringCase(const std::string& label, const std::string& value)
{
    std::cout << label << " = [" << value << "]\n";
}

static void Test_Format_Basic()
{
    PrintSubHeader("Test_Format_Basic");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);
    const std::string s = TimeUtil::Format(dt);

    PrintStringCase("Format", s);

    ExpectString("Format basic exact", s, "2026-04-07 19:42:15");
    ExpectSizeT("Format basic length", s.size(), 19);
    ExpectChar("Format basic char[4]", s[4], '-');
    ExpectChar("Format basic char[7]", s[7], '-');
    ExpectChar("Format basic char[10]", s[10], ' ');
    ExpectChar("Format basic char[13]", s[13], ':');
    ExpectChar("Format basic char[16]", s[16], ':');
    ExpectTrue("Format basic valid structure", IsValidDisplayFormat(s));
}

static void Test_Format_ZeroPadding()
{
    PrintSubHeader("Test_Format_ZeroPadding");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 9, 5, 3);
    const std::string s = TimeUtil::Format(dt);

    PrintStringCase("Format zero padded", s);

    ExpectString("Format zero padding exact", s, "2026-04-07 09:05:03");
    ExpectTrue("Format zero padding valid structure", IsValidDisplayFormat(s));
}

static void Test_FormatForFileName_Basic()
{
    PrintSubHeader("Test_FormatForFileName_Basic");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);
    const std::string s = TimeUtil::FormatForFileName(dt);

    PrintStringCase("FormatForFileName", s);

    ExpectString("FormatForFileName basic exact", s, "2026-04-07_19-42-15");
    ExpectSizeT("FormatForFileName length", s.size(), 19);
    ExpectChar("FormatForFileName char[4]", s[4], '-');
    ExpectChar("FormatForFileName char[7]", s[7], '-');
    ExpectChar("FormatForFileName char[10]", s[10], '_');
    ExpectChar("FormatForFileName char[13]", s[13], '-');
    ExpectChar("FormatForFileName char[16]", s[16], '-');
    ExpectTrue("FormatForFileName valid structure", IsValidFileNameFormat(s));
}

static void Test_FormatForFileName_ZeroPadding()
{
    PrintSubHeader("Test_FormatForFileName_ZeroPadding");

    const LocalDateTime dt = MakeLocalDateTime(2026, 1, 2, 3, 4, 5);
    const std::string s = TimeUtil::FormatForFileName(dt);

    PrintStringCase("FormatForFileName zero padded", s);

    ExpectString("FormatForFileName zero padding exact", s, "2026-01-02_03-04-05");
    ExpectTrue("FormatForFileName zero padding valid structure", IsValidFileNameFormat(s));
}

static void Test_FormatForDb_Basic()
{
    PrintSubHeader("Test_FormatForDb_Basic");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);
    const std::string s = TimeUtil::FormatForDb(dt);

    PrintStringCase("FormatForDb", s);

    ExpectString("FormatForDb exact", s, "2026-04-07 19:42:15");
    ExpectTrue("FormatForDb valid display structure", IsValidDisplayFormat(s));
}

static void Test_FormatCompact_Basic()
{
    PrintSubHeader("Test_FormatCompact_Basic");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);
    const std::string s = TimeUtil::FormatCompact(dt);

    PrintStringCase("FormatCompact", s);

    ExpectString("FormatCompact exact", s, "20260407_194215");
    ExpectSizeT("FormatCompact length", s.size(), 15);
    ExpectChar("FormatCompact char[8]", s[8], '_');
    ExpectTrue("FormatCompact valid structure", IsValidCompactFormat(s));
}

static void Test_FormatCompact_ZeroPadding()
{
    PrintSubHeader("Test_FormatCompact_ZeroPadding");

    const LocalDateTime dt = MakeLocalDateTime(2026, 1, 2, 3, 4, 5);
    const std::string s = TimeUtil::FormatCompact(dt);

    PrintStringCase("FormatCompact zero padded", s);

    ExpectString("FormatCompact zero padding exact", s, "20260102_030405");
    ExpectTrue("FormatCompact zero padding valid structure", IsValidCompactFormat(s));
}

static void Test_Format_EndOfYear()
{
    PrintSubHeader("Test_Format_EndOfYear");

    const LocalDateTime dt = MakeLocalDateTime(2023, 12, 31, 23, 59, 59);

    const std::string a = TimeUtil::Format(dt);
    const std::string b = TimeUtil::FormatForFileName(dt);
    const std::string c = TimeUtil::FormatForDb(dt);
    const std::string d = TimeUtil::FormatCompact(dt);

    PrintStringCase("Format", a);
    PrintStringCase("FormatForFileName", b);
    PrintStringCase("FormatForDb", c);
    PrintStringCase("FormatCompact", d);

    ExpectString("EndOfYear Format", a, "2023-12-31 23:59:59");
    ExpectString("EndOfYear FileName", b, "2023-12-31_23-59-59");
    ExpectString("EndOfYear Db", c, "2023-12-31 23:59:59");
    ExpectString("EndOfYear Compact", d, "20231231_235959");
}

static void Test_Format_BeginOfYear()
{
    PrintSubHeader("Test_Format_BeginOfYear");

    const LocalDateTime dt = MakeLocalDateTime(2024, 1, 1, 0, 0, 0);

    const std::string a = TimeUtil::Format(dt);
    const std::string b = TimeUtil::FormatForFileName(dt);
    const std::string c = TimeUtil::FormatForDb(dt);
    const std::string d = TimeUtil::FormatCompact(dt);

    PrintStringCase("Format", a);
    PrintStringCase("FormatForFileName", b);
    PrintStringCase("FormatForDb", c);
    PrintStringCase("FormatCompact", d);

    std::cout << "a = " << a << std::endl;

    ExpectString("BeginOfYear Format", a, "2024-01-01 00:00:00");
    ExpectString("BeginOfYear FileName", b, "2024-01-01_00-00-00");
    ExpectString("BeginOfYear Db", c, "2024-01-01 00:00:00");
    ExpectString("BeginOfYear Compact", d, "20240101_000000");
}

static void Test_Format_Consistency()
{
    PrintSubHeader("Test_Format_Consistency");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 9, 5, 3);

    const std::string normal  = TimeUtil::Format(dt);
    const std::string db      = TimeUtil::FormatForDb(dt);
    const std::string file    = TimeUtil::FormatForFileName(dt);
    const std::string compact = TimeUtil::FormatCompact(dt);

    PrintStringCase("Format", normal);
    PrintStringCase("FormatForDb", db);
    PrintStringCase("FormatForFileName", file);
    PrintStringCase("FormatCompact", compact);

    ExpectString("Format and Db should match", db, normal);
    ExpectTrue("Format valid structure", IsValidDisplayFormat(normal));
    ExpectTrue("Db valid structure", IsValidDisplayFormat(db));
    ExpectTrue("FileName valid structure", IsValidFileNameFormat(file));
    ExpectTrue("Compact valid structure", IsValidCompactFormat(compact));
}

static void Test_NowString_BasicStructure()
{
    PrintSubHeader("Test_NowString_BasicStructure");

    const std::string s = TimeUtil::NowString();

    PrintStringCase("NowString", s);

    ExpectSizeT("NowString length", s.size(), 19);
    ExpectTrue("NowString valid structure", IsValidDisplayFormat(s));
}

static void Test_NowFileNameString_BasicStructure()
{
    PrintSubHeader("Test_NowFileNameString_BasicStructure");

    const std::string s = TimeUtil::NowFileNameString();

    PrintStringCase("NowFileNameString", s);

    ExpectSizeT("NowFileNameString length", s.size(), 19);
    ExpectTrue("NowFileNameString valid structure", IsValidFileNameFormat(s));
}

static void Test_NowDbString_BasicStructure()
{
    PrintSubHeader("Test_NowDbString_BasicStructure");

    const std::string s = TimeUtil::NowDbString();

    PrintStringCase("NowDbString", s);

    ExpectSizeT("NowDbString length", s.size(), 19);
    ExpectTrue("NowDbString valid structure", IsValidDisplayFormat(s));
}

static void Test_NowCompactString_BasicStructure()
{
    PrintSubHeader("Test_NowCompactString_BasicStructure");

    const std::string s = TimeUtil::NowCompactString();

    PrintStringCase("NowCompactString", s);

    ExpectSizeT("NowCompactString length", s.size(), 15);
    ExpectTrue("NowCompactString valid structure", IsValidCompactFormat(s));
}

static void Test_NowStrings_CrossConsistency()
{
    PrintSubHeader("Test_NowStrings_CrossConsistency");

    const LocalDateTime now = TimeUtil::Now();

    const std::string expected_normal  = TimeUtil::Format(now);
    const std::string expected_file    = TimeUtil::FormatForFileName(now);
    const std::string expected_db      = TimeUtil::FormatForDb(now);
    const std::string expected_compact = TimeUtil::FormatCompact(now);

    PrintStringCase("Expected Format", expected_normal);
    PrintStringCase("Expected FileName", expected_file);
    PrintStringCase("Expected Db", expected_db);
    PrintStringCase("Expected Compact", expected_compact);

    // NowXXX()는 내부에서 다시 Now()를 호출하므로
    // 초 경계에서 바로 달라질 수 있음.
    // 따라서 exact 비교 대신 구조 검증과 길이 검증 위주로 본다.
    const std::string s1 = TimeUtil::NowString();
    const std::string s2 = TimeUtil::NowFileNameString();
    const std::string s3 = TimeUtil::NowDbString();
    const std::string s4 = TimeUtil::NowCompactString();

    PrintStringCase("NowString", s1);
    PrintStringCase("NowFileNameString", s2);
    PrintStringCase("NowDbString", s3);
    PrintStringCase("NowCompactString", s4);

    ExpectTrue("NowString structure", IsValidDisplayFormat(s1));
    ExpectTrue("NowFileNameString structure", IsValidFileNameFormat(s2));
    ExpectTrue("NowDbString structure", IsValidDisplayFormat(s3));
    ExpectTrue("NowCompactString structure", IsValidCompactFormat(s4));
}

static void Test_FileNameSafety()
{
    PrintSubHeader("Test_FileNameSafety");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);
    const std::string s = TimeUtil::FormatForFileName(dt);

    PrintStringCase("FileName-safe string", s);

    ExpectTrue("FileName should not contain colon", s.find(':') == std::string::npos);
    ExpectTrue("FileName should contain underscore", s.find('_') != std::string::npos);
    ExpectTrue("FileName valid structure", IsValidFileNameFormat(s));
}

static void Test_CompactStringSortableShape()
{
    PrintSubHeader("Test_CompactStringSortableShape");

    const LocalDateTime a = MakeLocalDateTime(2026, 4, 7, 9, 5, 3);
    const LocalDateTime b = MakeLocalDateTime(2026, 4, 7, 19, 42, 15);

    const std::string sa = TimeUtil::FormatCompact(a);
    const std::string sb = TimeUtil::FormatCompact(b);

    PrintStringCase("Compact A", sa);
    PrintStringCase("Compact B", sb);

    ExpectTrue("Compact A valid", IsValidCompactFormat(sa));
    ExpectTrue("Compact B valid", IsValidCompactFormat(sb));
    ExpectTrue("Compact lexical order should follow time order", sa < sb);
}

static void Tutorial_Format_UsageExamples()
{
    PrintSubHeader("Tutorial_Format_UsageExamples");

    const LocalDateTime dt = MakeLocalDateTime(2026, 4, 7, 9, 5, 3);

    const std::string display = TimeUtil::Format(dt);
    const std::string file    = TimeUtil::FormatForFileName(dt);
    const std::string db      = TimeUtil::FormatForDb(dt);
    const std::string compact = TimeUtil::FormatCompact(dt);

    std::cout << "[Display]   " << display << "\n";
    std::cout << "[FileName]  " << file << "\n";
    std::cout << "[DB]        " << db << "\n";
    std::cout << "[Compact]   " << compact << "\n";

    const std::string backup_name = "backup_" + file + ".zip";
    const std::string db_value = db;

    std::cout << "[Backup file example] " << backup_name << "\n";
    std::cout << "[DB value example]    " << db_value << "\n";

    ExpectString("Tutorial display exact", display, "2026-04-07 09:05:03");
    ExpectString("Tutorial file exact", file, "2026-04-07_09-05-03");
    ExpectString("Tutorial db exact", db, "2026-04-07 09:05:03");
    ExpectString("Tutorial compact exact", compact, "20260407_090503");
}


void test_time_utils_code::run_tests()
{
#if defined(_MSC_VER)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    Test_IsLeapYear();
    Test_DaysInMonth();
    Test_IsValidDate();
    Test_IsValidTime();
    Test_IsValidDateTime();
    Test_DecimalHoursFromHMS();
    Test_DecimalHoursToHMS_Basic();
    Test_DecimalHoursToHMS_Rounding();
    Test_DecimalHoursToHMS_WithDateAdjust();
    Test_RoundTrip_HMS_DecimalHours();
    Test_GetCurrentLocalDateTime();
    Test_GetDefaultLocalDateTime();
    Test_ToJulianDay_Basic();
    Test_JulianDay_RoundTrip();
    Test_KnownJulianDayReference();

    Test_Elapsed_Basic();
    Test_Elapsed_AcrossDay();
    Test_Elapsed_AcrossYear();
    Test_Elapsed_Negative();
    Test_Elapsed_InvalidInput();


    Tutorial_ElapsedExact_Usage();

    Test_ElapsedExact_Basic();
    Test_ElapsedExact_AcrossDay();
    Test_ElapsedExact_AcrossYear();
    Test_ElapsedExact_Negative();
    Test_ElapsedExact_InvalidInput();

    Test_ToUnixLikeSeconds_Basic();
    Test_ToUnixLikeSeconds_DeltaConsistency();

    Test_Elapsed_FloatingVsExact_Comparison();
    Test_FloatingTimeTolerance_Demonstration();


    Tutorial_Format_UsageExamples();

    Test_Format_Basic();
    Test_Format_ZeroPadding();

    Test_FormatForFileName_Basic();
    Test_FormatForFileName_ZeroPadding();

    Test_FormatForDb_Basic();

    Test_FormatCompact_Basic();
    Test_FormatCompact_ZeroPadding();

    Test_Format_EndOfYear();
    Test_Format_BeginOfYear();
    Test_Format_Consistency();

    Test_NowString_BasicStructure();
    Test_NowFileNameString_BasicStructure();
    Test_NowDbString_BasicStructure();
    Test_NowCompactString_BasicStructure();
    Test_NowStrings_CrossConsistency();

    Test_FileNameSafety();
    Test_CompactStringSortableShape();

    PrintHeader("TEST SUMMARY");
    std::cout << "PASS = " << g_test_pass << "\n";
    std::cout << "FAIL = " << g_test_fail << "\n";

    if (g_test_fail == 0)
    {
        std::cout << "ALL TESTS PASSED\n";
    }

    std::cout << "SOME TESTS FAILED\n";
}
```
---
