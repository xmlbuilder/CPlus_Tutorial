# Memoize 구현 코드

## 🛠️ C++에서 Memoization Decorator 설계 아이디어
### 1. 템플릿 기반 래퍼 클래스
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


### 2. 일반화된 설계
- 인자 타입이 다양할 수 있으므로 std::tuple과 std::hash를 활용해 범용화.
- std::apply를 사용하면 가변 인자 함수도 처리 가능.
- 캐시 정책(LRU, TTL 등)을 추가하면 Python의 functools.lru_cache와 유사하게 확장 가능.

### 3. Decorator 패턴과 결합
- memoize를 데코레이터 함수로 만들어, 다른 데코레이터(예: 로깅, 성능 측정)와 체인처럼 연결할 수 있습니다.
```cpp
auto decorated = log_decorator(memoize(slow_add));
```
## 📌 설계 포인트
- Key 설계: 인자를 어떻게 캐싱 키로 만들지 → tuple + hash 조합
- Cache 정책: 단순 map vs LRU 캐시
- Thread-safety: 멀티스레드 환경이면 std::mutex 필요
- 조합 가능성: 다른 데코레이터와 체인으로 연결할 수 있도록 함수 객체 반환

---

## Overview of the memoization wrapper and its flow
이 코드는 특정 함수 호출 결과를 캐시에 저장해 같은 입력에 대해 재계산을 피하는 **메모이제이션** 을 C++로 구현한 예시입니다.  
현재 구현은 인자 두 개가 int인 함수에 맞춰져 있고, 키로 std::tuple<int, int>를 사용합니다.  
아래에서 각 부분을 단계별로 설명하고, 쓰인 기법과 개선 포인트까지 정리합니다.

