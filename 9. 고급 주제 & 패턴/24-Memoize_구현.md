## 📘 Memoize 구현

### 📌 Memoization Decorator 설계 아이디어
#### 🔹 1. 템플릿 기반 래퍼 클래스
- 함수 객체(std::function)를 감싸고,
- 입력 인자를 키로 해서 결과를 std::unordered_map에 저장합니다.
- 동일한 인자가 들어오면 캐시된 값을 반환합니다.
  
```cpp
#include <iostream>
#include <functional>
#include <unordered_map>
#include <tuple>

// 해시 함수: tuple을 key로 쓰기 위해 필요
struct TupleHash {
    template <class T1, class T2>
    std::size_t operator()(const std::tuple<T1, T2>& t) const {
        return std::hash<T1>()(std::get<0>(t)) ^ std::hash<T2>()(std::get<1>(t));
    }
};
```
```cpp
template <typename Func>
auto memoize(Func f) {
    using Arg1 = int;  // 예시: 인자가 int 두 개일 때
    using Arg2 = int;
    using Result = decltype(f(std::declval<Arg1>(), std::declval<Arg2>()));

    std::unordered_map<std::tuple<Arg1, Arg2>, Result, TupleHash> cache;

    return [f, cache](Arg1 a, Arg2 b) mutable -> Result {
        auto key = std::make_tuple(a, b);
        auto it = cache.find(key);
        if (it != cache.end()) {
            std::cout << "Cache hit!\n";
            return it->second;
        }
        auto result = f(a, b);
        cache[key] = result;
        return result;
    };
}
```
```cpp
int slow_add(int a, int b) {
    std::cout << "Computing...\n";
    return a + b;
}
```
```cpp
int main() {
    auto fast_add = memoize(slow_add);
    std::cout << fast_add(3, 4) << "\n"; // Computing...
    std::cout << fast_add(3, 4) << "\n"; // Cache hit!
}
```

#### 🔹 2. 일반화된 설계
- 인자 타입이 다양할 수 있으므로 std::tuple과 std::hash를 활용해 범용화.
- std::apply를 사용하면 가변 인자 함수도 처리 가능.
- 캐시 정책(LRU, TTL 등)을 추가하면 Python의 functools.lru_cache와 유사하게 확장 가능.

#### 🔹 3. Decorator 패턴과 결합
- memoize를 데코레이터 함수로 만들어, 다른 데코레이터(예: 로깅, 성능 측정)와 체인처럼 연결할 수 있습니다.
```cpp
auto decorated = log_decorator(memoize(slow_add));
```
### 📌 설계 포인트
- Key 설계: 인자를 어떻게 캐싱 키로 만들지 → tuple + hash 조합
- Cache 정책: 단순 map vs LRU 캐시
- Thread-safety: 멀티스레드 환경이면 std::mutex 필요
- 조합 가능성: 다른 데코레이터와 체인으로 연결할 수 있도록 함수 객체 반환

---


### 📌 개선된 코드
```cpp
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <string>
#include <functional>
#include <type_traits>
#include <utility>

// 해시 결합 함수
inline void hash_combine(std::size_t& seed, std::size_t value) {
    seed ^= value + 0x9e3779b9u + (seed << 6) + (seed >> 2);
}
```
```cpp
struct TupleHashAny {
    template <typename... Ts>
    std::size_t operator()(const std::tuple<Ts...>& t) const {
        std::size_t seed = 0;
        std::apply([&](const Ts&... elems) {
            (hash_combine(seed, std::hash<std::decay_t<Ts>>{}(elems)), ...);
        }, t);
        return seed;
    }
};
```
```cpp
template <typename T>
struct KeyCanonical {
    using type = std::decay_t<T>;
    static type make(T&& v) {
        return std::forward<T>(v);
    }
};
```
```cpp
template <>
struct KeyCanonical<const char*> {
    using type = std::string;
    static type make(const char* s) {
        return std::string(s ? s : "");
    }
};
```
```cpp
template <>
struct KeyCanonical<char*> {
    using type = std::string;
    static type make(char* s) {
        return std::string(s ? s : "");
    }
};
```
```cpp
template <std::size_t N>
struct KeyCanonical<const char(&)[N]> {
    using type = std::string;
    static type make(const char (&s)[N]) {
        return std::string(s);
    }
};
```
```cpp
template <std::size_t N>
struct KeyCanonical<char(&)[N]> {
    using type = std::string;
    static type make(const char (&s)[N]) {
        return std::string(s);
    }
};
```
```cpp
template <typename T>
using KeyCanonicalT = typename KeyCanonical<T>::type;
```
```cpp
struct Person {
    std::string name;
    int age;

    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};
```
```cpp
namespace std {
    template <>
    struct hash<Person> {
        std::size_t operator()(const Person& p) const {
            std::size_t seed = 0;
            hash_combine(seed, std::hash<std::string>{}(p.name));
            hash_combine(seed, std::hash<int>{}(p.age));
            return seed;
        }
    };
}
```
```cpp
template <typename Func>
class Memoize {
    Func func;

public:
    explicit Memoize(Func f) : func(std::move(f)) {}

    template <typename... Args>
    auto operator()(Args&&... args) {
        using Key    = std::tuple<KeyCanonicalT<Args>...>;
        using Result = std::invoke_result_t<Func&, Args...>;

        static std::unordered_map<Key, Result, TupleHashAny> cache;

        Key key{ KeyCanonical<Args>::make(std::forward<Args>(args))... };

        if (auto it = cache.find(key); it != cache.end()) {
            std::cout << "Cache hit!\n";
            return it->second;
        }

        std::cout << "Computing...\n";
        Result res = func(std::forward<Args>(args)...);
        cache.emplace(std::move(key), res);
        return res;
    }
};
```
```cpp
std::string greet(const std::string& prefix, const Person& p) {
    return prefix + " " + p.name + " (" + std::to_string(p.age) + ")";
}
```
```cpp
int slow_add(int a, int b) {
    std::cout << "[slow_add 실행]\n";
    return a + b;
}
```
```cpp
int main() {
    Memoize memo_greet(greet);
    Memoize memo_add(slow_add);

    Person alice{"Alice", 30};
    Person bob{"Bob", 25};

    std::cout << "---- greet ----\n";
    std::cout << memo_greet("Hello", alice) << "\n"; // Computing...
    std::cout << memo_greet("Hello", alice) << "\n"; // Cache hit!
    std::cout << memo_greet("Hi", bob) << "\n";      // Computing...
    std::cout << memo_greet("Hi", bob) << "\n";      // Cache hit!

    std::cout << "---- add ----\n";
    std::cout << memo_add(1, 2) << "\n"; // Computing...
    std::cout << memo_add(1, 2) << "\n"; // Cache hit!
    std::cout << memo_add(2, 3) << "\n"; // Computing...
    std::cout << memo_add(2, 3) << "\n"; // Cache hit!
}
```
---


