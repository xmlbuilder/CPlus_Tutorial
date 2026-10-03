## 📘 FixedSizePool (OpenNurbs 스타일 간단 재 구현)
### 📌 소개
- 이 프로젝트는 OpenNurbs의 ON_FixedSizePool 아이디어를 기반으로 한  
**고정 크기 메모리 풀(Fixed-size memory pool)** 의 간단한 C++ 구현입니다.  
- 동일 크기의 객체를 빠르게 할당/반환할 수 있으며, 일반적인 new/delete보다 성능과 메모리 효율성이 뛰어납니다.  

### 📌 특징
- 블록 단위 할당: 한 번에 큰 블록을 받아와 내부에서 요소를 관리
- Free-list 재사용: 반환된 요소를 스택으로 관리하여 O(1) 할당/반환
- Placement new 지원: 클래스 타입 객체의 생성자/소멸자 호출 가능
- Thread-safe 버전 제공: 멀티스레드 환경에서도 안전하게 사용 가능
- 간단한 API: Create → Allocate → Return → Destroy 흐름

### 📌 구조
- FixedSizePool : raw memory 관리용 풀
- SimpleFixedSizePool<T> : 타입 안전 래퍼, placement new 지원
- AllocateElement() : zeroed 메모리 반환
- AllocateDirtyElement() : 초기화하지 않은 메모리 반환
- Construct(...) : placement new로 객체 생성
- DestructAndReturn(...) : 소멸자 호출 후 반환

### 📌 소스 코드
```cpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <mutex>

class FixedSizePool {
public:
    FixedSizePool();
    ~FixedSizePool();

    bool Create(size_t sizeof_element, size_t first_block_capacity = 0, size_t block_capacity = 0);

    void* AllocateElement();       // zeroed
    void* AllocateDirtyElement();  // uninitialized
    void ReturnElement(void* p);
    void ReturnAll();
    void Destroy();

    size_t SizeofElement() const;
    size_t ActiveElementCount() const;
    size_t TotalElementCount() const;
    size_t SizeOfPoolBytes() const;

    // Thread-safe wrappers
    void* ThreadSafeAllocateElement();
    void* ThreadSafeAllocateDirtyElement();
    void ThreadSafeReturnElement(void* p);

private:
    bool AllocateNewBlock(size_t capacity);
    static size_t AlignSize(size_t s);
    static size_t DefaultCapacity(size_t sizeof_element);

    void* first_block_;
    void* free_stack_;
    void* al_block_;
    void* al_element_cursor_;
    size_t al_remaining_;

    size_t sizeof_element_;
    size_t block1_capacity_;
    size_t blockN_capacity_;

    size_t active_count_;
    size_t total_count_;

    bool first_block_allocated_;
    bool initialized_;

    std::mutex lock_;
};
```

```cpp
// 타입 안전 래퍼
template <class T>
class SimpleFixedSizePool {
public:
    bool Create(size_t estimate = 0, size_t block_capacity = 0) {
        return pool_.Create(sizeof(T), estimate ? estimate : 0, block_capacity);
    }

    T* Allocate() { return static_cast<T*>(pool_.AllocateElement()); }
    T* AllocateDirty() { return static_cast<T*>(pool_.AllocateDirtyElement()); }

    template <class... Args>
    T* Construct(Args&&... args) {
        void* raw = pool_.AllocateDirtyElement();
        if (!raw) return nullptr;
        return new (raw) T(std::forward<Args>(args)...);
    }

    void DestructAndReturn(T* p) {
        if (!p) return;
        p->~T();
        pool_.ReturnElement(static_cast<void*>(p));
    }

    void ReturnAll() { pool_.ReturnAll(); }
    void Destroy() { pool_.Destroy(); }

    size_t ActiveCount() const { return pool_.ActiveElementCount(); }
    size_t TotalCount() const { return pool_.TotalElementCount(); }
    size_t SizeofElement() const { return pool_.SizeofElement(); }

private:
    FixedSizePool pool_;
};
```

