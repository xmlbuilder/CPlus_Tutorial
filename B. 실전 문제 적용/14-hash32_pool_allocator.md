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
