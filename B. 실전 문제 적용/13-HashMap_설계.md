## 📘 HashMap 설계 문서 (FixedSizePool 내부 포함 버전)
### 📌 1. 설계 의도
- 임베디드 환경에서 안정적이고 가벼운 Key–Value 저장소를 제공하기 위해 설계.
- SimpleFixedSizePool 내부에 포함시켜서 사용자가 풀을 직접 관리하지 않아도 되도록 단순화.
- 해시 충돌은 **체인 방식(Linked List)** 으로 처리.
- 초기화 시 사용자가 **TABLE_SIZE(버킷 개수)** 와 풀 capacity를 지정할 수 있고, 기본값도 제공.

### 📌 2. 구조 개요
#### 🔹 2.1 Node
```cpp
template <typename Key, typename Value>
struct Node {
    Key key;
    Value value;
    Node* next;
};
```

### 🔹 2.2 HashMap
```cpp
template <typename Key, typename Value>
class HashMapPool {
public:
    HashMap(size_t table_size = 64, size_t pool_capacity = 128);
    ~HashMap();

    void addItem(const Key& key, const Value& value);
    Value* findItem(const Key& key);
    void removeItem(const Key& key);

private:
    Node<Key, Value>** table;                  // 버킷 배열
    size_t capacity;                           // 버킷 개수
    SimpleFixedSizePool<Node<Key, Value>> pool; // 내부 메모리 풀

};
```

### 📌 3. 동작 방식
- 초기화
  - HashMap(table_size, pool_capacity) 호출 시 버킷 배열과 풀을 자동 생성.
  - table_size가 0이면 기본값 64 사용.
  - pool_capacity가 0이면 기본값 128 사용.
- 삽입(AddItem)
  - 해시 함수(hashKey)로 인덱스 계산 (hash % capacity).
  - 풀에서 Node 할당 → Key/Value 저장 → 버킷 리스트 맨 앞에 연결.
- 검색(FindItem)
  - 버킷 리스트 순회 → Key 문자열 비교 → 값 반환.
- 삭제(RemoveItem)
  - 버킷 리스트 순회 → Key 일치 시 제거 → Node를 풀에 반환.
- 소멸
  - ~HashMap()에서 버킷 배열 해제, 풀도 자동 Destroy().

### 📌 4. 장점
- 사용자 부담 최소화 → 풀을 직접 다루지 않고 HashMap만 사용.
- 임베디드 친화적 → 고정 길이 문자열 키, 단순 Value 타입, STL 의존성 없음.
- 예측 가능한 성능 → FixedSizePool 기반으로 일정한 할당/반환 속도.
- 유연성 → 초기화 시 TABLE_SIZE와 풀 capacity를 지정 가능, 기본값도 제공.
- 체인 방식 충돌 처리 → 단순하고 안정적.

### 📌 5. 예제 사용
```cpp
int main() {
    HashMap<std::string, int> map1(128, 256);
    map1.addItem("apple", 10);
    map1.addItem("banana", 20);
    if (auto* v = map1.findItem("banana")) printf("banana => %d\n", *v);

    HashMap<int, double> map2(64, 128);
    map2.addItem(42, 3.14);
    if (auto* v = map2.findItem(42)) printf("42 => %.2f\n", *v);

    return 0;
}
```

### 📌 결론
- 이 HashMapPool은 SimpleFixedSizePool 내부에 포함시켜서 사용자가 풀을 직접 관리하지 않아도 되며, 임베디드 환경에서 실전 투입하기에 적합합니다.
    - 단순한 인터페이스
    - 안정적인 메모리 관리
    - 충돌 처리와 성능 균형


### 📌 소스 코드 
```cpp
#pragma once

#include "fixed_size_pool.h"
#include <cstring>
#include <cstdio>
#include <new>

namespace HASH_MAP_POOL
{
    template <typename Key, typename Value>
    struct Node {
        Key key;
        Value value;
        Node* next;
    };

    template <typename Key, typename Value>
    class HashMapPool {

    public:
        HashMapPool(size_t table_size = 64, size_t pool_capacity = 128) {
            capacity = table_size == 0 ? 64 : table_size;
            table = new Node<Key, Value>*[capacity];
            memset(table, 0, sizeof(Node<Key, Value>*) * capacity);
            pool.Create(pool_capacity);
        }

        ~HashMapPool() {
            delete[] table;
            pool.Destroy();
        }

        void addItem(const Key& key, const Value& value) {
            uint32_t h = hashKey(key);
            uint32_t idx = h % capacity;

            Node<Key, Value>* n =
                pool.Construct(key, value, table[idx]);

            if (!n) {
                printf("Pool exhausted!\n");
                return;
            }

            table[idx] = n;
        }

        Value* findItem(const Key& key) {
            uint32_t h = hashKey(key);
            uint32_t idx = h % capacity;

            Node<Key, Value>* cur = table[idx];
            while (cur) {
                if (cur->key == key)
                    return &cur->value;
                cur = cur->next;
            }
            return nullptr;
        }

        void removeItem(const Key& key) {
            uint32_t h = hashKey(key);
            uint32_t idx = h % capacity;

            Node<Key, Value>* cur = table[idx];
            Node<Key, Value>* prev = nullptr;
            while (cur) {
                if (cur->key == key) {
                    if (prev) prev->next = cur->next;
                    else table[idx] = cur->next;
                    pool.DestructAndReturn(cur);
                    return;
                }
                prev = cur;
                cur = cur->next;
            }
        }

    private:
        Node<Key, Value>** table;
        size_t capacity;
        SimpleFixedSizePool<Node<Key, Value>> pool;

        static uint32_t hashKey(const Key& key)
        {
            if constexpr (std::is_same_v<Key, std::string>)
            {
                uint32_t h = 5381;

                for (unsigned char c : key)
                    h = ((h << 5) + h) + c;

                return h;
            }
            else if constexpr (std::is_same_v<Key, const char*>)
            {
                uint32_t h = 5381;

                const unsigned char* p =
                    reinterpret_cast<const unsigned char*>(key);

                while (*p)
                    h = ((h << 5) + h) + *p++;

                return h;
            }
            else
            {
                return static_cast<uint32_t>(key);
            }
        }
    };
}
```

### 📌 테스트 코드
```cpp
#include <stdio.h>
#include "hash_map_table_size.h"

int main() {

    HashMap<std::string, int> map1(128, 256);
    map1.addItem("apple", 10);
    map1.addItem("banana", 20);
    if (auto* v = map1.findItem("banana")) printf("banana => %d\n", *v);

    HashMap<int, double> map2(64, 128);
    map2.addItem(42, 3.14);
    if (auto* v = map2.findItem(42)) printf("42 => %.2f\n", *v);

    return 0;
}
```
---


