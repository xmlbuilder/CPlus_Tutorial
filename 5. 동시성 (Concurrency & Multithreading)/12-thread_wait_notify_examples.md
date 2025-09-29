# ⏳ C++ Thread Wait & Notify 예제 (`std::condition_variable`)

## 📌 개요
`std::condition_variable`은 **스레드 간의 동기화**를 위해 사용됩니다.  
한 스레드는 **조건이 충족될 때까지 대기(wait)** 하고, 다른 스레드는 **조건이 충족되었음을 알림(notify)** 으로써 대기 중인 스레드를 깨웁니다.

---

## 📂 예제 1 — Producer / Consumer (기본 형태)

### 설명
- **생산자(Producer)**: 큐에 아이템을 추가하고 `notify_one()` 호출
- **소비자(Consumer)**: 큐가 비어있을 때 `wait()`로 대기, 알림이 오면 아이템을 꺼내 처리

```cpp
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::mutex g_mutex;
std::condition_variable g_controller;
std::queue<std::string> g_itemQueue;

void pushItemToQueue() {
    std::unique_lock<std::mutex> lock(g_mutex);
    g_itemQueue.push("gameItem");
    std::cout << "pushItemToQueue()" << std::endl;
    g_controller.notify_one();
}

void saveItemToDB() {
    std::string item;
    {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_controller.wait(lock); // 큐가 채워질 때까지 대기
        item = g_itemQueue.front();
        g_itemQueue.pop();
    }
    std::cout << "Saved game item in the Database" << std::endl;
}

int main() {
    std::thread consumer(saveItemToDB);
    std::thread producer(pushItemToQueue);
    producer.join();
    consumer.join();
}
```

**실행 예시**
```
pushItemToQueue()
Saved game item in the Database
```

---

## 📂 예제 2 — 콘솔 입력과 스레드 메시지 전달

### 설명
- 메인 스레드: 콘솔에서 문자열 입력 후 메시지 전송
- 서브 스레드: `wait()` 상태로 대기하다가 알림이 오면 메시지를 출력

```cpp
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

std::mutex g_mutex;
std::condition_variable g_controller;
std::string g_msg;

std::thread th1([] {
    while (true) {
        std::unique_lock<std::mutex> lock(g_mutex);
        std::cout << "Please add message" << std::endl;
        g_controller.wait(lock);
        std::cout << "> Message from child: " << g_msg << std::endl;
    }
});

int main() {
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q") {
            th1.detach();
            break;
        }
        std::unique_lock<std::mutex> lock(g_mutex);
        g_msg = line;
        g_controller.notify_one();
    }
}
```

**실행 예시**
```
Please add message
message
> Message from child: message
Please add message
sample
> Message from child: sample
Please add message
```

---

## 📂 예제 3 — 템플릿 기반 동기화 큐

### 설명
- 제네릭 `queue<T>` 클래스
- `push()` → 값 추가 후 `notify_one()` 호출
- `pop()` → 큐가 비어있으면 `wait()`로 대기
- 최근 꺼낸 값은 `getRetVal()`로 확인 가능

```cpp
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <chrono>

template <typename T>
class queue {
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
    T retVal;

public:
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(d_mutex);
            d_queue.push_front(value);
        }
        d_condition.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(d_mutex);
        d_condition.wait(lock, [=] { return !d_queue.empty(); });
        T rc(std::move(d_queue.back()));
        d_queue.pop_back();
        retVal = rc;
        return rc;
    }

    T getRetVal() { return retVal; }
};

int main() {
    queue<int> x;
    std::thread t2(&queue<int>::pop, &x);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::thread t1(&queue<int>::push, &x, 20);
    t1.join();
    t2.join();
    std::cout << "Main Return " << x.getRetVal() << std::endl;
}
```

**실행 예시**
```
Main Return 20
```

---

## 📊 핵심 요약

| 메서드                          | 설명 |
|--------------------------------|------|
| `wait(lock)`                   | 조건이 만족될 때까지 스레드를 블록 |
| `wait(lock, predicate)`        | 조건이 참이 될 때까지 대기 |
| `notify_one()`                  | 하나의 대기 중인 스레드를 깨움 |
| `notify_all()`                  | 모든 대기 스레드를 깨움 |

💡 **주의**: `wait()` 호출 전 **반드시 `std::unique_lock<std::mutex>`를 소유**해야 합니다.
