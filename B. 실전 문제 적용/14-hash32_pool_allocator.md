## 1. Hash32PoolAllocator 특징 정리
```cpp
template<typename T, std::size_t BlockSize = 256>
class Hash32PoolAllocator;
```

### 1) 블록 단위(raw memory) 할당
- 한 번에 BlockSize 개의 T를 넣을 수 있는 메모리 블록을 operator new[]로 할당.
- 각 블록은
```cpp
struct Block {
  char*       memory;   // raw memory
  std::size_t capacity; // BlockSize
  std::size_t used;     // 현재까지 할당된 개수
};
```

- allocate()가 호출될 때:
  - 현재 블록에 빈 슬롯이 있으면 그 슬롯 사용
  - 없으면 새 블록을 만들어서 거기서 다시 할당

- 많은 노드를 한 번에 잡아두고, 그 안에서만 쪼개 써서 new T, delete T 를 수백/수천 번 반복하는 걸 피함.

### 2) free list를 이용한 재사용
```cpp
struct NodeSlot {
  NodeSlot* next;
};

NodeSlot* m_free_list;
```
- destroy(T* p) :
  - p->~T() 소멸자 호출
  - 해당 메모리를 NodeSlot로 간주해서 m_free_list에 push

- allocate() :
  - m_free_list에 노드가 있으면 새 블록을 만들지 않고 거기서 재사용

- 자주 생성/삭제되는 Node에 대해 메모리 재사용 + 캐시 locality 유지 효과.

### 3) 소멸자 호출과 수명 관리
- 중요한 점: Hash32PoolAllocator는 raw memory 풀만 관리하고, 객체 수명은 사용자가 책임집니다.
- 규칙:
  - T* p = pool.allocate();
  - new (p) T(args...); ← placement new
  - 사용 끝나면 pool.destroy(p); ← 여기서 ~T() 호출 + free list에 push
- ~Hash32PoolAllocator():
  - T의 소멸자를 자동으로 호출하지 않습니다.
  - 이미 destroy()로 모든 T가 파괴되어 있다고 가정하고, 블록 메모리만 operator delete[]로 해제.

- HashSet/HashMap 래퍼에서 destroy()를 항상 호출하도록 묶어놨던 이유가 이거입니다.

### 4) reset()으로 재사용
```cpp
void reset()
{
  m_free_list = nullptr;
  for (auto& blk : m_blocks) blk.used = 0;
  m_current_block = m_blocks.empty() ? nullptr : &m_blocks.front();
}
```
- 모든 블록의 used를 0으로 돌리고, free list를 비움.

- 이미 destroy()로 소멸된 T라는 전제 하에서 **같은 메모리 풀을 다시 깨끗하게 쓰겠다** 는 용도.
- 보통 컨테이너 전체 clear() 후, 다시 그 풀을 재사용할 때 유용합니다.

### 5) 어디에 적합한가?
- Hash32Set / Hash32Map / Hash32OrderedMap / Hash32MultiMap 노드처럼
  - 같은 타입 T가 매우 많이 생성/삭제되고
  - lifetime이 컨테이너와 같이 움직이는 경우
- 일반 힙 할당보다:
  - 할당/해제 오버헤드 감소
  - 메모리 파편화 감소
  - 캐시 친화적인 배치

## 소스 코드
```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>

namespace util_hash
{
  /// HashSet/HashMap 노드용 간단 메모리 풀
  /// - T: 노드 타입 (ex. Hash32OrderedMap<Key,T>::Node)
  /// - BlockSize: 한 번에 할당하는 노드 개수
  template<typename T, std::size_t BlockSize = 256>
  class Hash32PoolAllocator
  {
  public:
    Hash32PoolAllocator() = default;

    ~Hash32PoolAllocator()
    {
      // 사용자가 destroy()를 통해 모든 노드의 소멸자를 호출했다는 전제
      for (auto& blk : m_blocks)
      {
        ::operator delete[](blk.memory);
      }
    }

    Hash32PoolAllocator(const Hash32PoolAllocator&) = delete;
    Hash32PoolAllocator& operator=(const Hash32PoolAllocator&) = delete;

    /// T 하나를 위한 생 메모리 포인터 반환 (생성자는 호출되지 않음)
    T* allocate()
    {
      if (m_free_list)
      {
        // free list에 있으면 재사용
        NodeSlot* slot = m_free_list;
        m_free_list = m_free_list->next;
        return reinterpret_cast<T*>(slot);
      }

      if (!m_current_block || m_current_block->used == m_current_block->capacity)
      {
        add_block();
      }

      std::size_t idx = m_current_block->used++;
      char* ptr = m_current_block->memory + idx * sizeof(T);
      return reinterpret_cast<T*>(ptr);
    }

    /// T* p에 대해 소멸자를 호출하고 free list에 넣음
    void destroy(T* p)
    {
      if (!p) return;
      p->~T();

      NodeSlot* slot = reinterpret_cast<NodeSlot*>(p);
      slot->next = m_free_list;
      m_free_list = slot;
    }

    /// 풀 초기화 (할당된 raw 메모리 블록은 유지, free list 초기화)
    /// 컨테이너 전체 clear 후 재사용할 때 사용 가능
    void reset()
    {
      m_free_list = nullptr;
      for (auto& blk : m_blocks)
      {
        blk.used = 0;
      }
      if (!m_blocks.empty())
        m_current_block = &m_blocks.front();
      else
        m_current_block = nullptr;
    }

  private:
    struct NodeSlot
    {
      NodeSlot* next;
    };

    struct Block
    {
      char*       memory = nullptr;
      std::size_t capacity = 0;
      std::size_t used     = 0;
    };

    void add_block()
    {
      Block blk;
      blk.capacity = BlockSize;
      blk.used     = 0;
      blk.memory   = static_cast<char*>(
        ::operator new[](blk.capacity * sizeof(T), std::nothrow)
      );
      if (!blk.memory)
        throw std::bad_alloc();

      m_blocks.push_back(blk);
      m_current_block = &m_blocks.back();
    }

    std::vector<Block> m_blocks;
    Block*             m_current_block = nullptr;
    NodeSlot*          m_free_list     = nullptr;
  };

} // namespace util_hash
```
---

