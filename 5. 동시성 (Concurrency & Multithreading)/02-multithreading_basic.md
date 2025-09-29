# C++ Multi Threading 기초

## 1. 멀티스레딩이란?

- 하나의 **프로그램**이 여러 개의 **Thread(실행 흐름)**로 구성된 형태
- 각 Thread는 독립적으로 동작하지만, **동일한 프로세스 메모리 공간**을 공유
- 마치 여러 Thread가 동시에 실행되는 것처럼 **시분할(Time-Slicing)** 방식으로 번갈아 실행
- 하나의 CPU(코어)에서도 여러 Thread를 운영할 수 있으며, 멀티코어 환경에서는 **실제 병렬 실행** 가능

### 장점
- CPU 자원 활용 극대화
- I/O 작업과 CPU 연산을 동시에 수행 가능
- 응답성 향상(UI, 네트워크, 파일 처리 등)

### 단점
- 동기화 문제 발생 가능 (데드락, 레이스 컨디션)
- 디버깅 난이도 증가

---

## 2. Thread 종류

### Main Thread
- `main()` 함수가 실행되는 스레드
- 프로그램 실행 시 가장 먼저 시작

### Worker Thread (부가적인 Thread)
- 개발자가 생성하여 특정 작업을 맡기는 스레드
- Main Thread와 병렬로 동작

---

## 3. 간단한 Thread 예제

```cpp
#include <iostream>
#include <thread>
using namespace std;

void thread_test1() {
    int a = 10;

    // 람다 캡처 [=] 로 a 값 복사
    std::thread t([=] {
        cout << "hello world." << endl;
        cout << "a value : " << a << endl;
    });

    // join()을 호출해야 t 스레드가 끝날 때까지 대기
    t.join();
}

int main() {
    // 1) 현재 PC의 논리 프로세서(코어) 개수
    cout << "Hardware concurrency: " 
         << std::thread::hardware_concurrency() << endl;

    // 2) 현재 스레드(Main thread)의 ID
    cout << "Main thread ID: " << std::this_thread::get_id() << endl;

    // 3) 새로운 스레드 생성
    std::thread t1([] {
        cout << "Worker thread ID: " << std::this_thread::get_id() << endl;
    });

    // 4) t1 스레드 종료 대기
    t1.join();

    // 5) 추가 테스트
    thread_test1();
}
```

---

## 4. 실행 예시 (환경에 따라 다름)

```
Hardware concurrency: 16
Main thread ID: 25332
Worker thread ID: 23176
hello world.
a value : 10
```

---

## 5. 핵심 메서드

| 메서드 | 설명 |
|--------|------|
| `std::thread::hardware_concurrency()` | 시스템 논리 CPU 개수 반환 |
| `std::this_thread::get_id()` | 현재 실행 중인 스레드 ID 반환 |
| `join()` | 해당 스레드가 종료될 때까지 호출 스레드를 블록 |
| `detach()` | 스레드를 백그라운드로 분리 (join 없이 실행 가능, 하지만 관리 불가) |

---

## 6. 주의사항
- `join()` 또는 `detach()` 중 반드시 한 번은 호출해야 함 (그렇지 않으면 프로그램 종료 시 예외 발생)
- 멀티스레드 환경에서 공유 자원 접근 시 `std::mutex` 등 동기화 필요

