## 📘 Process Handling with Boost.Process (Linux & macOS)

- 표준 C++에는 **프로세스 생성/파이프/리다이렉션** 을 다루는 정식 라이브러리가 아직 없습니다.  
-  (사실상 `std::system` 정도만 존재)  
-  따라서 **Boost.Process** 를 사용하는 것이 가장 간단하고 휴대성 높은 방법입니다.
- 현재 표준(C++23/26 시점 기준)에는 `std::process`가 없습니다.  
- 추후 표준화 제안은 있지만(working paper 수준), 실제 표준에 포함되기 전까지는 **Boost.Process** 사용을 권장합니다.
- Boost는 표준화의 실험장이자 전초기지 역할을 하므로, 추후 표준화가 되면 **사용법이 크게 다르지 않을 가능성** 이 큽니다.


### 📌 CMake 설정 (권장 방식)

- `.so` 파일 이름을 직접 링크하지 말고 **임포트 타겟** 을 사용하세요.

~~~cmake
cmake_minimum_required(VERSION 3.16)
project(ProcessDemo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.70 REQUIRED COMPONENTS process filesystem system)
find_package(Threads REQUIRED)

add_executable(lambdaTest main.cpp)

# Boost.Process는 헤더온리이지만, 예제에서 search_path 등을 쓰면
# filesystem/system이 필요할 수 있습니다.
target_link_libraries(lambdaTest
  PRIVATE
    Boost::process
    Boost::filesystem
    Boost::system
    Threads::Threads   # -pthread
)
~~~

- 기존 예시처럼 `target_link_libraries(lambdaTest libpthread.so libboost_filesystem.so)` 처럼 쓰는 것은 **비권장** 입니다.  
-  배포 환경/플랫폼에 따라 파일명이 달라질 수 있고, CMake의 **이식성 장점** 을 잃습니다. 


### 📌 공통 유틸 함수 (stdout 읽기 + 진행 콜백)

- 아래 함수는 **명령 실행** 과 **stdout 파이프 읽기**, 그리고 **진행 콜백**(폴링)을 제공합니다.

~~~cpp
// main.cpp (공통)
#include <boost/process.hpp>
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

namespace bp = boost::process;

int runProcess(const std::string& exe,
               const std::vector<std::string>& vecArg,
               std::function<void()> on_progress)
{
    bp::ipstream out; // child's stdout
    // 주의: search_path를 사용하면 PATH에서 exe를 찾아줍니다.
    // exe가 절대경로면 search_path를 쓰지 않아도 됩니다.
    bp::child cmd(bp::search_path(exe), bp::args(vecArg), bp::std_out > out);

    // 프로세스가 도는 동안 진행 콜백 호출 (폴링)
    while (cmd.running()) {
        if (on_progress) on_progress();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    cmd.wait();

    // 남은 stdout 읽기
    std::string line;
    while (std::getline(out, line)) {
        if (!line.empty())
            std::cout << line << '\n';
    }
    return cmd.exit_code();
}
~~~



### 📌 Linux 예제

- `ls -al /home/jhjeong` 을 실행하면서 콜백으로 진행률을 출력합니다.

~~~cpp
int main()
{
    std::vector<std::string> args;
    args.emplace_back("-al");
    args.emplace_back("/home/jhjeong");

    int progress = 0;
    int rc = runProcess("ls", args, [&](){
        ++progress;
        // 필요 시 GUI/텍스트 업데이트 등
        // std::cout << "tick: " << progress << '\n';
    });

    std::cout << "exit code = " << rc << "\n";
    std::cout << "progress ticks = " << progress << "\n";
    return rc;
}
~~~

---

### 📌 macOS 예제 (`.app` 실행)

- macOS에서는 **직접 바이너리** 대신 `open` 커맨드로 **앱 번들** 을 여는 경우가 많습니다.  
- 앱이 종료될 때까지 기다리려면 `-W` 옵션을 사용하세요.

~~~cpp
int main()
{
    std::vector<std::string> args;
    args.emplace_back("-W");
    args.emplace_back("/Users/jjh/Development/Sample.app");

    int rc = runProcess("open", args, [](){
        // macOS에서는 굳이 폴링 메시지를 찍지 않아도 됨
    });
    std::cout << "exit code = " << rc << "\n";
    return rc;
}
~~~

- 참고: `.app` 내부의 실제 실행 파일을 직접 실행할 수도 있지만,  
- 권장되는 방식은 **`open` 이용** 입니다. (권한, 워킹디렉터리, 런치서비스 연계 등)

---

### 📌 팁 & 주의사항

- **표준만으로 대체 가능한가?**  
  - 현재는 사실상 `std::system` 한계. 파이프/리다이렉션/비동기 실행을 표준만으로 **편하게** 구현하기 어렵습니다.
- **stderr도 받고 싶다** 면 `bp::std_err > some_stream` 을 추가하거나 `std_out & std_err > out` 으로 합쳐서 받을 수 있습니다.
- **환경 변수 전달/작업 디렉터리 지정**: `bp::environment` / `bp::start_dir` 사용.
- **타임아웃**이 필요하면 별도 스레드에서 타이머를 두고 `cmd.terminate()` or 적절한 대기 로직을 추가.
- **Windows**: `cmd /C <command>` 방식이나 exe 경로를 직접 주면 됩니다. CMake 링크 타겟은 동일합니다.

---

### 📌 정리

- 표준 C++에는 아직 **프로세스 관리 표준 라이브러리 부재** → Boost.Process 채택 권장
- CMake는 **임포트 타겟** 사용 (파일명 직접 링크 ❌)
- Linux/macOS 모두 동일 유틸 함수로 처리 가능, macOS는 `open -W <.app>` 활용

---

