## ☕ SafeMemMgrEx 사용 가이드 for Java 개발자

### 📌 도입 취지

- Java 개발자는 보통 Garbage Collector(GC)에 의존해 객체의 생명주기를 자동으로 관리합니다.
- 하지만 C++에서는 `new`와 `delete`를 직접 호출해야 하며, 이로 인해 다음과 같은 문제가 발생할 수 있습니다:
    - 메모리 누수
    - 객체 소멸자 누락
    - 성능 저하 (반복적인 `new/delete`)
    - 멀티스레드 환경에서의 동기화 문제

- `SafeMemMgrEx`는 이러한 문제를 해결하기 위해 설계된 **C++용 객체 중심 메모리 관리 시스템** 입니다.
- Java 개발자가 GC 없이도 안전하고 효율적으로 객체를 관리할 수 있도록 도와줍니다.

---

### 📌 핵심 개념

| Java 개념              | ON_SafeMemMgrEx 대응               |
|------------------------|------------------------------------|
| `new T()`              | `AllocObject<T>(tag, args...)`     |
| `obj.close()` 또는 GC  | `FreeObject(tag, obj)`             |
| `try-with-resources`  | 태그 기반 객체 그룹 관리           |
| `ConcurrentHashMap`   | 내부 `unordered_map` + `mutex` 사용 |

---

### 📌 사용법

#### 1. 객체 생성

```cpp
SafeMemMgrEx memMgr;

// Java: MyClass obj = new MyClass(10);
MyVirtualClass* obj = memMgr.AllocObject<MyVirtualClass>("network", 10);
```

#### 2. 객체 해제
```cpp
// Java: obj.close() 또는 GC
memMgr.FreeObject("network", obj);
```

- 객체의 소멸자를 호출한 뒤 메모리를 반환합니다.
- 태그를 통해 해당 객체가 어떤 그룹에 속했는지 명시합니다.

---

#### 3. 태그 그룹 전체 해제
```cpp
// Java: 리소스 그룹 전체 정리
memMgr.FreeObjectsByTag("network");
```

#### 4. 전체 객체 해제
```cpp
// Java: System.gc() 또는 shutdown hook
memMgr.FreeAllObjects();
```
- 모든 객체를 소멸시키고, 내부 메모리 풀도 정리합니다.

---

### 📌 예제 코드
```cpp
class MyVirtualClass {
public:
    MyVirtualClass(int a) {
        std::cout << "Created with " << a << std::endl;
    }
    virtual ~MyVirtualClass() {
        std::cout << "Destroyed" << std::endl;
    }
};

int main() {
    SafeMemMgrEx memMgr;

    auto* obj1 = memMgr.AllocObject<MyVirtualClass>("network", 1);
    auto* obj2 = memMgr.AllocObject<MyVirtualClass>("graphics", 2);
    auto* obj3 = memMgr.AllocObject<MyVirtualClass>("network", 3);

    memMgr.FreeObject("network", obj1);         // 개별 해제
    memMgr.FreeObjectsByTag("network");         // 그룹 해제
    memMgr.FreeAllObjects();                    // 전체 해제
}

```
---

### 📌 장점 요약

- 객체 생성/소멸을 안전하게 관리
- 메모리 풀 기반으로 성능 향상
- 태그 기반 그룹 관리로 구조적 해제 가능
- 스레드 안전성 확보
- Java 개발자에게 친숙한 API 스타일

---

### 📌 참고 

- 객체를 생성할 때는 반드시 AllocObject를 사용하세요. new를 직접 쓰면 메모리 풀을 우회하게 됩니다.
- 객체를 해제할 때는 FreeObject 또는 FreeObjectsByTag를 사용하세요. delete를 직접 쓰면 소멸자 호출은 되지만 메모리 풀에는 반환되지 않습니다.
- 태그는 "network", "graphics", "session" 등 자유롭게 지정할 수 있으며, 그룹 해제에 유용합니다.


