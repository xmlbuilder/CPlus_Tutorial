## 📘 C++17 Exception과 Stack Trace (Windows/MSVC)

### 📌 1. 목적

- C++17에서 Java의 `Exception`과 비슷하게 **예외 메시지, 발생 파일, 줄 번호, 함수 이름, 호출 스택** 을 함께 기록한다.
  - `NurbsException`: 오류 내용과 `NURBS_THROW` 발생 위치 보관
  - `StackTrace`: 예외 객체가 만들어지는 시점의 호출 경로 수집
  - `catch`에서 출력하더라도 예외 발생 당시의 스택을 확인할 수 있다.

> **범위:** 아래 구현은 Windows/MSVC용이다. Linux/macOS에서는 스택 수집부를 `backtrace()` 등으로 교체할 수 있다.
> 실제 호출 스택은 컴파일러 최적화, 심볼(PDB), 프레임 구성에 따라 달라질 수 있다.

### 📌 2. StackTrace.h

```cpp
#pragma once
#include <string>

class StackTrace {
public:
    static std::string Capture(unsigned int skipFrames = 0,
                               unsigned int maxFrames = 32);
};
```

### 📌 3. StackTrace.cpp

```cpp
#include "StackTrace.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <DbgHelp.h>

#include <algorithm>
#include <array>
#include <mutex>
#include <sstream>

#pragma comment(lib, "Dbghelp.lib")

std::string StackTrace::Capture(unsigned int skipFrames,
                                unsigned int maxFrames)
{
    constexpr unsigned int kMaxFrames = 64;
    std::array<void*, kMaxFrames> frames{};
    const unsigned int requested = std::min(maxFrames, kMaxFrames);
    if (requested == 0)
        return {};

    // Capture() 자체의 프레임을 건너뛴다.
    const USHORT count = CaptureStackBackTrace(
        static_cast<DWORD>(skipFrames + 1),
        static_cast<DWORD>(requested),
        frames.data(), nullptr);

    // DbgHelp 함수는 스레드 안전하지 않으므로 호출을 직렬화한다.
    static std::mutex symbolMutex;
    std::lock_guard<std::mutex> lock(symbolMutex);
    const HANDLE process = GetCurrentProcess();

    // 프로세스에서 DbgHelp를 사용하는 다른 코드가 있다면
    // 초기화/잠금/종료 정책을 공통으로 관리해야 한다.
    static const bool initialized = [process] {
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS |
                      SYMOPT_LOAD_LINES);
        return SymInitialize(process, nullptr, TRUE) == TRUE;
    }();

    std::ostringstream out;
    if (!initialized) {
        out << "Symbol initialization failed\n";
        return out.str();
    }

    alignas(SYMBOL_INFO) char storage[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
    auto* symbol = reinterpret_cast<SYMBOL_INFO*>(storage);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    for (USHORT i = 0; i < count; ++i) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[i]);
        DWORD64 displacement = 0;
        out << "  [" << i << "] ";
        if (SymFromAddr(process, address, &displacement, symbol))
            out << symbol->Name;
        else
            out << "Unknown";
        out << " (0x" << std::hex << address << std::dec << ")\n";
    }
    return out.str();
}
```

- `CaptureStackBackTrace()`는 주소를 수집하고 `SymFromAddr()`는 디버그 심볼을 이용해 함수 이름을 찾는다.
- 위 구현은 **함수 이름과 주소** 를 출력한다.
- 파일·줄 번호는 `NurbsException`이 `__FILE__`, `__LINE__`으로 따로 보관한다.
- 각 스택 프레임의 소스 위치가 필요하면 `SymGetLineFromAddr64()`를 추가할 수 있다.

### 📌 4. NurbsException.h

