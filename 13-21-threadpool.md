# 🧠 Thread Pool이란?
스레드 풀은 일정 수의 스레드를 미리 생성해두고, 작업이 들어올 때마다 스레드에게 할당하는 방식입니다.  이렇게 하면:
- 스레드 생성/소멸 비용 절감
- 자원 낭비 방지
- 병렬 작업 처리 효율 향상

## ⚙️ std::jthread 기반 Thread Pool 핵심 구성
- 작업 큐: 처리할 작업들을 저장
- 스레드 그룹: std::jthread로 구성된 워커 스레드들
- 동기화 도구: std::mutex, std::condition_variable로 작업 큐 보호 및 알림

## 🧪 샘플 코드: 간단한 Thread Pool
```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <jthread>

class ThreadPool {
public:
    ThreadPool(size_t threadCount) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) {
                    std::function<void()> task;

                    {
                        std::unique_lock lock(queueMutex);
                        condition.wait(lock, [this, &stoken] {
                            return !tasks.empty() || stoken.stop_requested();
                        });

                        if (stoken.stop_requested()) break;
                        if (!tasks.empty()) {
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                    }

                    if (task) task();
                }
            });
        }
    }

    ~ThreadPool() {
        // jthread 자동으로 join + stop 요청됨
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard lock(queueMutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

private:
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
};

```
## 사용 예시:
```cpp
int main() {
    ThreadPool pool(4); // 4개의 워커 스레드

    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " is running on thread " 
                      << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(3)); // 작업 대기
}
```


✅ std::jthread의 장점
| 기능 | 설명 | 
|------|-----|
| 자동 join | 소멸 시 자동으로 join() 호출됨 | 
| stop_token 지원 | 안전한 종료 요청 처리 가능 | 
| RAII 기반 | 예외 발생 시에도 자원 누수 없음 | 



##💡 확장 아이디어
- std::future와 std::promise를 활용해 결과 반환
- 작업 우선순위 큐 구현
- 동적 스레드 수 조절 기능 추가
- std::packaged_task로 작업 래핑

---



# 1. std::future를 반환하는 ThreadPool
이 버전은 작업을 std::packaged_task로 래핑해서 std::future를 반환합니다.   호출자는 작업 결과를 비동기적으로 받을 수 있어요.
```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <jthread>

class ThreadPool {
public:
    ThreadPool(size_t threadCount) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) {
                    std::function<void()> task;

                    {
                        std::unique_lock lock(queueMutex);
                        condition.wait(lock, [this, &stoken] {
                            return !tasks.empty() || stoken.stop_requested();
                        });

                        if (stoken.stop_requested()) break;
                        if (!tasks.empty()) {
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                    }

                    if (task) task();
                }
            });
        }
    }

    ~ThreadPool() = default;

    template<typename Func>
    auto enqueue(Func&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(f));
        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard lock(queueMutex);
            tasks.push([task]() { (*task)(); });
        }

        condition.notify_one();
        return result;
    }

private:
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
};
```

사용 예시:
```cpp
int main() {
    ThreadPool pool(4);

    auto future = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    });

    std::cout << "Waiting for result...\n";
    std::cout << "Result: " << future.get() << "\n";
}
```


## 🛑 2. std::stop_source를 활용한 graceful shutdown

std::stop_source는 std::jthread와 함께 쓰면 스레드에게 종료 요청을 안전하게 전달할 수 있어요.
```cpp
#include <iostream>
#include <jthread>
#include <chrono>

void worker(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::cout << "Working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Graceful shutdown\n";
}

int main() {
    std::stop_source stopSrc;
    std::jthread t(worker, stopSrc.get_token());

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Requesting stop...\n";
    stopSrc.request_stop(); // 안전하게 종료 요청

    t.join(); // jthread는 자동 join이지만 명시적으로도 가능
}
```

## 📌 출력 예시:
Working...
Working...
Working...
Requesting stop...
Graceful shutdown



## ✨ 요약
| 기능 | 설명 |
|------|-----| 
| std::future | 작업 결과를 비동기적으로 반환 | 
| std::packaged_task | 함수 래핑 + future 연결 | 
| std::stop_source | 스레드에게 종료 요청 전달 | 
| std::jthread | 자동 join + stop_token 지원 | 

-----



