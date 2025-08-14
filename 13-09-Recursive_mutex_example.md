# 🔄 C++ `std::recursive_mutex` 예제

## 📌 개요
이 예제는 C++에서 `std::recursive_mutex`를 사용하여 **같은 스레드에서 동일한 뮤텍스를 중첩 잠금(lock)** 하는 방법을 보여줍니다.  
특히, 한 함수(`fun2`)가 잠금을 획득한 상태에서 **다른 잠금을 요구하는 함수(`fun1`)를 호출**하는 경우를 안전하게 처리합니다.

---

## 📂 코드

```cpp
#include <iostream>
#include <mutex>
#include <thread>

class X {
    std::recursive_mutex m; // 같은 스레드에서 중첩 잠금 가능
    std::string shared;

public:
    void fun1() {
        std::lock_guard<std::recursive_mutex> lk(m);
        std::thread::id this_id = std::this_thread::get_id();
        std::cout << "fun1 Thread Id : " << this_id << std::endl;

        shared = "fun1";
        std::cout << "in fun1, shared variable is now " << shared << std::endl;
    }

    void fun2() {
        std::lock_guard<std::recursive_mutex> lk(m);
        std::thread::id this_id = std::this_thread::get_id();
        std::cout << "fun2 Thread Id : " << this_id << std::endl;

        shared = "fun2";
        std::cout << "in fun2, shared variable is now " << shared << std::endl;

        // fun2가 m을 이미 잠근 상태에서 fun1 호출 → recursive_mutex 덕분에 Deadlock 없음
        fun1();

        std::cout << "back in fun2, shared variable is " << shared << std::endl;
    }
};

int main() {
    X x;

    std::thread t1(&X::fun1, &x);
    std::thread t2(&X::fun2, &x);

    std::thread::id this_id = std::this_thread::get_id();
    std::cout << "main Thread Id : " << this_id << std::endl;

    t1.join();
    t2.join();
}
```

---

## 📜 실행 예시

```
main Thread Id : 33044
fun2 Thread Id : 8320
in fun2, shared variable is now fun2
fun1 Thread Id : 8320
in fun1, shared variable is now fun1
back in fun2, shared variable is fun1
fun1 Thread Id : 39436
in fun1, shared variable is now fun1
```

---

## 💡 동작 원리
- `std::mutex`는 **같은 스레드에서 두 번 잠그면** 데드락(Deadlock)이 발생합니다.
- `std::recursive_mutex`는 **잠금 횟수를 카운트**하여, 같은 스레드가 여러 번 잠금을 요청해도 허용합니다.
- 이 예제에서 `fun2`는 `m`을 잠근 상태에서 `fun1`을 호출합니다.  
  `fun1`도 `m`을 잠그지만, 같은 스레드이므로 데드락이 발생하지 않습니다.
- 다른 스레드가 잠금을 요청하면, 이전 스레드가 모든 잠금을 해제할 때까지 대기합니다.

---

## 📊 요약
| 뮤텍스 종류             | 같은 스레드 중첩 잠금 | 사용 목적 |
|------------------------|------------------|-----------|
| `std::mutex`           | ❌ 불가능         | 단순한 상호배제 |
| `std::recursive_mutex` | ✅ 가능           | 중첩 호출(재귀) 상황에서 안전 |

---

📌 **사용 시 주의**: `std::recursive_mutex`는 잠금 횟수를 관리하기 때문에, 불필요하게 사용하면 성능 저하가 발생할 수 있습니다.  
재귀 호출이나 중첩된 잠금이 필요한 경우에만 사용하세요.