```cpp
#pragma once

#include "StackTrace.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

class NurbsException final : public std::runtime_error {
public:
    explicit NurbsException(const std::string& message,
                            const char* file,
                            int line,
                            const char* function)
        : std::runtime_error(message),
          m_file(file ? file : ""),
          m_line(line),
          m_function(function ? function : ""),
          m_stackTrace(StackTrace::Capture(1))
    {}

    const std::string& File() const noexcept { return m_file; }
    int Line() const noexcept { return m_line; }
    const std::string& Function() const noexcept { return m_function; }
    const std::string& GetStackTrace() const noexcept { return m_stackTrace; }

    std::string ToString() const {
        std::ostringstream os;
        os << "NurbsException: " << what()
           << "\n  File     : " << m_file
           << "\n  Line     : " << m_line
           << "\n  Function : " << m_function;
        return os.str();
    }

    void Print(std::ostream& os = std::cerr) const {
        os << ToString() << '\n';
    }

    void PrintStackTrace(std::ostream& os = std::cerr) const {
        os << ToString() << "\n\nStack Trace:\n" << m_stackTrace;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const NurbsException& ex) {
        return os << ex.ToString();
    }

private:
    std::string m_file;
    int m_line{0};
    std::string m_function;
    std::string m_stackTrace;
};

#define NURBS_THROW(message) \
    throw NurbsException((message), __FILE__, __LINE__, __func__)
```

- `StackTrace::Capture(1)`은 `NurbsException` 생성자 프레임을 건너뛰고 실제 호출 함수를 먼저 보여주기 위한 설정이다.
- 인라이닝이나 빌드 설정에 따라 프레임 번호가 달라질 수 있으므로 필요하면 `skipFrames`를 조정한다.

### 📌 5. 사용 예제

```cpp
#include "NurbsException.h"
#include <iostream>

void EvaluateCurve() {
    NURBS_THROW("Invalid curve parameter");
}

void RunTests() {
    EvaluateCurve();
}

int main() {
    try {
        RunTests();
    } catch (const NurbsException& ex) {
        std::cout << "Message: " << ex.what() << "\n\n";
        ex.PrintStackTrace(std::cout);
    }
}
```

- 출력 예시(주소·프레임은 실행 환경에 따라 다름):

```text
Message: Invalid curve parameter

NurbsException: Invalid curve parameter
  File     : test_exception.cpp
  Line     : 5
  Function : EvaluateCurve

Stack Trace:
  [0] EvaluateCurve (0x7ff6ff851f1b)
  [1] RunTests (0x7ff6ff851e81)
  [2] main (0x7ff6ff717ff3)
  [3] invoke_main (...)
  ...
```

### 📌 6. CMake

```cmake
# StackTrace.cpp를 실제 라이브러리 타깃에 추가한다.
target_sources(NurbsLib PRIVATE StackTrace.cpp)

if(WIN32)
    target_link_libraries(NurbsLib PRIVATE dbghelp)
endif()
```

- `NurbsLib`는 실제 CMake 타깃 이름으로 변경한다.
- 함수 이름 해석에는 PDB가 도움이 된다. Release에서는 인라이닝·최적화로 호출 프레임이 누락되거나 달라질 수 있다.

### 📌 7. Java와 비교

| Java | C++17 구현 | 비고 |
|---|---|---|
| `getMessage()` | `what()` | 오류 메시지 |
| `toString()` | `ToString()` | 메시지와 발생 위치 |
| `printStackTrace()` | `PrintStackTrace()` | 생성 시점에 저장한 호출 경로 |
| `getCause()` | 미구현 | 필요하면 `std::nested_exception` 검토 |

### 📌 8. 주의 사항

- **예외 생성 시점에 캡처:** `catch`에서 처음 수집하면 예외를 던진 함수의 스택은 이미 해제되었을 수 있다.
- **DbgHelp 동기화:** 이 예제 내부의 `Capture()` 호출은 mutex로 보호한다. 다른 모듈에서 DbgHelp를 호출한다면 **같은 잠금** 과 공통 초기화 정책이 필요하다.
- **심볼 의존성:** PDB가 없으면 함수 이름이 `Unknown`으로 나올 수 있다.
- **성능:** 예외 객체 생성 때마다 스택을 수집·해석하므로 정상적인 반복 계산 경로에서 예외를 분기 처리처럼 사용하지 않는다.
- **범위:** `NurbsException`은 명시적으로 던진 예외를 추적한다. Use-After-Free나 Heap Corruption의 최초 발생 위치를 찾으려면 AddressSanitizer 같은 별도 도구가 필요하다.
- **C++17:** 표준 `std::stacktrace`는 C++23 기능이며, 구현체별 지원 여부를 확인해야 한다.
- 정리: `NurbsException`은 오류 내용과 발생 위치를, `StackTrace`는 호출 경로를 담당한다. 플랫폼 의존 코드를 `StackTrace.cpp`에 격리하면 나중에 Linux/macOS 구현을 추가하기 쉽다.

---