### 📌 소스 코드
```cpp
#pragma once
#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <new>
#include <vector>
#include <memory>

#ifndef FS_MM_DEFAULT_CHUNK_SIZE
#define FS_MM_DEFAULT_CHUNK_SIZE 4096
#endif

class FixedSizeMemMgr
{
public:
    explicit FixedSizeMemMgr(
        const int allocSize,
        const int chunkSize = FS_MM_DEFAULT_CHUNK_SIZE)
        : m_allocSize(Align(allocSize))
        , m_chunkSize(chunkSize)
        , m_free(nullptr)
        , m_chunk(nullptr)
    {
        assert(m_allocSize >= static_cast<int>(sizeof(void *)));
        assert(m_chunkSize > static_cast<int>(sizeof(CHUNK)));
        assert(m_chunkSize >= static_cast<int>(sizeof(CHUNK)) + m_allocSize);
    }

    ~FixedSizeMemMgr() {
        FreeAllMem();
    }
    FixedSizeMemMgr(const FixedSizeMemMgr&) = delete;
    FixedSizeMemMgr& operator=(const FixedSizeMemMgr&) = delete;


    void* Alloc()
    {
        std::lock_guard lock(m_mutex);

        if (!m_free)
            MakeNewChunk();

        ITEM* p = m_free;
        m_free = m_free->next;
        return p;
    }

    void Free(void* mem)
    {
        if (!mem) return;

        std::lock_guard lock(m_mutex);

        const auto p = static_cast<ITEM*>(mem);
        p->next = m_free;
        m_free = p;
    }

    void FreeAllMem()
    {
        std::lock_guard lock(m_mutex);

        CHUNK* c = m_chunk;
        while (c)
        {
            CHUNK* next = c->next;
            delete[] reinterpret_cast<char*>(c);
            c = next;
        }
        m_chunk = nullptr;
        m_free = nullptr;
    }
    [[nodiscard]] std::size_t AllocSize() const noexcept { return m_allocSize; }
    [[nodiscard]] std::size_t ChunkSize() const noexcept { return m_chunkSize; }

private:
    struct ITEM { ITEM* next; };
    struct CHUNK { CHUNK* next; };

    static int Align(const int size)
    {
        constexpr int ALIGN = alignof(std::max_align_t);
        return size + ALIGN - 1 & ~(ALIGN - 1);
    }

    static std::uintptr_t AlignPtr(std::uintptr_t p, std::uintptr_t a)
    {
        return p + a - 1 & ~(a - 1);
    }

    void MakeNewChunk()
    {
        auto mem = new char[static_cast<size_t>(m_chunkSize)];
        const auto chunk = reinterpret_cast<CHUNK*>(mem);

        chunk->next = m_chunk;
        m_chunk = chunk;

        const int usable0 = m_chunkSize - static_cast<int>(sizeof(CHUNK));
        assert(usable0 > 0);

        const auto base = reinterpret_cast<std::uintptr_t>(mem + sizeof(CHUNK));
        const std::uintptr_t aligned = AlignPtr(base, alignof(std::max_align_t));
        int usable = usable0 - static_cast<int>(aligned - base);
        assert(usable >= m_allocSize);

        const int count = usable / m_allocSize;
        assert(count >= 1);

        auto p0 = reinterpret_cast<char *>(aligned);

        const auto first = reinterpret_cast<ITEM*>(p0);
        ITEM* cur = first;

        for (int i = 1; i < count; ++i)
        {
            char* next = p0 + static_cast<size_t>(i) * static_cast<size_t>(m_allocSize);
            cur->next = reinterpret_cast<ITEM*>(next);
            cur = cur->next;
        }
        cur->next = nullptr;
        m_free = first;
    }

int m_allocSize;
    int m_chunkSize;

    ITEM*  m_free;
    CHUNK* m_chunk;

    std::mutex m_mutex;
};

class SimpleMemMgr final {
public:
    explicit SimpleMemMgr(const int nAllocSize,
        int = FS_MM_DEFAULT_CHUNK_SIZE)
    { m_nAllocSize = nAllocSize; }

    ~SimpleMemMgr() = default;

    [[nodiscard]] void* Alloc() const {
        return new char[m_nAllocSize];
    }
    static void  Free(void* pMem) {
        delete [] static_cast<char *>(pMem);
    }
    static void  FreeAllMem() {}
    [[nodiscard]] std::size_t AllocSize() const noexcept { return m_nAllocSize; }
    friend std::ostream& operator<< (std::ostream& out, const SimpleMemMgr& item);
protected:
    int m_nAllocSize;
};


class EnhancedMemMgr final
{
public:
    explicit EnhancedMemMgr(std::size_t nAllocSize,
                            std::size_t nChunkSize = 1024)
        : m_pool(static_cast<int>(nAllocSize),
            static_cast<int>(ComputeChunkBytes(nAllocSize, nChunkSize)))
    {
    }

    [[nodiscard]] void* Alloc()
    {
        return m_pool.Alloc();
    }

    void Free(void* pMem)
    {
        m_pool.Free(pMem);
    }

    void FreeAllMem()
    {
        m_pool.FreeAllMem();
    }

    template<typename T, typename... Args>
    T* AllocObject(Args&&... args)
    {
        static_assert(!std::is_array_v<T>, "T must not be an array type");
        assert(sizeof(T) <= m_pool.AllocSize());

        void* rawMem = Alloc();
        return new(rawMem) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void FreeObject(T* obj)
    {
        if (!obj)
            return;

        obj->~T();
        Free(static_cast<void*>(obj));
    }

    friend std::ostream& operator<<(std::ostream& out, const EnhancedMemMgr& item);

private:
    static std::size_t ComputeChunkBytes(std::size_t allocSize, std::size_t chunkCount)
    {
        const std::size_t alignedAlloc =
            (allocSize + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);

        const std::size_t safeChunkCount =
            (std::max)(chunkCount, static_cast<std::size_t>(1));

        return sizeof(void*) + alignedAlloc * safeChunkCount + alignof(std::max_align_t);
    }

private:
    FixedSizeMemMgr m_pool;
};


class SafeMemMgr final : public FixedSizeMemMgr
{
private:
    struct ObjectRecord
    {
        void* ptr = nullptr;
        void (*destructor)(void*) = nullptr;
    };

public:
    explicit SafeMemMgr(const std::size_t allocSize,
                        const std::size_t chunkSize = 1024)
        : FixedSizeMemMgr(static_cast<int>(allocSize), static_cast<int>(chunkSize))
    {
    }

    ~SafeMemMgr()
    {
        FreeAllObjects();
    }

    template<typename T, typename... Args>
    T* AllocObject(Args&&... args)
    {
        static_assert(!std::is_array_v<T>, "T must not be an array type");
        assert(sizeof(T) <= AllocSize());

        void* rawMem = Alloc();
        if (!rawMem)
            return nullptr;

        T* obj = new(rawMem) T(std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> lock(m_listMutex);
            m_allocatedObjects.push_back(
                ObjectRecord{
                    static_cast<void*>(obj),
                    [](void* p) { static_cast<T*>(p)->~T(); }
                });
        }

        return obj;
    }

    template<typename T>
    void FreeObject(T* obj)
    {
        if (!obj)
            return;

        ObjectRecord record{};

        {
            std::lock_guard<std::mutex> lock(m_listMutex);

            auto it = std::find_if(
                m_allocatedObjects.begin(),
                m_allocatedObjects.end(),
                [obj](const ObjectRecord& rec)
                {
                    return rec.ptr == static_cast<void*>(obj);
                });

            if (it == m_allocatedObjects.end())
                return;

            record = *it;
            m_allocatedObjects.erase(it);
        }

        if (record.destructor)
            record.destructor(record.ptr);

        Free(record.ptr);
    }

    void FreeAllObjects()
    {
        std::vector<ObjectRecord> objectsCopy;

        {
            std::lock_guard<std::mutex> lock(m_listMutex);
            objectsCopy.swap(m_allocatedObjects);
        }

        for (auto& rec : objectsCopy)
        {
            if (rec.destructor)
                rec.destructor(rec.ptr);

            Free(rec.ptr);
        }

        FreeAllMem();
    }

    friend std::ostream& operator<<(std::ostream& out, const SafeMemMgr& item);

private:
    std::mutex m_listMutex;
    std::vector<ObjectRecord> m_allocatedObjects;
};

class SafeMemMgrEx
{
private:
    struct ObjectRecord
    {
        void* ptr = nullptr;
        void (*destructor)(void*) = nullptr;
        std::size_t allocSize = 0;
    };

    struct Pool
    {
        std::unique_ptr<FixedSizeMemMgr> memMgr;
        std::size_t allocSize = 0;
    };

public:
    SafeMemMgrEx() = default;

    ~SafeMemMgrEx()
    {
        FreeAllObjects();
    }

    template<typename T, typename... Args>
    T* AllocObject(const std::string& tag, Args&&... args)
    {
        static_assert(!std::is_array_v<T>, "T must not be an array type");

        const std::size_t allocSize = sizeof(T);
        FixedSizeMemMgr* memMgr = nullptr;

        {
            std::lock_guard lock(m_mutex);

            auto it = m_pools.find(allocSize);
            if (it == m_pools.end())
            {
                auto pool = std::make_unique<Pool>();
                pool->allocSize = allocSize;
                pool->memMgr = std::make_unique<FixedSizeMemMgr>(allocSize);

                it = m_pools.emplace(allocSize, std::move(pool)).first;
            }

            memMgr = it->second->memMgr.get();
        }

        void* rawMem = memMgr->Alloc();
        if (!rawMem)
            return nullptr;

        T* obj = new(rawMem) T(std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_taggedObjects[tag].push_back(
                ObjectRecord{
                    static_cast<void*>(obj),
                    [](void* p) { static_cast<T*>(p)->~T(); },
                    allocSize
                });
        }

        return obj;
    }

    template<typename T>
    void FreeObject(const std::string& tag, T* obj)
    {
        if (!obj)
            return;

        ObjectRecord record{};
        bool found = false;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto tagIt = m_taggedObjects.find(tag);
            if (tagIt == m_taggedObjects.end())
                return;

            auto& vec = tagIt->second;
            auto it = std::find_if(
                vec.begin(),
                vec.end(),
                [obj](const ObjectRecord& rec)
                {
                    return rec.ptr == static_cast<void*>(obj);
                });

            if (it == vec.end())
                return;

            record = *it;
            vec.erase(it);
            if (vec.empty())
                m_taggedObjects.erase(tagIt);

            found = true;
        }

        if (!found) return;

        if (record.destructor)
            record.destructor(record.ptr);

        FixedSizeMemMgr* memMgr = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto poolIt = m_pools.find(record.allocSize);
            if (poolIt != m_pools.end())
                memMgr = poolIt->second->memMgr.get();
        }

        if (memMgr)
            memMgr->Free(record.ptr);
    }

    void FreeObjectsByTag(const std::string& tag)
    {
        std::vector<ObjectRecord> records;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_taggedObjects.find(tag);
            if (it == m_taggedObjects.end())
                return;

            records.swap(it->second);
            m_taggedObjects.erase(it);
        }

        for (auto& rec : records)
        {
            if (rec.destructor)
                rec.destructor(rec.ptr);

            FixedSizeMemMgr* memMgr = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto poolIt = m_pools.find(rec.allocSize);
                if (poolIt != m_pools.end())
                    memMgr = poolIt->second->memMgr.get();
            }

            if (memMgr)
                memMgr->Free(rec.ptr);
        }
    }

    void FreeAllObjects()
    {
        std::vector<ObjectRecord> allRecords;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto& [tag, vec] : m_taggedObjects)
            {
                for (auto& rec : vec)
                    allRecords.push_back(rec);
            }

            m_taggedObjects.clear();
        }

        for (auto& rec : allRecords)
        {
            if (rec.destructor)
                rec.destructor(rec.ptr);

            FixedSizeMemMgr* memMgr = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto poolIt = m_pools.find(rec.allocSize);
                if (poolIt != m_pools.end())
                    memMgr = poolIt->second->memMgr.get();
            }

            if (memMgr)
                memMgr->Free(rec.ptr);
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [size, pool] : m_pools)
        {
            pool->memMgr->FreeAllMem();
        }
        m_pools.clear();
    }

    friend std::ostream& operator<<(std::ostream& out, const SafeMemMgrEx& item);

private:
    std::unordered_map<std::string, std::vector<ObjectRecord>> m_taggedObjects;
    std::unordered_map<std::size_t, std::unique_ptr<Pool>> m_pools;
    std::mutex m_mutex;
};
```
```cpp
#include "mem_manager.h"
#include <fstream>


std::ostream& operator<<(std::ostream& out, const FixedSizeMemMgr& item)
{
    out << "FixedSizeMemMgr{"
        << "allocSize=" << item.AllocSize()
        << ", chunkSize=" << item.ChunkSize()
        << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SimpleMemMgr& item)
{
    out << "SimpleMemMgr{"
        << "allocSize=" << item.AllocSize()
        << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const EnhancedMemMgr& item)
{
    out << "EnhancedMemMgr{" << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SafeMemMgr& item)
{
    out << "SafeMemMgr{"
        << "allocSize=" << item.AllocSize()
        << ", chunkSize=" << item.ChunkSize()
        << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SafeMemMgrEx& item)
{
    out << "SafeMemMgrEx{" << "}";
    return out;
}
```
---



