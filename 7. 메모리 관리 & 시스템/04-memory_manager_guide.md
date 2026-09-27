## 📘 Memory Manager 확장판 가이드

### 📌 개요
본 문서는 `CFixedSizeMemMgr`, `ON_FixedSizeMemMgr`, `ON_EnhancedMemMgr`, `ON_SafeMemMgrEx` 등 고정 크기 메모리 풀 관리 클래스들의 구조, 특징, 장단점, 사용 예제를 정리한 자료입니다.  
게임 엔진, CAD, 실시간 시뮬레이션 등에서 할당/해제 성능을 극대화하기 위해 사용됩니다.

### 📌 1. CFixedSizeMemMgr
#### 목적
- **고정 크기** 의 객체를 매우 빠르게 할당/해제
- 할당/해제가 **매우 빈번한 경우** 에 최적

#### 특징
- **Thread-Safe**: `std::mutex`로 동기화
- 내부적으로 **chunk 단위 메모리 풀** 사용
- 모든 블록은 **동일 크기**

#### 장점
- malloc/free 대비 매우 빠름
- 캐시 친화적 구조

#### 단점
- 내부 단편화 발생 가능
- chunk 단위로만 메모리 반환

#### 사용 예시
```cpp
CFixedSizeMemMgr mgr(sizeof(MyStruct), 1024*10);

auto* p = (MyStruct*)mgr.Alloc();
mgr.Free(p);

mgr.FreeAllMem(); // 모든 chunk 해제, 기존 포인터 무효
```

### 📌 2. ON_FixedSizeMemMgr
#### 목적
- `CFixedSizeMemMgr`의 변형
- OpenNURBS 환경 최적화

#### 특징
- Thread-Safe (`std::mutex` 사용)
- Free List + Chunk 관리

#### 장점
- 다양한 크기 관리 가능
- 재사용성 높음


### 📌 3. ON_EnhancedMemMgr
### 목적
- 확장형 고정 크기 메모리 관리자
- 할당 크기별 Pool 관리

### 특징
- Thread-Safe
- Pool별 mutex 분리 가능

### 장점
- 다양한 크기 할당에 대응
- Pool 내부 재사용성 극대화


### 4. ON_SafeMemMgrEx
#### 목적
- **태그 기반 객체 추적** 및 해제
- 특정 그룹 단위 해제 가능

#### 특징
- Thread-Safe
- `tag`를 사용하여 관련 객체 묶음 관리

#### 장점
- 게임/시뮬레이션에서 특정 장면 단위 메모리 해제 가능
- 디버깅에 유리 (메모리 누수 추적)

#### 주의
- `allocSize` 계산 시 반드시 **정확한 객체 크기** 사용 필요


### 📌 비교표

| 클래스명              | Thread-Safe | 크기 고정 | 동기화 방식  | 특징 |
|----------------------|-------------|-----------|--------------|------|
| CFixedSizeMemMgr     | ✅           | ✅         | `std::mutex` | Chunk 단위 고정 크기 |
| ON_FixedSizeMemMgr   | ✅           | ✅         | `std::mutex` | OpenNURBS 최적화 |
| ON_EnhancedMemMgr    | ✅           | ❌         | `std::mutex` | 크기별 Pool 관리 |
| ON_SafeMemMgrEx      | ✅           | ❌         | `std::mutex` | 태그 기반 그룹 해제 |



### 📌 성능 튜닝 팁
- 1. **Chunk Size 조정**
   - 초기 할당 크기를 실제 사용량에 맞춰 설정
   - 너무 작으면 잦은 할당 → 성능 저하
   - 너무 크면 메모리 낭비

- 2. **FreeAllMem() 사용 시 주의**
   - 호출 즉시 모든 메모리 해제
   - 기존 포인터 전부 무효화




### 📌 예제: 멀티풀 매니저 사용
```cpp
ON_EnhancedMemMgr mgr;

// 128바이트 객체 할당
void* obj1 = mgr.Alloc(128);

// 256바이트 객체 할당
void* obj2 = mgr.Alloc(256);

// 해제
mgr.Free(obj1);
mgr.Free(obj2);
```



