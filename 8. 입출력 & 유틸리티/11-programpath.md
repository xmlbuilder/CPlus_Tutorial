# 📦 실행 파일 경로 얻기 - 크로스 플랫폼 정리

| 플랫폼     | 방법              | 코드 예시                                               | 설명                     |
|------------|------------------|--------------------------------------|-------------------------------------------|
| Windows    | `GetModuleFileName()` | `GetModuleFileNameA(NULL, path, MAX_PATH);` | 실행 중인 `.exe`의 전체 경로 반환 |
| Linux      | `readlink("/proc/self/exe")`| `readlink("/proc/self/exe", path, PATH_MAX);` | 현재 실행 중인 바이너리의 심볼릭 링크|
| macOS      | `_NSGetExecutablePath()` | `_NSGetExecutablePath(path, &size);` | macOS에서 실행 파일 경로를 얻는 공식 API|
| C++17 이상 | `std::filesystem::current_path()`| `std::filesystem::current_path();`| 현재 작업 디렉토리 (실행 경로와 다를 수 있음) |


## 🧠 주의사항
- std::filesystem::current_path()는 작업 디렉토리를 반환하며, 실행 파일의 위치와 다를 수 있음.
- Windows/macOS/Linux 모두 실행 파일 경로를 얻는 API는 표준 C++에는 포함되어 있지 않음.
- 크로스 플랫폼 지원을 위해서는 #ifdef 등을 활용한 분기 처리 필요.

## 🛠 예시: 크로스 플랫폼 함수
```cpp
std::string getExecutablePath() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
#elif __APPLE__
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
        return std::string(path);
    else
        return "";
#elif __linux__
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    return std::string(path, (count > 0) ? count : 0);
#else
    return "";
#endif
}
```
----