```cpp
#include "FixedSizePool.h"

FixedSizePool::FixedSizePool()
    : first_block_(nullptr), free_stack_(nullptr),
      al_block_(nullptr), al_element_cursor_(nullptr),
      al_remaining_(0), sizeof_element_(0),
      block1_capacity_(0), blockN_capacity_(0),
      active_count_(0), total_count_(0),
      first_block_allocated_(false), initialized_(false) {}

FixedSizePool::~FixedSizePool() { Destroy(); }

size_t FixedSizePool::AlignSize(size_t s) {
    const size_t a = sizeof(void*);
    return (s + (a - 1)) & ~(a - 1);
}

size_t FixedSizePool::DefaultCapacity(size_t sizeof_element) {
    size_t page = 4096;
    if (page < 512) page = 512;
    const size_t overhead = 2 * sizeof(void*) + 32;
    size_t pages = 1;
    size_t cap = (pages * page - overhead) / sizeof_element;
    while (cap < 1000) {
        pages *= 2;
        cap = (pages * page - overhead) / sizeof_element;
        if (pages > 8 && cap > 64) break;
    }
    return (cap == 0 ? 1 : cap);
}

bool FixedSizePool::Create(size_t sizeof_element, size_t first_block_capacity, size_t block_capacity) {
    if (initialized_) return false;
    if (sizeof_element == 0) return false;

    sizeof_element_ = AlignSize(sizeof_element);
    if (first_block_capacity == 0) first_block_capacity = DefaultCapacity(sizeof_element_);
    if (block_capacity == 0) block_capacity = first_block_capacity;

    block1_capacity_ = first_block_capacity;
    blockN_capacity_ = block_capacity;

    initialized_ = true;
    return true;
}

bool FixedSizePool::AllocateNewBlock(size_t capacity) {
    const size_t bytes = 2 * sizeof(void*) + capacity * sizeof_element_;
    void* blk = ::operator new(bytes);
    void** next_ptr = reinterpret_cast<void**>(blk);
    void** end_ptr = reinterpret_cast<void**>(static_cast<char*>(blk) + sizeof(void*));
    void* head = static_cast<void*>(static_cast<char*>(blk) + 2 * sizeof(void*));
    void* end = static_cast<void*>(static_cast<char*>(blk) + 2 * sizeof(void*) + capacity * sizeof_element_);
    *next_ptr = nullptr;
    *end_ptr = end;

    if (!first_block_) {
        first_block_ = blk;
    } else {
        void* tail = first_block_;
        while (*reinterpret_cast<void**>(tail)) {
            tail = *reinterpret_cast<void**>(tail);
        }
        *reinterpret_cast<void**>(tail) = blk;
    }

    al_block_ = blk;
    al_element_cursor_ = head;
    al_remaining_ = capacity;
    first_block_allocated_ = true;
    return true;
}

void* FixedSizePool::AllocateElement() {
    void* p = AllocateDirtyElement();
    if (p) std::memset(p, 0, sizeof_element_);
    return p;
}

void* FixedSizePool::AllocateDirtyElement() {
    if (!initialized_) return nullptr;

    if (free_stack_) {
        void* p = free_stack_;
        free_stack_ = *reinterpret_cast<void**>(free_stack_);
        ++active_count_;
        return p;
    }

    if (!al_block_ || al_remaining_ == 0) {
        size_t capacity = (first_block_allocated_ ? blockN_capacity_ : block1_capacity_);
        if (!AllocateNewBlock(capacity)) return nullptr;
    }

    void* p = al_element_cursor_;
    al_element_cursor_ = static_cast<void*>(static_cast<char*>(al_element_cursor_) + sizeof_element_);
    --al_remaining_;
    ++active_count_;
    ++total_count_;
    return p;
}

void FixedSizePool::ReturnElement(void* p) {
    if (!p) return;
    if (active_count_ == 0) return;
    --active_count_;
    *reinterpret_cast<void**>(p) = free_stack_;
    free_stack_ = p;
}

void FixedSizePool::ReturnAll() {
    free_stack_ = nullptr;
    al_block_ = first_block_;
    if (al_block_) {
        al_element_cursor_ = static_cast<char*>(al_block_) + 2 * sizeof(void*);
        al_remaining_ = block1_capacity_;
    }
    active_count_ = 0;
    total_count_ = 0;
    first_block_allocated_ = true;
}

void FixedSizePool::Destroy() {
    void* blk = first_block_;
    while (blk) {
        void* next = *reinterpret_cast<void**>(blk);
        ::operator delete(blk);
        blk = next;
    }
    first_block_ = nullptr;
    free_stack_ = nullptr;
    al_block_ = nullptr;
    al_element_cursor_ = nullptr;
    al_remaining_ = 0;
    sizeof_element_ = 0;
    block1_capacity_ = 0;
    blockN_capacity_ = 0;
    active_count_ = 0;
    total_count_ = 0;
    first_block_allocated_ = false;
    initialized_ = false;
}

size_t FixedSizePool::SizeofElement() const { return sizeof_element_; }
size_t FixedSizePool::ActiveElementCount() const { return active_count_; }
size_t FixedSizePool::TotalElementCount() const { return total_count_; }

size_t FixedSizePool::SizeOfPoolBytes() const {
    size_t bytes = 0;
    for (void* blk = first_block_; blk; blk = *reinterpret_cast<void**>(blk)) {
        bytes += 2 * sizeof(void*) + blockN_capacity_ * sizeof_element_;
    }
    return bytes;
}

void* FixedSizePool::ThreadSafeAllocateElement() {
    std::lock_guard<std::mutex> g(lock_);
    return AllocateElement();
}

void* FixedSizePool::ThreadSafeAllocateDirtyElement() {
    std::lock_guard<std::mutex> g(lock_);
    return AllocateDirtyElement();
}

void FixedSizePool::ThreadSafeReturnElement(void* p) {
    std::lock_guard<std::mutex> g(lock_);
    ReturnElement(p);
}
````
---

### 📌 사용법
#### 🔹 1. POD 구조체 관리
```cpp
struct Node {
    int next;
    double weight;
};