### 📌 예제: 태그 기반 해제
```cpp
ON_SafeMemMgrEx mgr;

auto* obj1 = (MyStruct*)mgr.AllocObject(sizeof(MyStruct), "SceneA");
auto* obj2 = (MyStruct*)mgr.AllocObject(sizeof(MyStruct), "SceneA");
auto* obj3 = (MyStruct*)mgr.AllocObject(sizeof(MyStruct), "SceneB");

// SceneA 전체 해제
mgr.FreeObjectsByTag("SceneA");

// SceneB 개별 해제
mgr.Free(obj3);
```

### 📌 메모리 풀 내부 동작 다이어그램

```text
+-------------------------------+
|          Memory Pool          |
+-------------------------------+
|    Chunk #1                   |
|   +-------+  +-------+         |
|   | Block |->| Block |-> NULL |
|   +-------+  +-------+         |
|                               |
|    Chunk #2                   |
|   +-------+  +-------+  +-------+
|   | Block |->| Block |->| Block |-> NULL
|   +-------+  +-------+  +-------+
+--------------------------------+

- 할당 과정:
   - 1. Free List에서 첫 번째 블록 Pop
   - 2. 사용자가 포인터 획득
   - 3. Free List 헤드 변경

- 해제 과정:
   - 1. 반환된 블록을 Free List 헤드에 Push
   - 2. 블록 내부 next 포인터 갱신
   - 3. 재사용 대기
```

---

### 📌 주의사항
- 멀티스레드 환경에서는 반드시 Thread-Safe 버전 사용
- `FreeAllMem()` 후 남아있는 포인터 접근 금지
- Pool 관리 클래스는 new/delete 혼용 금지