## 테스트 코드 예제

아래 코드는 hash32_pool_allocator.h를 테스트하는 단독 실행용 main입니다.
- TestNode:
  - 생성/소멸 시 live_count를 조정

- 테스트 내용:
  - 기본 할당/소멸이 정상인지
  - free list 재사용이 되는지(주소 재사용 확인)
  - 여러 블록을 사용할 때도 문제 없는지
  - reset() 호출 후 다시 할당이 정상인지

```cpp
// test_hash32_pool_allocator.cpp
#include <iostream>
#include <vector>
#include <cassert>
#include "hash32_pool_allocator.h"

using namespace util_hash;

struct TestNode
{
  static int live_count;
  int id;

  explicit TestNode(int v = 0)
    : id(v)
  {
    ++live_count;
  }

  ~TestNode()
  {
    --live_count;
  }
};

int TestNode::live_count = 0;

bool test_basic_allocation()
{
  std::cout << "[TEST] basic allocation / destroy\n";

  Hash32PoolAllocator<TestNode, 4> pool; // 블록 당 4개

  {
    std::vector<TestNode*> ptrs;
    ptrs.reserve(8);

    // 8개 할당 (2블록 이상 쓸 것)
    for (int i = 0; i < 8; ++i)
    {
      TestNode* p = pool.allocate();
      new (p) TestNode(i); // placement new
      ptrs.push_back(p);
    }

    assert(TestNode::live_count == 8);

    // 값 확인
    for (int i = 0; i < 8; ++i)
    {
      assert(ptrs[i]->id == i);
    }

    // destroy 호출
    for (auto* p : ptrs)
    {
      pool.destroy(p);
    }

    assert(TestNode::live_count == 0);
  }

  std::cout << "  OK\n";
  return true;
}

bool test_reuse_free_list()
{
  std::cout << "[TEST] reuse free list\n";

  Hash32PoolAllocator<TestNode, 4> pool;

  // 첫 2개 할당
  TestNode* a = pool.allocate();
  new (a) TestNode(1);

  TestNode* b = pool.allocate();
  new (b) TestNode(2);

  assert(TestNode::live_count == 2);

  void* addr_a = static_cast<void*>(a);
  void* addr_b = static_cast<void*>(b);

  // a를 destroy -> free list에 들어감
  pool.destroy(a);
  assert(TestNode::live_count == 1);

  // 다시 allocate -> a의 메모리를 재사용해야 함
  TestNode* c = pool.allocate();
  new (c) TestNode(3);

  void* addr_c = static_cast<void*>(c);

  // free list 재사용 확인: c 주소 == a 주소
  assert(addr_c == addr_a);

  // 정리
  pool.destroy(b);
  pool.destroy(c);
  assert(TestNode::live_count == 0);

  std::cout << "  OK\n";
  return true;
}

bool test_multiple_blocks_and_reset()
{
  std::cout << "[TEST] multiple blocks + reset\n";

  Hash32PoolAllocator<TestNode, 2> pool; // 일부러 작은 블록

  std::vector<TestNode*> ptrs;

  // 6개 할당 -> 블록 3개 이상 사용
  for (int i = 0; i < 6; ++i)
  {
    TestNode* p = pool.allocate();
    new (p) TestNode(i);
    ptrs.push_back(p);
  }
  assert(TestNode::live_count == 6);

  // destroy(K 일부만)
  for (int i = 0; i < 3; ++i)
  {
    pool.destroy(ptrs[i]);  // 앞의 3개만 파괴
  }
  assert(TestNode::live_count == 3);

  // 나머지도 모두 파괴
  for (int i = 3; i < 6; ++i)
  {
    pool.destroy(ptrs[i]);
  }
  assert(TestNode::live_count == 0);

  // reset 후 다시 사용
  pool.reset();

  TestNode* q1 = pool.allocate();
  new (q1) TestNode(100);

  TestNode* q2 = pool.allocate();
  new (q2) TestNode(200);

  assert(TestNode::live_count == 2);
  assert(q1->id == 100);
  assert(q2->id == 200);

  pool.destroy(q1);
  pool.destroy(q2);
  assert(TestNode::live_count == 0);

  std::cout << "  OK\n";
  return true;
}

int main()
{
  bool ok = true;
  ok = ok && test_basic_allocation();
  ok = ok && test_reuse_free_list();
  ok = ok && test_multiple_blocks_and_reset();

  if (ok)
  {
    std::cout << "\n=== ALL Hash32PoolAllocator TESTS PASSED ===\n";
    return 0;
  }
  else
  {
    std::cerr << "\n=== SOME TESTS FAILED ===\n";
    return 1;
  }
}
```
---

