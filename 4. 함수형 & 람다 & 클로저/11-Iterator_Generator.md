## 📘iterator 와 generator 결합

- Python에서 __iter__()와 yield를 결합한 구조처럼, C에서도 iterator와 coroutine을 결합한 구조를 만들 수 있음.
- 다만 C에서는 yield 대신 co_yield, __iter__() 대신 begin()/end() 또는 next()를 구현해야 함
- promise_type, coroutine_handle, suspend point 같은 저수준 요소를 직접 다뤄야 합니다.

### 📌 목표: Python 스타일의 generator 반복자
```cpp
WordSplitter ws("Do today what you could to tomorrow");
for (auto word : ws) {
    std::cout << word << std::endl;
}
``` 


### 📌 구현: coroutine 기반 반복자
```cpp
#include <coroutine>
#include <string>
#include <sstream>
#include <iostream>

class WordSplitter {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::string current;
        std::stringstream ss;

        auto get_return_object() {
            return WordSplitter{handle_type::from_promise(*this)};
        }

        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void return_void() {}
        void unhandled_exception() { std::exit(1); }

        auto yield_value(std::string word) {
            current = word;
            return std::suspend_always{};
        }
    };

    WordSplitter(handle_type h) : coro(h) {}
    ~WordSplitter() { if (coro) coro.destroy(); }

    struct iterator {
        handle_type coro;

        iterator(handle_type h) : coro(h) {}

        std::string operator*() const {
            return coro.promise().current;
        }

        iterator& operator++() {
            coro.resume();
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return !coro.done();
        }
    };

    iterator begin() {
        coro.resume();
        return iterator{coro};
    }

    iterator end() {
        return iterator{nullptr};
    }

private:
    handle_type coro;
};

// coroutine 함수
WordSplitter split_words(std::string text) {
    std::stringstream ss(text);
    std::string word;
    while (ss >> word)
        co_yield word;
}
```


### 📌 사용 예
```cpp
int main() {
    for (auto word : split_words("Do today what you could to tomorrow")) {
        std::cout << word << std::endl;
    }
}
```

#### 🔹 출력:
```
Do  
today  
what  
you  
could  
to  
tomorrow
```


### 📌 요점
| Python 구조       | C++ 대응 구조              |
|-------------------|----------------------------|
| `__iter__()`      | `begin()` / `end()` 반복자 |
| `yield`           | `co_yield`                 |
| generator object  | coroutine handle + promise |



### 📌 그럼에도 불구하고 쓰는 이유
| 이유                     | 설명                                      |
|--------------------------|-------------------------------------------|
| 성능 최적화              | 스택 없는 코루틴으로 오버헤드 최소화         |
| 흐름 제어력              | `suspend`/`resume` 지점을 직접 제어 가능     |
| 비동기 처리              | `co_await`로 논블로킹 로직 구현 가능         |
| 라이브러리 통합성        | Boost, cppcoro, Folly 등과 쉽게 연동 가능     |
| 표현력 향상              | 복잡한 상태 머신을 간결한 코드로 표현 가능    |


### 📌 현실적인 해결책
#### 🔹 1. 직접 쓰지 말고 Wrapper나 라이브러리에 맡기자
```cpp
- cppcoro::generator<T>
- folly::coro::Task<T>
- asio::awaitable<T>
```
- 이런 것들은 내부의 복잡한 구조를 감추고, 사용자는 co_yield, co_await만 쓰면 됨
#### 🔹 2. DSL처럼 추상화하자
```
co_await move_to(target);
co_await wait(2s);
co_await play_animation("attack");
```

→ 내부는 coroutine이지만, 외부는 마치 스크립트처럼 간결하게 보이게

### 📌 요약하자면
- C++ coroutine은 직접 쓰면 고통, 잘 감싼 Wrapper나 라이브러리를 쓰면 강력한 도구가 됩니다.

----

## 📘 DSL 추상화

### 📌 목표: 게임 캐릭터의 행동을 coroutine으로 표현
```cpp
co_await move_to(target);
co_await wait(2s);
co_await play_animation("attack");
```

→ 이 코드는 캐릭터가 목표 지점으로 이동하고, 2초 기다린 뒤 공격 애니메이션을 실행하는 흐름을 coroutine으로 표현한 거예요.

### 📌 핵심 구성 요소
#### 🔹 1. co_await 가능한 타입 만들기
```cpp
struct MoveTo {
    std::string target;

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        // 비동기 이동 로직 시작
        std::cout << "Moving to " << target << std::endl;
        // 예시: 타이머나 이벤트 큐에 등록
        h.resume();  // 실제로는 이동 완료 후 resume
    }

    void await_resume() {
        std::cout << "Arrived at " << target << std::endl;
    }
};
```

#### 🔹 2. wait()도 같은 방식으로 구현
```cpp
struct Wait {
    int seconds;

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        std::cout << "Waiting for " << seconds << " seconds..." << std::endl;
        // 예시: 타이머 등록
        h.resume();  // 실제로는 시간 경과 후 resume
    }

    void await_resume() {}
};
```


#### 🔹 3. play_animation()도 마찬가지
```cpp
struct PlayAnimation {
    std::string name;

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        std::cout << "Playing animation: " << name << std::endl;
        h.resume();  // 애니메이션 끝나면 resume
    }

    void await_resume() {}
};
```


### 📌 실제 coroutine 함수
```cpp
task<void> character_behavior() {
    co_await MoveTo{"enemy"};
    co_await Wait{2};
    co_await PlayAnimation{"attack"};
}

```

→ 이 함수는 coroutine으로 동작하며, 각 단계에서 중단되고, 이벤트가 발생하면 재개됩니다.

### 📌 요약: DSL처럼 보이게 만드는 법
| 구성 요소         | 역할                                      |
|------------------|-------------------------------------------|
| `struct Action`  | `await_ready`, `await_suspend`, `await_resume` 구현 |
| `co_await`       | coroutine 흐름 제어                        |
| `task<T>`        | coroutine 반환 타입                        |
| `resume()`       | 이벤트 발생 시 coroutine 재개              |

---




