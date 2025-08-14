# 🔒 C++ Mutex & Lock 예제

## 📌 개요
멀티스레드 환경에서 **공유 데이터 보호**를 위해 `std::mutex`와 그 관련 도구(`lock_guard`, `unique_lock`)를 사용하는 방법을 소개합니다.  
이 문서는 다음 내용을 다룹니다:
- `std::mutex` 기본 사용법
- `std::lock_guard`와 `std::unique_lock` 비교
- `lock()`, `unlock()`, `try_lock()` 동작 방식
- 실제 동작 예시 및 출력 결과

---

## 📂 코드 1 — `std::lock_guard` 예제

`lock_guard`는 스코프 기반의 자동 잠금/해제를 제공합니다.  
스코프가 종료되면 자동으로 `unlock()`이 호출됩니다.

```cpp
#include <thread>
#include <iostream>
#include <mutex>

int v = 1;

void critical_section(int change_v) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx); // 자동 lock/unlock

    if(change_v == 3){
        int t = 0;
        for(int i=0; i<10000; i++){
            t += i; // 부하 작업
        }
    }
    v = change_v;
}

void thread_test2(){
    std::thread t2(critical_section, 2), t1(critical_section, 3);
    t2.join();
    t1.join();
    std::cout << v << std::endl;
}
```

**출력 예시**
```
2
```

---

## 📂 코드 2 — `std::unique_lock` 예제

`unique_lock`은 `lock_guard`보다 유연하게 **잠금과 해제를 반복**할 수 있습니다.  
`unlock()` 후 다시 `lock()` 가능하며, 필요 시 지연 잠금(deferred lock)도 가능합니다.

```cpp
#include <thread>
#include <iostream>
#include <mutex>

int v1 = 1;

void critical_section1(int change_v) {
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx); // 유연한 잠금

    v1 = change_v;
    lock.unlock(); // 잠금 해제

    lock.lock(); // 다시 잠금
    std::cout << "Something ...." << std::endl;
    v1 += 1;
    std::cout << "v value : " <<  v1 << std::endl;
    lock.unlock();
}

void thread_test3(){
    std::thread t2(critical_section1, 2), t1(critical_section1, 3);
    t2.join();
    t1.join();
    std::cout << v1 << std::endl;
}
```

**출력 예시**
```
Something ....
v value : 4
Something ....
v value : 3
3
```

---

## 📂 코드 3 — `lock()` / `unlock()` 직접 사용

직접 잠금과 해제를 수행하는 예제입니다.  
**주의**: 예외 발생 시 unlock()이 호출되지 않으면 데드락 위험이 있습니다.  
가능하면 `lock_guard` 또는 `unique_lock` 사용을 권장합니다.

```cpp
#include <thread>
#include <mutex>
#include <iostream>

std::mutex mtx;

void ThreadFunc(int nID, int& nVal) {
    for (int i = 0 ; i < 5 ; ++i) {
        mtx.lock();
        std::cout << "Value of " << nID << " -> " << nVal << std::endl;
        ++nVal;
        mtx.unlock();
    }
}

int main() {
    int nVal = 0;
    std::thread th1(ThreadFunc, 0, std::ref(nVal));
    std::thread th2(ThreadFunc, 1, std::ref(nVal));
    th1.join();
    th2.join();
    return 0;
}
```

**출력 예시**
```
Value of 0 -> 0
Value of 0 -> 1
Value of 0 -> 2
Value of 0 -> 3
Value of 0 -> 4
Value of 1 -> 5
Value of 1 -> 6
Value of 1 -> 7
Value of 1 -> 8
Value of 1 -> 9
```

---

## 💡 `try_lock()` 사용법
```cpp
if (mtx.try_lock()) {
    // 잠금 성공 → 작업 수행
    mtx.unlock();
} else {
    // 잠금 실패 → 다른 작업 수행
}
```
- **즉시 반환**하며, 잠금 성공 시 `true`, 실패 시 `false`를 리턴
- 이미 같은 스레드에서 잠금 상태라면 **미정의 동작(undefined behavior)** 발생

---

## 📊 요약
| 방식                        | 특징 | 장점 | 단점 |
|-----------------------------|------|------|------|
| `lock_guard`                | 스코프 기반 자동 잠금 해제 | 안전, 간단 | 중간 해제 불가 |
| `unique_lock`               | 유연한 잠금/해제 제어 가능 | lock/unlock 반복 가능 | 약간의 오버헤드 |
| `lock()` / `unlock()` 직접  | 수동 제어 | 완전 제어 가능 | 예외 처리 미흡 시 데드락 위험 |
| `try_lock()`                 | 즉시 잠금 시도 | 대기 없이 처리 가능 | 실패 시 처리 로직 필요 |