SimpleFixedSizePool<Node> pool;
pool.Create(16, 0);

Node* n1 = pool.Allocate();
n1->next = 42;
n1->weight = 3.14;

pool.DestructAndReturn(n1);
pool.Destroy();
```

#### 🔹 2. 클래스 객체 관리 (생성자/소멸자 호출)
```cpp
struct Person {
    Person(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Construct Person(" << id_ << ", " << name_ << ")\n";
    }
    ~Person() {
        std::cout << "Destruct Person(" << id_ << ", " << name_ << ")\n";
    }
    void greet() const {
        std::cout << "Hello, I'm " << name_ << " (id=" << id_ << ")\n";
    }
    int id_;
    std::string name_;
};

SimpleFixedSizePool<Person> pool;
pool.Create(10, 0);

Person* p1 = pool.Construct(1, "Alice");
Person* p2 = pool.Construct(2, "Bob");

p1->greet();
p2->greet();

pool.DestructAndReturn(p1);
pool.DestructAndReturn(p2);

pool.Destroy();
```


#### 🔹 3. 대량 할당/반환 성능 테스트
```cpp
SimpleFixedSizePool<int> pool;
pool.Create(1000, 0);

const int N = 50000;
std::vector<int*> items;
items.reserve(N);

for (int i = 0; i < N; ++i) {
    int* p = pool.Allocate();
    *p = i;
    items.push_back(p);
}

for (int* p : items) {
    pool.DestructAndReturn(p);
}

std::cout << "Active count: " << pool.ActiveCount() << "\n";
pool.Destroy();
```


#### 🔹 4. 멀티스레드 ThreadSafe 테스트
```cpp
struct Item { int id; };

SimpleFixedSizePool<Item> pool;
pool.Create(100, 0);

auto worker = [&](int tid) {
    for (int i = 0; i < 1000; ++i) {
        Item* it = pool.AllocateDirty();
        it->id = tid * 1000 + i;
        pool.DestructAndReturn(it);
    }
};

std::thread t1(worker, 1);
std::thread t2(worker, 2);
t1.join();
t2.join();

std::cout << "Active count: " << pool.ActiveCount() << "\n";
pool.Destroy();
```


#### 🔹 결과 예시
```
Construct Person(1, Alice)
Construct Person(2, Bob)
Hello, I'm Alice (id=1)
Hello, I'm Bob (id=2)
Destruct Person(1, Alice)
Destruct Person(2, Bob)
Active count: 0
```

---
### 📌 테스트 코드

#### 🔹 1. Case1
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <string>

// 간단한 클래스 정의
class MyClass {
public:
    MyClass(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Construct MyClass(" << id_ << ", " << name_ << ")\n";
    }
    ~MyClass() {
        std::cout << "Destruct MyClass(" << id_ << ", " << name_ << ")\n";
    }
    void hello() const {
        std::cout << "Hello, my name is " << name_ << " and id=" << id_ << "\n";
    }
private:
    int id_;
    std::string name_;
};
```
```cpp
int main() {

    struct Node { int next; double weight; };
    SimpleFixedSizePool<Node> podPool;
    podPool.Create(16, 0);

    Node* n1 = podPool.Allocate();
    n1->next = 42;
    n1->weight = 3.14;
    std::cout << "Node: next=" << n1->next << " weight=" << n1->weight << "\n";
    podPool.DestructAndReturn(n1);


    SimpleFixedSizePool<MyClass> classPool;
    classPool.Create(8, 0);

    MyClass* c1 = classPool.Construct(1, "Alice");
    MyClass* c2 = classPool.Construct(2, "Bob");

    c1->hello();
    c2->hello();

    classPool.DestructAndReturn(c1);
    classPool.DestructAndReturn(c2);

    std::cout << "Active count after return: " << classPool.ActiveCount() << "\n";

    classPool.Destroy();
    podPool.Destroy();

    return 0;
}
```
#### 🔹 2. Case2
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <string>

