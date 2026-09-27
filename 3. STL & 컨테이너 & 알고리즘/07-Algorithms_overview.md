## 📚 C++ STL 알고리즘 정리

### 📌 개요

- STL의 알고리즘은 **반복자에 의해 지정된 요소 집합에 대해 수행되는 표준 연산** 입니다.  
- 모든 알고리즘은 반복자를 인자로 받으며, 컨테이너의 종류에 따라 적용 가능한 알고리즘이 다를 수 있습니다.

```cpp
#include <algorithm>
```

- 함수를 인자로 전달하는 방법:
    - **Functor** (함수 객체)
    - **Function Pointer** (함수 포인터)
    - **Lambda Expression** (람다 표현식)

- 반복자는 요소가 삭제되거나 컨테이너가 재할당되면 **무효화** 될 수 있으므로 주의가 필요합니다.

---

### 📌 `std::find`

- `find` 알고리즘은 컨테이너 내에서 **특정 요소가 처음 등장하는 위치** 를 찾습니다.
- 반환값: 해당 요소를 가리키는 반복자 또는 `end()`
- 요소 비교 시 `operator==` 사용 → 사용자 정의 타입은 오버로딩 필요

#### 기본 예제
```cpp
std::vector<int> vec{1, 2, 3};
auto itrFind = std::find(vec.begin(), vec.end(), 3);
if(itrFind != vec.end()){
    std::cout << (*itrFind) << std::endl;
}
```

#### 사용자 정의 타입 예제
```cpp
class Player {
private:
    std::string name;
    int val1 {0};
    int val2 {0};
public:
    Player(const std::string &name, int val1, int val2);
    bool operator==(const Player& other) const;
};

Player::Player(const std::string &name, int val1, int val2)
    : name(name), val1(val1), val2(val2) {}

bool Player::operator==(const Player &other) const {
    return name == other.name;
}

std::vector<Player> teams = {
    {"Hello", 100, 12},
    {"jhjeong", 1, 2}
};

Player p{"Hello", 20 ,30};
auto loc = std::find(teams.begin(), teams.end(), p);
if(loc != teams.end()){
    std::cout << "Founded" << std::endl;
}
```



### 📌 `std::for_each`

- 컨테이너의 **각 요소에 대해 주어진 함수를 호출** 합니다.

#### 1) Functor 사용
```cpp
struct SquareRoot {
    void operator()(int x) {
        std::cout << x * x << " ";
    }
};

std::array<int, 5> arr = {1, 2, 3, 4, 5};
std::for_each(arr.begin(), arr.end(), SquareRoot());
// 출력: 1 4 9 16 25
```

#### 2) 함수 포인터 사용
```cpp
void func(int x) {
    std::cout << x * x << " ";
}
std::vector<int> vec{1, 2, 3};
std::for_each(vec.begin(), vec.end(), func);
```

#### 3) 람다 사용
```cpp
std::vector<int> vec{1, 2, 3};
std::for_each(vec.begin(), vec.end(), [](int x) { 
    std::cout << x * x << " "; 
});
```


### ➕ `std::accumulate`

- 누산(합산) 또는 커스텀 연산을 수행합니다. `<numeric>` 헤더 필요.

```cpp
#include <numeric>
```

#### 기본 사용
```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
int sum = std::accumulate(v.begin(), v.end(), 0);
std::cout << sum << std::endl; // 15
```

#### 곱 계산 (람다)
```cpp
int product = std::accumulate(v.begin(), v.end(), 1, [](int a, int b) {
    return a * b;
});
```

#### 곱 계산 (`std::multiplies` 사용)
```cpp
int product1 = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
```

#### 문자열 연결
```cpp
auto dash_fold = [](std::string a, int b) {
    return std::move(a) + '-' + std::to_string(b);
};
std::string s = std::accumulate(std::next(v.begin()), v.end(), std::to_string(v[0]), dash_fold);
std::cout << s << std::endl; // 1-2-3-4-5
```

#### 역순 문자열 연결
```cpp
std::string rs = std::accumulate(std::next(v.rbegin()), v.rend(),
                                 std::to_string(v.back()), dash_fold);
std::cout << rs << std::endl; // 5-4-3-2-1
```


### 📌 요약

- **find**: 요소 검색 (`operator==` 필요)
- **for_each**: 각 요소에 대해 연산 수행 (Functor, 함수 포인터, 람다)
- **accumulate**: 누산 연산 (합, 곱, 문자열 등)
- 모든 알고리즘은 **반복자** 를 인자로 사용
- 반복자 무효화와 예외 처리에 주의

---
