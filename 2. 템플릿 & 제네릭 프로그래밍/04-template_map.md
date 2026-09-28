## 📘 template과 std::map 연동 정리
### 1️⃣ 기본 구조
```cpp
template<typename Key, typename Value, typename F>
void update(std::map<Key, Value>& m, F foo) {
    for (auto keyValue : m) {
        std::cout << foo(keyValue.first) << std::endl;
    }
}
```

- Key, Value는 std::map의 키와 값 타입
- F는 **함수 객체(람다, 함수 포인터, functor 등)** 를 받아서 키에 대해 처리
- keyValue.first는 키, keyValue.second는 값

### 2️⃣ 구조 분해 방식 (C++17 이상)
```cpp
for (const auto& [key, value] : m) {
    std::cout << foo(key) << std::endl;
}
```
- 구조 분해를 통해 key와 value를 직접 변수로 받음
- 가독성 향상, 명확한 의도 표현

### 3️⃣ 사용 예시
```cpp
std::map<std::string, long> m {
    {"a", 1},
    {"b", 2},
    {"c", 3},
};

update(m, [](std::string key) {
    return key;
});
```

- update는 map의 모든 키에 대해 람다를 적용
- 출력 결과:
```
a
b
c
```


### 4️⃣ 다양한 람다 활용 예시
#### 🔹 키를 대문자로 변환
```cpp
update(m, [](const std::string& key) {
    std::string upper = key;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
});
```

#### 🔹 키와 값을 함께 출력
```cpp
template<typename Key, typename Value, typename F>
void update(std::map<Key, Value>& m, F foo) {
    for (const auto& [key, value] : m) {
        std::cout << foo(key, value) << std::endl;
    }
}

update(m, [](const std::string& key, long value) {
    return key + ": " + std::to_string(value);
});
```


### 5️⃣ 보강: const std::map<Key, Value>&로 받기
- std::map을 수정하지 않는다면 const 참조로 받는 것이 안전
```cpp
template<typename Key, typename Value, typename F>
void update(const std::map<Key, Value>& m, F foo) {
    for (const auto& [key, value] : m) {
        std::cout << foo(key) << std::endl;
    }
}
```


### 📌 요약
| 요소 | 설명 |
|-----|------|
| template<typename Key, Value, F> | std::map과 함수 객체를 범용적으로 처리 | 
| auto& [key, value] | 구조 분해로 키/값 직접 접근 | 
| foo(key) | 키에 대해 사용자 정의 함수 적용 | 
| const std::map& | 수정이 없을 경우 안전한 참조 | 

----