// 간단한 클래스 정의
class MyClass {
public:
    MyClass(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Construct MyClass(" << id_ << ", " << name_ << ")\n";
    }
    ~MyClass() {
        std::cout << "Destruct MyClass(" << id_ << ", " << name_ << ")\n";
    }
    void hello() const {
        std::cout << "Hello, my name is " << name_ << " and id=" << id_ << "\n";
    }
private:
    int id_;
    std::string name_;
};
```
```cpp
int main() {
    struct Node { int next; double weight; };
    SimpleFixedSizePool<Node> podPool;
    podPool.Create(16, 0);

    Node* n1 = podPool.Allocate();
    n1->next = 42;
    n1->weight = 3.14;
    std::cout << "Node: next=" << n1->next << " weight=" << n1->weight << "\n";
    podPool.DestructAndReturn(n1);

    SimpleFixedSizePool<MyClass> classPool;
    classPool.Create(8, 0);

    MyClass* c1 = classPool.Construct(1, "Alice");
    MyClass* c2 = classPool.Construct(2, "Bob");

    c1->hello();
    c2->hello();

    classPool.DestructAndReturn(c1);
    classPool.DestructAndReturn(c2);

    std::cout << "Active count after return: " << classPool.ActiveCount() << "\n";

    classPool.Destroy();
    podPool.Destroy();

    return 0;
}
``` 
#### 🔹 3. Case3
```cpp
#include "FixedSizePool.h"
#include <iostream>

struct Node {
    int next;
    double weight;
};
```
```cpp
int main() {
    SimpleFixedSizePool<Node> pool;
    pool.Create(16, 0);

    Node* n1 = pool.Allocate();
    n1->next = 42;
    n1->weight = 3.14;

    std::cout << "Node: next=" << n1->next << " weight=" << n1->weight << "\n";

    pool.DestructAndReturn(n1);
    pool.Destroy();
    return 0;
}
```
#### 🔹 4. Case4
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <string>

struct Person {
    Person(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Construct Person(" << id_ << ", " << name_ << ")\n";
    }
    ~Person() {
        std::cout << "Destruct Person(" << id_ << ", " << name_ << ")\n";
    }
    void greet() const {
        std::cout << "Hello, I'm " << name_ << " (id=" << id_ << ")\n";
    }
    int id_;
    std::string name_;
};
```
```cpp
int main() {
    SimpleFixedSizePool<Person> pool;
    pool.Create(10, 0);

    Person* p1 = pool.Construct(1, "Alice");
    Person* p2 = pool.Construct(2, "Bob");

    p1->greet();
    p2->greet();

    pool.DestructAndReturn(p1);
    pool.DestructAndReturn(p2);

    pool.Destroy();
    return 0;
}
```
#### 🔹 5. Case5
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <string>

struct Person {
    Person(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Construct Person(" << id_ << ", " << name_ << ")\n";
    }
    ~Person() {
        std::cout << "Destruct Person(" << id_ << ", " << name_ << ")\n";
    }
    void greet() const {
        std::cout << "Hello, I'm " << name_ << " (id=" << id_ << ")\n";
    }
    int id_;
    std::string name_;
};
```
```cpp
int main() {
    SimpleFixedSizePool<Person> pool;
    pool.Create(10, 0);

    Person* p1 = pool.Construct(1, "Alice");
    Person* p2 = pool.Construct(2, "Bob");

    p1->greet();
    p2->greet();

    pool.DestructAndReturn(p1);
    pool.DestructAndReturn(p2);

    pool.Destroy();
    return 0;
}
```
#### 🔹 6. Case6
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <vector>
#include <chrono>

struct Data {
    int id;
    double value;
};
```
```cpp
int main() {
    SimpleFixedSizePool<Data> pool;
    pool.Create(1000, 0);

    const int N = 50000;
    std::vector<Data*> items;
    items.reserve(N);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        Data* d = pool.Allocate();
        d->id = i;
        d->value = i * 0.1;
        items.push_back(d);
    }

    for (Data* d : items) {
        pool.DestructAndReturn(d);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed ms: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "\n";
    std::cout << "Active count: " << pool.ActiveCount() << "\n";

    pool.Destroy();
    return 0;
}
```
#### 🔹 7. Case7
```cpp
#include "FixedSizePool.h"
#include <iostream>
#include <thread>
#include <vector>

struct Item {
    int id;
};

void worker(SimpleFixedSizePool<Item>& pool, int tid) {
    for (int i = 0; i < 1000; ++i) {
        Item* it = pool.AllocateDirty();
        it->id = tid * 1000 + i;
        pool.DestructAndReturn(it);
    }
}
```
```cpp
int main() {
    SimpleFixedSizePool<Item> pool;
    pool.Create(100, 0);

    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back(worker, std::ref(pool), t);
    }
    for (auto& th : threads) th.join();

    std::cout << "Active count: " << pool.ActiveCount() << "\n";
    pool.Destroy();
    return 0;
}
```
---

