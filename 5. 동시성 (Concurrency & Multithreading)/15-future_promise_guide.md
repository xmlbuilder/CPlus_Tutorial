## 📘 `std::future` 완전 정리 — `promise` / `packaged_task` / `async`

> 비동기 작업의 **결과값** 또는 **예외**를 안전하게 전달하는 표준 도구들:  
> `std::promise`, `std::future`, `std::packaged_task`, `std::async`


### 📌 왜 `future`인가?
- 메인 스레드 A가 다른 스레드 B에게 일을 시키고 **나중에 결과를 받고 싶을 때** 사용합니다.  
- `std::future<T>`는 비동기 작업의 **결과(값/예외)의 소유권** 을 가지며, `get()` 호출 시 결과가 준비될 때까지 **블록** 합니다.


### 📌 `future`를 얻는 3가지 경로

- 1) **`std::promise<T>` → `get_future()`**  
   - 임의의 스레드/함수에서 값을 **직접 set**하거나 **예외 설정**  
- 2) **`std::packaged_task<R(Args...)>`**  
   - Callable(함수/람다/펑터)의 **리턴값이 자동으로 set_value** 됨. 예외는 자동으로 **set_exception** 됨  
- 3) **`std::async`**  
   - 고수준 API. 스레드 생성과 결과 전달을 한 번에 처리



### 📌 `packaged_task` 동작 구조 (ASCII 다이어그램)

```md
get() --> [future] --> value
                      ^
                      |
                [promise] <--- set_value(x) <--- return x (Callable)
                      |
                      +--- set_exception(px) <--- throw x (Callable)
```

### 📌 예제 1 — `packaged_task` 기본

```cpp
#include <iostream>
#include <thread>
#include <future>

void future_test1() {
    std::packaged_task<int()> task([] {
        int total = 0;
        for (int i = 0; i < 10'000'000; ++i) total = i;
        return 7;
    });

    std::future<int> result = task.get_future();
    std::thread(std::move(task)).detach();

    std::cout << "waiting ...";
    result.wait();
    std::cout << "done!";
    std::cout << result.get() << '';
}
```


### 📌 예제 2 — 예외 전파

```cpp
#include <future>
#include <thread>
#include <iostream>
#include <stdexcept>

int might_fail() {
    throw std::runtime_error("boom");
}

int main() {
    std::packaged_task<int()> task(might_fail);
    std::future<int> f = task.get_future();

    std::thread t(std::move(task));
    t.join();

    try {
        int v = f.get();
        std::cout << v << '';
    } catch (const std::exception& e) {
        std::cerr << "caught: " << e.what() << '';
    }
}
```

### 📌 예제 3 — `wait_for()` 타임아웃

```cpp
#include <future>
#include <thread>
#include <iostream>
#include <chrono>

int work() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
}

int main() {
    std::packaged_task<int()> task(work);
    auto f = task.get_future();
    std::thread th(std::move(task));

    while (f.wait_for(std::chrono::milliseconds(300)) != std::future_status::ready) {
        std::cout << "." << std::flush;
    }
    std::cout << "\nresult: " << f.get() << '\n';
    th.join();
}
```


### 📌 예제 4 — `shared_future`

```cpp
#include <future>
#include <thread>
#include <iostream>

int main() {
    std::promise<int> p;
    std::future<int> f = p.get_future();
    std::shared_future<int> sf = f.share();

    std::thread producer([&]{ p.set_value(99); });

    std::thread c1([sf]{ std::cout << "c1: " << sf.get() << '\n'; });
    std::thread c2([sf]{ std::cout << "c2: " << sf.get() << '\n'; });

    producer.join();
    c1.join();
    c2.join();
}
```

---

### 📌 예제 5 — `std::async` 런치 정책

```cpp
#include <future>
#include <iostream>
#include <chrono>

int work() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 7;
}

int main() {
    auto f1 = std::async(work);
    auto f2 = std::async(std::launch::async, work);
    auto f3 = std::async(std::launch::deferred, work);

    std::cout << f1.get() + f2.get() + f3.get() << '\n';
}
```
```cpp
{

   auto f2 = std::async(std::launch::async, work);
   
   // work()와 동시에 진행할 수 있는 다른 작업
   DoSomethingElse();
   
   int result = f2.get();
}
```

```cpp
{
   auto f3 = std::async(std::launch::deferred, work);
   
   // 이 시점에는 work()가 실행되지 않음
   DoSomethingElse();
   
   int result = f3.get(); // 여기서 현재 스레드가 work() 실행
}
```

#### 🔹 코드 설명
|코드	|실행 방식|	실행 시점|
|-----|--------|------------|
| std::async(work) | 구현이 async 또는 deferred 선택| 선택된 정책에 따라 결정|
| launch::async | 별도의 실행 스레드에서 실행 | get()을 기다리지 않고 실행 시작|
| launch::deferred| 처음 get() 또는 wait()를 | 호출한 스레드에서 실행|

### 📌 정리
- `promise`/`future`는 값·예외 전달의 기본 도구
- `packaged_task`는 Callable 결과를 future에 연결
- `async`는 스레드 실행과 결과 수집을 한 번에 처리

---