### Code walkthrough, step by step
```cpp
#include <iostream>
#include <functional>
#include <unordered_map>
#include <tuple>
```
- Headers:
- iostream: 입출력에 사용되는 스트림 지원.
- functional: 함수 객체 및 유틸(여기선 std::declval에 대비해 포함했지만 실제론 <utility>가 필요).
- unordered_map: 해시 기반 딕셔너리 컨테이너.
- tuple: 여러 값을 묶는 간단한 구조체.
```cpp
// 해시 함수: tuple을 key로 쓰기 위해 필요
struct TupleHash {
    template <class T1, class T2>
    std::size_t operator()(const std::tuple<T1, T2>& t) const {
        return std::hash<T1>()(std::get<0>(t)) ^ std::hash<T2>()(std::get<1>(t));
    }
};
```
- 커스텀 해시 functor:
- 목적: std::unordered_map는 기본적으로 std::tuple에 대한 해시를 제공하지 않을 수 있으므로, 튜플을 키로 쓰려면 해시를 정의.
- 템플릿 멤버 함수: T1, T2 타입을 일반화하여 다양한 튜플을 처리.
- 해싱 방식: 각 요소를 std::hash로 해싱 후 XOR(^)로 결합. 간단하지만 충돌 저항성은 강하지 않으므로 더 나은 조합 기법을 고려할 수 있음(아래 개선에서 설명).
```cpp
template <typename Func>
auto memoize(Func f) {
    using Arg1 = int;  // 예시: 인자가 int 두 개일 때
    using Arg2 = int;
    using Result = decltype(f(std::declval<Arg1>(), std::declval<Arg2>()));
```
- 템플릿 함수:
- 목적: 임의의 함수 f를 받아 메모이제이션된 래퍼를 반환.
- 타입 별칭: 현재는 인자 타입을 고정(int, int). Result는 f(int,int)의 반환 타입을 decltype로 추론.
- std::declval: 실제 호출 없이 **해당 인자를 넣었을 때의 반환 타입** 을 컴파일 타임에 추론하기 위한 유틸.
```cpp
    std::unordered_map<std::tuple<Arg1, Arg2>, Result, TupleHash> cache;
```
- 캐시 컨테이너:
- 키: (Arg1, Arg2)를 담은 std::tuple.
- 값: 함수 결과(Result).
- 해시: TupleHash로 튜플을 해싱해 버킷 결정.
```cpp
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

- 람다 반환:
- 캡처: f와 cache를 값으로 캡처.
- mutable: 람다 본문에서 cache를 수정할 수 있도록 필요.
- 값 캡처는 기본적으로 const이기 때문에 mutable이 없으면 `cache[key] = result;` 가 불가능.
- 키 생성: std::make_tuple(a, b).
- 캐시 조회: cache.find(key)로 존재 여부 확인. 존재하면 재계산 없이 반환.
- 미스 처리: f(a,b)로 계산 후, 캐시에 저장하고 반환.
```cpp
int slow_add(int a, int b) {
    std::cout << "Computing...\n";
    return a + b;
}
```
- 데모 함수:
- 의미: 계산이 **느린** 척을 출력으로 표현. 실제로는 단순 덧셈.
```cpp
int main() {
    auto fast_add = memoize(slow_add);

    std::cout << fast_add(3, 4) << "\n"; // Computing...
    std::cout << fast_add(3, 4) << "\n"; // Cache hit!
}
```
- 실행 흐름:
- 첫 호출: 캐시 미스 → “Computing...” 출력 → 7 저장 및 반환.
- 두 번째 동일 호출: 캐시 히트 → 곧바로 저장된 7 반환.

### Techniques used in this code
- Templates and type deduction
  - 템플릿 함수: memoize(Func f)는 다양한 함수 객체를 받기 위해 템플릿 사용.
  - decltype + std::declval: 실제로 호출하지 않고 반환 타입을 컴파일 타임에 추론.
  - 제네릭 해시 functor: TupleHash::operator()는 템플릿으로 구성해 다양한 튜플 요소 타입 처리.
- Lambda with value capture and mutable
  - 값 캡처: f와 cache가 람다 내부 복사본으로 존재. 외부 수명과 무관하게 안전.
  - mutable 람다: 값 캡처된 멤버를 수정하려면 mutable 필요.
  - 반환 타입 명시: -> Result로 반환 타입을 명시해 가독성 향상.
- Unordered map and custom hashing
  - std::unordered_map: 평균 O(1) 조회/삽입 성능.
  - 커스텀 해시: 튜플을 키로 쓰기 위해 사용자 정의 해시 제공.
  - 키 구성: std::tuple로 두 인자를 하나의 키로 압축.
- Tuple utilities
  - std::get, std::make_tuple: 튜플 요소 접근 및 생성.
  - 유연성: 튜플은 인자 수가 늘어나도 확장하기 쉬움(단, 해시도 함께 확장 필요).

### Execution flow detail
- 초기화: memoize(slow_add) 호출 시, 내부에서 빈 캐시(unordered_map) 생성 후 이를 값으로 캡처하는 람다를 반환.
- 첫 호출 fast_add(3,4): 키 (3,4)를 만들고 cache.find → 없음. slow_add(3,4) 호출하여 7 계산, 캐시에 저장.
- 두 번째 호출 fast_add(3,4): 같은 키 조회 → 존재. “Cache hit!” 출력 후 캐시된 7 반환.
- 상태 유지: 람다의 값 캡처로 인해 fast_add가 가진 내부 캐시는 그 람다 인스턴스의 수명 동안 유지됨.

### Complexity and behavior
- 시간 복잡도:
  - 캐시 조회/삽입: 평균 O(1) (해시 품질 및 로드 팩터에 따라).
  - 미스 시 계산 비용: 원 함수 f의 비용 + 삽입 비용.
- 공간 복잡도:
  - 캐시 크기: 호출된 서로 다른 인자 조합 수에 비례.
- 동일 인자 재사용:
  - 효과: 반복 호출에서 계산 비용 제거.
- 콜드 스타트:
  - 첫 호출은 항상 원 함수 계산 수행.

### Limitations and pitfalls
- 고정 인자 타입: 현재 Arg1, Arg2를 int로 고정. 범용성 부족.
- 해시 결합 방식: 단순 XOR은 충돌에 취약. 더 나은 해시 결합이 필요.
- 값 캡처 비용: 큰 캐시를 값으로 캡처하면 복사 비용이 큼. 여기선 람다 생성 시 한 번 복사되지만, 의도적으로 여러 인스턴스가 생기면 메모리 사용이 늘 수 있음.
- 스레드 안전성: 멀티스레드 환경에서는 unordered_map 접근 보호가 필요(std::mutex).
- 부작용 함수: f가 부작용을 가지면 동일 인자에 대해 캐싱이 의미가 달라질 수 있음.
- 메모리 무제한 성장: LRU/TTL 등의 정책 부재로 캐시가 무한 성장 가능.


---

## 개선된 코드
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

## 📘 Memoize 구현 점검 및 문서화
### 1. 문제 원인
- 초기 구현에서 캐시가 동작하지 않았던 이유는 std::string과 const char* 비교 문제였습니다.
- unordered_map 키로 const char*를 쓰면 문자열 내용이 아니라 포인터 주소를 비교합니다.
- 따라서 "Hello"라는 리터럴을 두 번 넣어도 서로 다른 주소로 인식되어 캐시 히트가 발생하지 않았습니다.

### 2. 해결 방법: KeyCanonical
- 이를 해결하기 위해 KeyCanonical 템플릿을 도입했습니다.
- 역할: 함수 인자로 들어오는 다양한 문자열 표현(const char*, char*, 배열 리터럴 등)을 항상 std::string으로 변환합니다.
- 예시:
```cpp
template <>
struct KeyCanonical<const char*> {
    using type = std::string;
    static type make(const char* s) {
        return std::string(s ? s : "");
    }
};
```
- 결과: 캐시 키가 항상 std::string으로 정규화되어 내용 비교가 가능해집니다.

### 3. TupleHashAny
- 키는 여러 인자를 묶은 std::tuple입니다.
- TupleHashAny는 튜플의 각 요소를 해시하여 결합합니다.
- std::decay_t를 사용해 레퍼런스/const를 제거하고 값 타입으로 해시합니다.
- 덕분에 (std::string, Person) 같은 복합 키도 안정적으로 해시됩니다.

### 4. Memoize 클래스 구조
- Func func: 원래 함수 저장.
- operator():
  - Key = std::tuple<KeyCanonicalT<Args>...>
    - 인자를 KeyCanonical로 정규화해 튜플 키 생성.
  - Result = std::invoke_result_t<Func&, Args...>
    -  함수 반환 타입 자동 추론.
    - static std::unordered_map<Key, Result, TupleHashAny> cache;
  - 호출 간 유지되는 캐시.
    - 캐시 조회:
      - 있으면 "Cache hit!" 출력 후 반환.
      - 없으면 "Computing..." 출력 후 계산, 캐시에 저장.

### 5. Person 구조체
- operator== 정의: 이름과 나이가 같으면 동일.
- std::hash<Person> 특수화: name과 age를 해시 결합.
- 덕분에 Person을 캐시 키로 사용할 수 있습니다.

### 6. 실행 흐름
- greet 함수
  - 첫 호출: "Hello", alice → 캐시 미스 → "Computing..." → 결과 저장.
  - 두 번째 동일 호출: 캐시 히트 → "Cache hit!".
  - "Hi", bob도 동일한 흐름.
- slow_add 함수
  - (1,2) 첫 호출: 캐시 미스 → "Computing..." → 결과 저장.
  - (1,2) 두 번째 호출: 캐시 히트.
  - (2,3)도 동일한 흐름.

### 7. 출력 결과
```
---- greet ----
Computing...
Hello Alice (30)
Cache hit!
Hello Alice (30)
Computing...
Hi Bob (25)
Cache hit!
Hi Bob (25)
---- add ----
Computing...
[slow_add 실행]
3
Cache hit!
3
Computing...
[slow_add 실행]
5
Cache hit!
5
```


### 8. 장점
- 범용성: 어떤 함수든 감쌀 수 있음 (greet, slow_add 등).
- 안정성: 문자열 인자를 항상 std::string으로 변환해 캐시 히트가 정확히 발생.
- 확장성: 다른 사용자 정의 타입도 operator==와 std::hash만 정의하면 키로 사용 가능.

## 📌 결론
- 캐시가 안 되던 원인은 문자열 리터럴(const char*) 비교 문제였고, 이를 KeyCanonical로 해결했습니다.
- 현재 구현은 범용 Memoize 클래스로, 문자열/사용자 정의 타입/숫자 등 다양한 인자를 지원하며 캐시가 정상적으로 동작합니다.

---

## 📌 std::invoke_result_t란?
- 정의:
  - C++17에서 도입된 타입 트레이트(type trait)입니다.  
  - 특정 함수(또는 호출 가능한 객체)를 주어진 인자 타입으로 호출했을 때 반환되는 타입을 컴파일 시점에 추론해 줍니다.
- 형식:
```cpp
std::invoke_result_t<F, Args...>
```
- 여기서
  - F = 함수 타입 또는 호출 가능한 객체(callable)
  - Args... = 함수에 전달할 인자 타입들
- 동작:
```cpp
`std::invoke_result<F, Args...>::type` 의 축약형(alias template)이 바로 `std::invoke_result_t<F, Args...>` 입니다.
```
## ✨ 예시
```cpp
#include <type_traits>
#include <string>
#include <iostream>