# RTOS 적용

### 📌 RTOS에서 적용하는 방법
#### 🔹 1. FreeRTOS 예시
```cpp
#include "FixedSizePool.h"
#include "FreeRTOS.h"
#include "semphr.h"

struct FixedSizePool {
    // ...
    SemaphoreHandle_t lock_;

    FixedSizePool() {
        lock_ = xSemaphoreCreateMutex();
    }
    ~FixedSizePool() {
        vSemaphoreDelete(lock_);
    }

    void* ThreadSafeAllocateElement() {
        xSemaphoreTake(lock_, portMAX_DELAY);
        void* p = AllocateElement();
        xSemaphoreGive(lock_);
        return p;
    }

    void ThreadSafeReturnElement(void* p) {
        xSemaphoreTake(lock_, portMAX_DELAY);
        ReturnElement(p);
        xSemaphoreGive(lock_);
    }
};
```

- std::mutex 대신 FreeRTOS의 SemaphoreHandle_t를 사용합니다.
- xSemaphoreTake / xSemaphoreGive로 임계 구역을 보호하면 됩니다.

#### 🔹 2. Zephyr RTOS 예시
```cpp
#include <zephyr.h>

struct FixedSizePool {
    struct k_mutex lock_;

    FixedSizePool() {
        k_mutex_init(&lock_);
    }

    void* ThreadSafeAllocateElement() {
        k_mutex_lock(&lock_, K_FOREVER);
        void* p = AllocateElement();
        k_mutex_unlock(&lock_);
        return p;
    }
};
```
- Zephyr에서는 k_mutex를 사용합니다.

#### 🔹 3. ThreadX 예시
```cpp
#include "tx_api.h"

struct FixedSizePool {
    TX_MUTEX lock_;

    FixedSizePool() {
        tx_mutex_create(&lock_, "pool_lock", TX_NO_INHERIT);
    }

    void* ThreadSafeAllocateElement() {
        tx_mutex_get(&lock_, TX_WAIT_FOREVER);
        void* p = AllocateElement();
        tx_mutex_put(&lock_);
        return p;
    }
};
```
- ThreadX에서는 TX_MUTEX를 사용합니다.

#### 🔹 요약
- RTOS 환경에서는 std::mutex 대신 RTOS 전용 동기화 객체를 사용해야 합니다.
- FreeRTOS → SemaphoreHandle_t
- Zephyr → k_mutex
- ThreadX → TX_MUTEX
- 나머지 풀 로직은 동일하게 유지됩니다.

---

### 📌 Fixed-size pool for FreeRTOS

FreeRTOS 환경에서 바로 사용할 수 있도록 만든 OpenNurbs 스타일의 고정 크기 메모리 풀 전체 소스입니다.  
블록 단위 할당, free-list 재사용, O(1) 할당/반환을 지원하며, 동기화는 FreeRTOS의 뮤텍스(SemaphoreHandle_t)로 처리합니다.  
블록 메모리는 pvPortMalloc/vPortFree를 사용합니다.

### 📌 Files overview
- FixedSizePool_FREERTOS.h: 헤더 (API와 템플릿 래퍼)
- FixedSizePool_FREERTOS.cpp: 구현 (FreeRTOS 락/힙 사용)
- Sample_FREERTOS.cpp: 샘플 코드 (POD/클래스/멀티태스크 예시)

