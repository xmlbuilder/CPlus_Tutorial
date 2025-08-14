# C++ `thread_local` 변수 심화 가이드

## 1. 개념

C++11에서 새로 추가된 저장소 클래스 지정자(Storage Class Specifier)로,  
각 **스레드마다 독립적인 저장 공간**을 가지는 변수를 선언할 때 사용합니다.

- 기존 저장소 지정자: `register`, `static`, `extern`, `auto`
- C++11 추가: `thread_local` (TLS - **Thread Local Storage** 지원)

> `thread_local` 변수는 스레드마다 별도의 인스턴스를 가지므로, 스레드 간에 값이 공유되지 않습니다.

---

## 2. 특징

- 선언된 변수는 **스레드별로 독립된 복사본**을 가짐
- 같은 이름의 `thread_local` 변수라도 각 스레드에서 다른 값 유지
- 스레드가 생성될 때 해당 스레드용 인스턴스가 초기화되고, 스레드 종료 시 파괴됨
- 전역, 네임스페이스 범위, 함수 범위, 클래스의 정적 멤버로 사용 가능
- Visual Studio는 2015 버전부터 지원

---

## 3. 기본 사용 예제

```cpp
#include <iostream>
#include <thread>
#include <mutex>

thread_local unsigned int i = 0; // 스레드별 독립 변수
std::mutex mtx;

void ThreadFunc(int nID)
{
    ++i; // 현재 스레드의 i 값만 변경
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << nID << "-thread : " << i << std::endl;
}

int main()
{
    std::thread th1(ThreadFunc, 0), th2(ThreadFunc, 1);

    {
        std::unique_lock<std::mutex> lock(mtx);
        i += 2; // 메인 스레드의 i만 변경
        std::cout << "Main thread : " << i << std::endl;
    }

    th1.join();
    th2.join();
    return 0;
}
```

**출력 예시**
```
Main thread : 2
0-thread : 1
1-thread : 1
```

---

## 4. 동작 원리: TLS(Thread Local Storage) 메커니즘

### 4.1 컴파일러와 TLS
- `thread_local`로 선언된 전역/정적 변수는 **TLS 영역**에 배치됩니다.
- TLS 영역은 **각 스레드마다 별도로 존재**하며, 운영체제의 스레드 제어 블록(TCB)에 의해 관리됩니다.
- 스레드가 시작될 때, TLS 초기화 코드가 실행되어 변수의 초기값이 설정됩니다.

### 4.2 메모리 구조
운영체제/플랫폼마다 구현 차이는 있으나 개념은 비슷합니다.

**일반적인 스레드 메모리 배치**
```
[ 스레드 스택 ]
[ TLS (Thread Local Storage) ]
[ 힙 영역 (thread-safe) ]
[ 코드 영역 / 전역 데이터 ]
```

- **TLS 영역**: 스레드 전용 전역/정적 데이터 보관
- 각 스레드마다 TLS의 **별도 복사본**이 있어 데이터 레이스 없이 접근 가능

### 4.3 접근 방식
- 컴파일러는 `thread_local` 변수 접근 시, 현재 실행 중인 스레드의 TCB에서 해당 변수의 오프셋을 계산하여 접근합니다.
- 리눅스의 경우 ELF 바이너리에서 `.tdata`(초기화 데이터), `.tbss`(0 초기화 데이터) 섹션이 TLS 용도로 사용됩니다.

---

## 5. C++17 이후: `constexpr thread_local` 초기화

C++17부터는 `constexpr`로 선언한 `thread_local` 변수를 **정적 초기화(static initialization)** 가능하게 지원합니다.

### 5.1 예제
```cpp
#include <iostream>
#include <thread>

constexpr thread_local int base_value = 42;
thread_local int counter = base_value; // 각 스레드별로 42로 초기화

void print_counter(int id) {
    std::cout << "Thread " << id << " counter = " << counter << std::endl;
}

int main() {
    std::thread t1(print_counter, 1);
    std::thread t2(print_counter, 2);
    t1.join();
    t2.join();
}
```

**출력 예시**
```
Thread 1 counter = 42
Thread 2 counter = 42
```

- `constexpr thread_local` 초기화는 컴파일 타임에 결정되어, 스레드 시작 시 별도의 동적 초기화 코드 없이 TLS에 바로 배치됩니다.
- **장점**: 초기화 비용이 줄어들고, 예측 가능성이 향상됩니다.

---

## 6. 활용 예시

- 스레드별 버퍼, 캐시, 로깅 컨텍스트 저장
- 스레드 전용 상태 관리 (예: 연결 ID, 스레드별 통계)
- 락 없이 안전한 스레드별 데이터 접근

---

## 7. 주의 사항

- 스레드 로컬 변수는 **메모리 사용량 증가** 가능성 있음 (스레드 수 × 변수 크기)
- 객체 소멸 시점은 해당 스레드 종료 시
- 너무 큰 객체를 thread_local에 두면 스레드 생성/파괴 비용이 증가

---

## 8. 요약

`thread_local`은 C++11에서 도입된 스레드 전용 저장소 지정자이며,  
C++17부터는 `constexpr` 초기화를 통한 컴파일 타임 TLS 초기화가 가능해졌습니다.  
이를 통해 스레드별 독립 상태를 안전하게 유지하고, 동기화 오버헤드를 줄일 수 있습니다.