int add(int a, int b) { return a + b; }

struct Greeter {
    std::string operator()(const std::string& name) {
        return "Hello " + name;
    }
};
```
```cpp
int main() {
    // 함수 add(int,int)의 반환 타입 추론
    using AddResult = std::invoke_result_t<decltype(add), int, int>;
    static_assert(std::is_same_v<AddResult, int>);

    // 함수 객체 Greeter의 operator()(std::string)의 반환 타입 추론
    using GreetResult = std::invoke_result_t<Greeter, std::string>;
    static_assert(std::is_same_v<GreetResult, std::string>);

    std::cout << "AddResult is int\n";
    std::cout << "GreetResult is std::string\n";
}
```

### 출력:
```
AddResult is int
GreetResult is std::string
```


## 📌 왜 쓰는가?
- 범용 코드 작성:
  - 템플릿에서 함수 반환 타입을 미리 알 수 없을 때, std::invoke_result_t로 안전하게 추론합니다.
- Memoize 구현에서 사용:
```cpp
Result = std::invoke_result_t<Func&, Args...>;
```
- Func를 Args...로 호출했을 때 반환되는 타입을 자동으로 Result로 지정합니다.
- 덕분에 Func가 int를 반환하든, std::string을 반환하든, Memoize가 자동으로 맞춰집니다.

### 👉 정리하면:
- std::invoke_result_t는 **이 함수(또는 호출 객체)를 이런 인자로 호출했을 때 반환되는 타입** 을 컴파일러가 알려주는 도구입니다.