### 📌 FixedSizePool_FREERTOS.h
```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <limits>
#include <cassert>

extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

// FreeRTOS용 FixedSizePool: raw memory 관리 (블록 + free-list)
struct FixedSizePool {
    FixedSizePool();
    ~FixedSizePool();

    // Initialize pool
    bool Create(size_t sizeof_element, size_t first_block_capacity = 0, size_t block_capacity = 0);

    // Allocation
    void* AllocateElement();       // zeroed
    void* AllocateDirtyElement();  // uninitialized

    // Return memory to pool (uses first sizeof(void*) bytes for free-list next)
    void ReturnElement(void* p);

    // Reset internal allocation cursor to first block (keep blocks)
    void ReturnAll();

    // Free all blocks and reset
    void Destroy();

    // Stats
    size_t SizeofElement() const;
    size_t ActiveElementCount() const;
    size_t TotalElementCount() const;
    size_t SizeOfPoolBytes() const;

    // Thread-safe versions (FreeRTOS mutex)
    void* ThreadSafeAllocateElement();
    void* ThreadSafeAllocateDirtyElement();
    void ThreadSafeReturnElement(void* p);

private:
    // Internal helpers
    bool AllocateNewBlock(size_t capacity);
    static size_t AlignSize(size_t s);
    static size_t DefaultCapacity(size_t sizeof_element);

    // Block list: [next (void*)][end (void*)][elements...]
    static constexpr size_t HeaderSize() { return 2 * sizeof(void*); }
    static void* BlockHead(void* block) {
        return static_cast<void*>(static_cast<char*>(block) + HeaderSize());
    }
    static void* BlockEnd(void* block) {
        return *reinterpret_cast<void**>(static_cast<char*>(block) + sizeof(void*));
    }

private:
    // Pool state
    void* first_block_ = nullptr;         // head of block list
    void* free_stack_ = nullptr;          // returned elements stack (uses first sizeof(void*))
    void* al_block_ = nullptr;            // current allocation block
    void* al_element_cursor_ = nullptr;   // next element address in current block
    size_t al_remaining_ = 0;             // remaining elements in current block

    size_t sizeof_element_ = 0;
    size_t block1_capacity_ = 0;
    size_t blockN_capacity_ = 0;

    size_t active_count_ = 0;
    size_t total_count_ = 0;

    bool first_block_allocated_ = false;
    bool initialized_ = false;

    // FreeRTOS mutex
    SemaphoreHandle_t lock_ = nullptr;
};

// 타입 안전 템플릿 래퍼 (placement new + 명시적 소멸자 호출 제공)
template <class T>
struct SimpleFixedSizePool {
    bool Create(size_t estimate = 0, size_t block_capacity = 0) {
        return pool_.Create(sizeof(T), estimate ? estimate : 0, block_capacity);
    }

    // Raw memory (zeroed / dirty)
    T* Allocate() { return static_cast<T*>(pool_.AllocateElement()); }
    T* AllocateDirty() { return static_cast<T*>(pool_.AllocateDirtyElement()); }

    // Placement new (constructor invoked)
    template <class... Args>
    T* Construct(Args&&... args) {
        void* raw = pool_.AllocateDirtyElement();
        if (!raw) return nullptr;
        return new (raw) T(std::forward<Args>(args)...);
    }

    // Explicit destructor + return to pool
    void DestructAndReturn(T* p) {
        if (!p) return;
        p->~T();
        pool_.ReturnElement(static_cast<void*>(p));
    }

    // Thread-safe helpers (use pool's mutex)
    template <class... Args>
    T* ThreadSafeConstruct(Args&&... args) {
        void* raw = pool_.ThreadSafeAllocateDirtyElement();
        if (!raw) return nullptr;
        return new (raw) T(std::forward<Args>(args)...);
    }
    void ThreadSafeDestructAndReturn(T* p) {
        if (!p) return;
        p->~T();
        pool_.ThreadSafeReturnElement(static_cast<void*>(p));
    }

    // Lifecycle
    void ReturnAll() { pool_.ReturnAll(); }
    void Destroy() { pool_.Destroy(); }

    // Stats
    size_t ActiveCount() const { return pool_.ActiveElementCount(); }
    size_t TotalCount() const { return pool_.TotalElementCount(); }
    size_t SizeofElement() const { return pool_.SizeofElement(); }

private:
    FixedSizePool pool_;
};
```
### 📌 FixedSizePool_FREERTOS.cpp
```cpp
#include "FixedSizePool_FREERTOS.h"

// Align to pointer size for bookkeeping
size_t FixedSizePool::AlignSize(size_t s) {
    const size_t a = sizeof(void*);
    return (s + (a - 1)) & ~(a - 1);
}

// Heuristic similar to OpenNurbs (page multiples)
size_t FixedSizePool::DefaultCapacity(size_t sizeof_element) {
    size_t page = 1024; // FreeRTOS heap granularity can vary; use a modest page
    if (page < 512) page = 512;
    const size_t overhead = HeaderSize() + 32;
    size_t pages = 1;
    size_t cap = (pages * page - overhead) / sizeof_element;
    while (cap < 1000) {
        pages *= 2;
        cap = (pages * page - overhead) / sizeof_element;
        if (pages > 8 && cap > 64) break;
    }
    return (cap == 0 ? 1 : cap);
}

FixedSizePool::FixedSizePool() {
    lock_ = xSemaphoreCreateMutex();
}

FixedSizePool::~FixedSizePool() {
    Destroy();
    if (lock_) {
        vSemaphoreDelete(lock_);
        lock_ = nullptr;
    }
}

bool FixedSizePool::Create(size_t sizeof_element, size_t first_block_capacity, size_t block_capacity) {
    if (initialized_) return false;
    if (sizeof_element == 0) return false;

    sizeof_element_ = AlignSize(sizeof_element);
    if (first_block_capacity == 0) first_block_capacity = DefaultCapacity(sizeof_element_);
    if (block_capacity == 0) block_capacity = first_block_capacity;

    block1_capacity_ = first_block_capacity;
    blockN_capacity_ = block_capacity;

    initialized_ = true;
    return true;
}

bool FixedSizePool::AllocateNewBlock(size_t capacity) {
    const size_t bytes = HeaderSize() + capacity * sizeof_element_;
    void* blk = pvPortMalloc(bytes);
    if (!blk) return false;

    // header
    void** next_ptr = reinterpret_cast<void**>(blk);
    void** end_ptr  = reinterpret_cast<void**>(static_cast<char*>(blk) + sizeof(void*));
    void*  head     = BlockHead(blk);
    void*  end      = static_cast<void*>(static_cast<char*>(blk) + HeaderSize() + capacity * sizeof_element_);
    *next_ptr = nullptr;
    *end_ptr  = end;

    // append block
    if (!first_block_) {
        first_block_ = blk;
    } else {
        void* tail = first_block_;
        while (*reinterpret_cast<void**>(tail)) {
            tail = *reinterpret_cast<void**>(tail);
        }
        *reinterpret_cast<void**>(tail) = blk;
    }

    al_block_ = blk;
    al_element_cursor_ = head;
    al_remaining_ = capacity;
    first_block_allocated_ = true;
    return true;
}

void* FixedSizePool::AllocateElement() {
    void* p = AllocateDirtyElement();
    if (p) std::memset(p, 0, sizeof_element_);
    return p;
}

void* FixedSizePool::AllocateDirtyElement() {
    if (!initialized_) return nullptr;

    // 1) Try free-list
    if (free_stack_) {
        void* p = free_stack_;
        free_stack_ = *reinterpret_cast<void**>(free_stack_);
        ++active_count_;
        return p;
    }

    // 2) Need block?
    if (!al_block_ || al_remaining_ == 0) {
        size_t capacity = (first_block_allocated_ ? blockN_capacity_ : block1_capacity_);
        if (!AllocateNewBlock(capacity)) return nullptr;
    }

    // 3) Serve from current block
    void* p = al_element_cursor_;
    al_element_cursor_ = static_cast<void*>(static_cast<char*>(al_element_cursor_) + sizeof_element_);
    --al_remaining_;
    ++active_count_;
    ++total_count_;
    return p;
}

void FixedSizePool::ReturnElement(void* p) {
    if (!p) return;
    if (active_count_ == 0) {
        // programming guard (double free or foreign pointer)
        assert(false && "ReturnElement called with no active elements");
        return;
    }
    --active_count_;
    // push to free-list; first sizeof(void*) used for next pointer
    *reinterpret_cast<void**>(p) = free_stack_;
    free_stack_ = p;
}

void FixedSizePool::ReturnAll() {
    free_stack_ = nullptr;
    al_block_ = first_block_;
    if (al_block_) {
        al_element_cursor_ = BlockHead(al_block_);
        // Reset to first block capacity (subsequent blocks reused later)
        al_remaining_ = block1_capacity_;
    }
    active_count_ = 0;
    total_count_ = 0;
    first_block_allocated_ = (first_block_ != nullptr);
}

void FixedSizePool::Destroy() {
    void* blk = first_block_;
    while (blk) {
        void* next = *reinterpret_cast<void**>(blk);
        vPortFree(blk);
        blk = next;
    }
    // reset state
    first_block_ = nullptr;
    free_stack_ = nullptr;
    al_block_ = nullptr;
    al_element_cursor_ = nullptr;
    al_remaining_ = 0;
    sizeof_element_ = 0;
    block1_capacity_ = 0;
    blockN_capacity_ = 0;
    active_count_ = 0;
    total_count_ = 0;
    first_block_allocated_ = false;
    initialized_ = false;
}

size_t FixedSizePool::SizeofElement() const { return sizeof_element_; }
size_t FixedSizePool::ActiveElementCount() const { return active_count_; }
size_t FixedSizePool::TotalElementCount() const { return total_count_; }

size_t FixedSizePool::SizeOfPoolBytes() const {
    size_t bytes = 0;
    for (void* blk = first_block_; blk; blk = *reinterpret_cast<void**>(blk)) {
        // end - head = payload size
        auto end  = static_cast<char*>(BlockEnd(blk));
        auto head = static_cast<char*>(BlockHead(blk));
        bytes += HeaderSize() + static_cast<size_t>(end - head);
    }
    return bytes;
}

// Thread-safe (FreeRTOS mutex)
void* FixedSizePool::ThreadSafeAllocateElement() {
    if (lock_) xSemaphoreTake(lock_, portMAX_DELAY);
    void* p = AllocateElement();
    if (lock_) xSemaphoreGive(lock_);
    return p;
}
void* FixedSizePool::ThreadSafeAllocateDirtyElement() {
    if (lock_) xSemaphoreTake(lock_, portMAX_DELAY);
    void* p = AllocateDirtyElement();
    if (lock_) xSemaphoreGive(lock_);
    return p;
}
void FixedSizePool::ThreadSafeReturnElement(void* p) {
    if (lock_) xSemaphoreTake(lock_, portMAX_DELAY);
    ReturnElement(p);
    if (lock_) xSemaphoreGive(lock_);
}
```