### 📌 소스
```cpp

#pragma once

#include <mutex>

#define FSMM_DEFAULT_CHUNK_SIZE  1024000

#pragma pack(push, 4)

class ON_CLASS ON_FixedSizeMemMgr
{
public:
    explicit ON_FixedSizeMemMgr(int nAllocSize, int nChunkSize = FSMM_DEFAULT_CHUNK_SIZE);
    virtual ~ON_FixedSizeMemMgr();
public:
    void* Alloc();
    void  Free(void* pMem);
    void  FreeAllMem();
    friend std::ostream& operator<< (std::ostream& out, const ON_FixedSizeMemMgr& item);
protected:
    void  MakeNewChunk();
protected:
    typedef struct tagITEM
    {
        struct tagITEM* pNext;
        tagITEM() : pNext(0) {}
    } ITEM;
    ITEM* m_pFree;
    typedef struct tagCHUNK
    {
        struct tagCHUNK* pNext;
        tagCHUNK() : pNext(0) {}
    } CHUNK;
    CHUNK* m_pChunk;

    int m_nAllocSize;
    int m_nChunkSize;

    std::mutex m_mutex;
};

class ON_SimpleMemMgr
{
public:
    explicit ON_SimpleMemMgr(int nAllocSize, int nChunkSize = FSMM_DEFAULT_CHUNK_SIZE) 
    { m_nAllocSize = nAllocSize; }
    virtual ~ON_SimpleMemMgr() = default;
public:
    inline void* Alloc() const { return new char[m_nAllocSize]; }
    static inline void  Free(void* pMem) { delete (char*)pMem; }
    inline void  FreeAllMem() {}
    friend std::ostream& operator<< (std::ostream& out, const ON_SimpleMemMgr& item);
protected:
    int m_nAllocSize;
};


class ON_CLASS ON_EnhancedMemMgr
{
public:
  explicit ON_EnhancedMemMgr(int nAllocSize, int nChunkSize = 1024)
    : m_nAllocSize(nAllocSize), m_nChunkSize(nChunkSize)
  {
    AllocateChunk();  // 초기 청크 할당
  }

  virtual ~ON_EnhancedMemMgr()
  {
    FreeAllMem();
  }

  void* Alloc()
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_freeList) {
      AllocateChunk();
    }

    void* mem = m_freeList;
    m_freeList = m_freeList->pNext;
    return mem;
  }

  void Free(void* pMem)
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    FreeNode* node = static_cast<FreeNode*>(pMem);
    node->pNext = m_freeList;
    m_freeList = node;
  }

  void FreeAllMem()
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (void* chunk : m_chunks) {
      delete[] static_cast<char*>(chunk);
    }
    m_chunks.clear();
    m_freeList = nullptr;
  }

  template<typename T, typename... Args>
  T* AllocObject(Args&&... args)
  {
    void* rawMem = Alloc();
    return new(rawMem) T(std::forward<Args>(args)...);
  }

  template<typename T>
  void FreeObject(T* obj)
  {
    if (obj) {
      obj->~T();
      Free(static_cast<void*>(obj));
    }
  }

private:
  struct FreeNode
  {
    FreeNode* pNext;
  };

  void AllocateChunk()
  {
    char* chunk = new char[m_nAllocSize * m_nChunkSize];
    m_chunks.push_back(chunk);

    for (int i = 0; i < m_nChunkSize; ++i) {
      FreeNode* node = reinterpret_cast<FreeNode*>(chunk + i * m_nAllocSize);
      node->pNext = m_freeList;
      m_freeList = node;
    }
  }

private:
  int m_nAllocSize;
  int m_nChunkSize;

  FreeNode* m_freeList = nullptr;
  std::vector<void*> m_chunks;
  std::mutex m_mutex;
};


class ON_CLASS ON_SafeMemMgr : public ON_FixedSizeMemMgr
{
private:
    struct ObjectRecord
    {
        void* ptr;
        std::function<void()> destructor;
    };
    std::mutex m_listMutex;
    std::vector<ObjectRecord> m_allocatedObjects;

public:
    ON_SafeMemMgr(int allocSize, int chunkSize = 1024)
        : ON_FixedSizeMemMgr(allocSize, chunkSize)
    {
    }

    template<typename T, typename... Args>
    T* AllocObject(Args&&... args)
    {
        void* rawMem = Alloc();
        if (!rawMem) return nullptr;

        T* obj = new(rawMem) T(std::forward<Args>(args)...);

        std::function<void()> destructor = [obj]() {
            obj->~T();
            };

        {
            std::lock_guard<std::mutex> lock(m_listMutex);
            m_allocatedObjects.push_back({ static_cast<void*>(obj), destructor });
        }

        return obj;
    }

    template<typename T>
    void FreeObject(T* obj)
    {
        if (!obj) return;

        std::function<void()> destructor;

        {
            std::lock_guard<std::mutex> lock(m_listMutex);

            auto it = std::find_if(m_allocatedObjects.begin(), m_allocatedObjects.end(),
                [obj](const ObjectRecord& rec) {
                    return rec.ptr == static_cast<void*>(obj);
                });

            if (it != m_allocatedObjects.end()) {
                destructor = it->destructor;
                m_allocatedObjects.erase(it);
            }
            else {
                return;
            }
        }
        destructor();

        Free(static_cast<void*>(obj));
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
            rec.destructor();
            Free(rec.ptr);
        }

        FreeAllMem();
    }
};


class ON_CLASS ON_SafeMemMgrEx
{
private:
  struct ObjectRecord
  {
    void* ptr;
    std::function<void()> destructor;
    std::string tag;
  };

  struct Pool
  {
    std::unique_ptr<ON_FixedSizeMemMgr> memMgr;
    int allocSize;
  };

  std::unordered_map<std::string, std::vector<ObjectRecord>> m_taggedObjects;
  std::unordered_map<int, std::unique_ptr<Pool>> m_pools;
  std::mutex m_mutex;

public:
  ON_SafeMemMgrEx() = default;
  ~ON_SafeMemMgrEx() { FreeAllObjects(); }

  template<typename T, typename... Args>
  T* AllocObject(const std::string& tag, Args&&... args)
  {
    const int allocSize = sizeof(T);

    Pool* pool = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_pools.find(allocSize) == m_pools.end()) {
        auto newPool = std::make_unique<Pool>();
        newPool->allocSize = allocSize;
        newPool->memMgr = std::make_unique<ON_FixedSizeMemMgr>(allocSize);
        m_pools[allocSize] = std::move(newPool);
      }
      pool = m_pools[allocSize].get();
    }

    void* rawMem = pool->memMgr->Alloc();
    if (!rawMem) return nullptr;

    T* obj = new(rawMem) T(std::forward<Args>(args)...);

    std::function<void()> destructor = [obj]() {
      obj->~T();
      };

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_taggedObjects[tag].push_back({ static_cast<void*>(obj), destructor, tag });
    }

    return obj;
  }

  template<typename T>
  void FreeObject(const std::string& tag, T* obj)
  {
    if (!obj) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto& vec = m_taggedObjects[tag];
    auto it = std::find_if(vec.begin(), vec.end(), [obj](const ObjectRecord& rec) {
      return rec.ptr == static_cast<void*>(obj);
      });

    if (it != vec.end()) {
      it->destructor();
      const int allocSize = sizeof(T);
      if (m_pools.find(allocSize) != m_pools.end()) {
        m_pools[allocSize]->memMgr->Free(it->ptr);
      }
      vec.erase(it);
    }
  }

  void FreeObjectsByTag(const std::string& tag)
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_taggedObjects.find(tag);
    if (it == m_taggedObjects.end()) return;

    for (auto& rec : it->second) {
      rec.destructor();
      const int allocSize = static_cast<int>(rec.tag.size()); // 추정치가 아닌 실제 크기 사용 필요
      if (m_pools.find(allocSize) != m_pools.end()) {
        m_pools[allocSize]->memMgr->Free(rec.ptr);
      }
    }
    m_taggedObjects.erase(it);
  }

  void FreeAllObjects()
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& [tag, vec] : m_taggedObjects) {
      for (auto& rec : vec) {
        rec.destructor();
        const int allocSize = static_cast<int>(rec.tag.size()); // 실제 크기 사용 필요
        if (m_pools.find(allocSize) != m_pools.end()) {
          m_pools[allocSize]->memMgr->Free(rec.ptr);
        }
      }
    }
    m_taggedObjects.clear();

    for (auto& [size, pool] : m_pools) {
      pool->memMgr->FreeAllMem();
    }
    m_pools.clear();
  }
};

#pragma pack(pop)


#include "opennurbs.h"

ON_FixedSizeMemMgr::ON_FixedSizeMemMgr(int nAllocSize, int nChunkSize)
{
  ON_ASSERT(nAllocSize % 4 == 0);
  ON_ASSERT(nChunkSize >= 1024);
  m_nAllocSize = nAllocSize;
  m_nChunkSize = nChunkSize;
  m_pFree = 0;
  m_pChunk = 0;
}

ON_FixedSizeMemMgr::~ON_FixedSizeMemMgr(void)
{
  FreeAllMem();
}

void* ON_FixedSizeMemMgr::Alloc()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_pFree == 0) MakeNewChunk();
  void* pMem = m_pFree;
  m_pFree = m_pFree->pNext;
  return pMem;
}

void ON_FixedSizeMemMgr::Free(void* pMem)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ITEM* pItem = (ITEM*)pMem;
  pItem->pNext = m_pFree;
  m_pFree = pItem;
}

void ON_FixedSizeMemMgr::FreeAllMem()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  CHUNK* pChunk = m_pChunk;
  CHUNK* pChunkNext = 0;
  while (pChunk)
  {
    pChunkNext = pChunk->pNext;
    delete[] (char*)pChunk;
    pChunk = pChunkNext;
  }
  m_pChunk = 0;
  m_pFree = 0;
}

void ON_FixedSizeMemMgr::MakeNewChunk()
{
  char* pMem = new char[m_nChunkSize];
  CHUNK* pChunk = (CHUNK*)pMem;
  pChunk->pNext = m_pChunk;
  m_pChunk = pChunk;

  int nCountItem = (m_nChunkSize - sizeof(CHUNK)) / m_nAllocSize;
  ITEM* pItem = (ITEM*)(pMem + sizeof(CHUNK));
  assert(m_pFree == 0);
  m_pFree = pItem;
  for (int i = 0; i < nCountItem - 1; i++)
  {
    pItem->pNext = (ITEM*)((char*)pItem + m_nAllocSize);
    pItem = pItem->pNext;
  }
  pItem->pNext = 0;
}

std::ostream& operator<<(std::ostream& out, const ON_FixedSizeMemMgr& item)
{
  return out;
}

```
---