### 📌 Sample_FREERTOS.cpp
```cpp
#include "FixedSizePool_FREERTOS.h"
#include <cstdio>

// POD 예제
struct Node {
    int next;
    float weight;
};

// 클래스 예제 (placement new + 명시적 소멸자)
struct Person {
    Person(int id, const char* name) : id_(id), name_(name) {
        printf("Construct Person(%d, %s)\n", id_, name_);
    }
    ~Person() {
        printf("Destruct Person(%d, %s)\n", id_, name_);
    }
    void greet() const {
        printf("Hello, I'm %s (id=%d)\n", name_, id_);
    }
    int id_;
    const char* name_;
};

// 멀티태스크 예제용 데이터
struct Item {
    int tid;
    int seq;
};

// FreeRTOS 태스크 (동시 할당/반환)
extern "C" void WorkerTask(void* pv) {
    auto* pool = static_cast<SimpleFixedSizePool<Item>*>(pv);
    for (int i = 0; i < 1000; ++i) {
        Item* it = pool->ThreadSafeConstruct(1, i);
        // ... 작업 ...
        pool->ThreadSafeDestructAndReturn(it);
    }
    vTaskDelete(nullptr);
}
```
```cpp
int main(void) {
    // 1) POD 테스트
    {
        SimpleFixedSizePool<Node> pool;
        pool.Create(32, 0);

        Node* a = pool.Allocate();      // zeroed
        a->next = 7;
        a->weight = 0.5f;

        printf("Node: next=%d weight=%.3f\n", a->next, a->weight);
        pool.DestructAndReturn(a);

        pool.Destroy();
    }

    // 2) 클래스 테스트
    {
        SimpleFixedSizePool<Person> pool;
        pool.Create(16, 0);

        Person* p1 = pool.Construct(1, "Alice");
        Person* p2 = pool.Construct(2, "Bob");

        p1->greet();
        p2->greet();

        pool.DestructAndReturn(p1);
        pool.DestructAndReturn(p2);

        printf("Active count: %u\n", static_cast<unsigned>(pool.ActiveCount()));
        pool.Destroy();
    }

    // 3) 멀티태스크 테스트 (실제 RTOS 환경에서만 동작)
    // 주의: 이 코드는 FreeRTOS 커널이 실행 중일 때만 의미가 있습니다.
    // 테스트 환경이 없다면 참고용으로만 보세요.
    /*
    {
        static SimpleFixedSizePool<Item> pool; // static: 태스크 종료 후에도 유지
        pool.Create(64, 0);

        xTaskCreate(WorkerTask, "W1", 1024, &pool, tskIDLE_PRIORITY + 1, nullptr);
        xTaskCreate(WorkerTask, "W2", 1024, &pool, tskIDLE_PRIORITY + 1, nullptr);

        vTaskStartScheduler(); // 실제 RTOS에서 스케줄러 시작
        // 스케줄러가 돌아가면 main은 보통 여기서 리턴하지 않습니다.
    }
    */

    // 테스트 환경이 없으므로 여기서 종료
    return 0;
}
```

### 📌 Notes for RTOS integration
- 메모리: 블록은 pvPortMalloc/vPortFree로 관리합니다. 프로젝트의 FreeRTOS heap 설정에 맞춰 동작합니다.
- 동기화: SemaphoreHandle_t 기반 뮤텍스 사용. ThreadSafe* 함수들만 락을 잡고, 일반 함수는 락 없이 동작합니다.
- 태스크 간 공유 시에는 ThreadSafe 함수 사용을 권장합니다.
- Bookkeeping 영역: 반환된 요소의 첫 sizeof(void*) 바이트는 free-list 포인터로 사용합니다.
- 클래스 타입을 쓸 때는 반드시 placement new를 사용하고, 반환 전 소멸자를 명시적으로 호출.
- ReturnAll: 블록은 유지하고 커서만 초기화합니다. 다음 사이클에서 블록 재사용으로 빠르게 시작할 수 있습니다.
- Capacity 힌트: Create(sizeof_element, estimate, block_capacity)에서 estimate를 적절히 크게 잡으면  
  첫 블록이 넉넉하게 잡혀 초기 힙 호출을 줄일 수 있습니다.

### 📌 Build (example)
프로젝트에 FreeRTOS 헤더/라이브러리를 포함한 뒤, 아래와 같이 컴파일합니다.
- Include paths: FreeRTOS headers, config, portable layer
- Link: FreeRTOS kernel, heap implementation (heap4 등)

---





