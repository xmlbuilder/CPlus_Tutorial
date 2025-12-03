## 소스 코드
```cpp
#pragma once

#include "hash_set32.h"          // Hash32Item, Hash32Table, DefaultHash32
#include "hash32_pool_allocator.h"
#include <functional>
#include <iterator>
#include <cstddef>
#include <cstdint>

namespace util_hash
{
  /// 삽입 순서를 유지하는 Hash Set
  /// - iterator는 삽입 순서대로 순회
  /// - set 특성상 외부에는 const T& 만 노출 (값을 바꾸면 안 됨)
  template<
    typename T,
    typename Hash  = DefaultHash32<T>,
    typename Equal = std::equal_to<T>,
    std::size_t PoolBlockSize = 256
  >
  class Hash32OrderedSet
  {
    struct Node : public Hash32Item
    {
      T value;

      Node* prev_order = nullptr;
      Node* next_order = nullptr;

      explicit Node(const T& v)
        : value(v)
      {
      }

      template<typename... Args>
      explicit Node(std::in_place_t, Args&&... args)
        : value(std::forward<Args>(args)...)
      {
      }
    };

    using NodePool = Hash32PoolAllocator<Node, PoolBlockSize>;

    Hash32Table m_table;
    std::size_t m_size  = 0;
    Hash        m_hash;
    Equal       m_equal;

    Node*       m_head = nullptr;
    Node*       m_tail = nullptr;

    NodePool    m_pool;

  public:
    class const_iterator
    {
      const Node* m_node = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = T;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const T*;
      using reference         = const T&;

      const_iterator() = default;
      explicit const_iterator(const Node* n) : m_node(n) {}

      reference operator*() const { return m_node->value; }
      pointer   operator->() const { return &m_node->value; }

      const_iterator& operator++()
      {
        if (m_node) m_node = m_node->next_order;
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator tmp(*this);
        ++(*this);
        return tmp;
      }

      friend bool operator==(const const_iterator& a, const const_iterator& b)
      {
        return a.m_node == b.m_node;
      }

      friend bool operator!=(const const_iterator& a, const const_iterator& b)
      {
        return !(a == b);
      }
    };

    using iterator = const_iterator; // set 특성상 const만 제공

    // ----- 생성 / 소멸 -----
    Hash32OrderedSet() = default;

    explicit Hash32OrderedSet(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32OrderedSet()
    {
      clear();
    }

    Hash32OrderedSet(const Hash32OrderedSet&)            = delete;
    Hash32OrderedSet& operator=(const Hash32OrderedSet&) = delete;

    // ----- 기본 프로퍼티 -----
    bool        empty() const noexcept { return m_size == 0; }
    std::size_t size()  const noexcept { return m_size; }

    // ----- iterator (삽입 순서) -----
    iterator begin() const { return iterator(m_head); }
    iterator end()   const { return iterator(nullptr); }

    iterator cbegin() const { return begin(); }
    iterator cend()   const { return end();   }

  private:
    std::uint32_t make_hash32(const T& value) const
    {
      return m_hash(value);
    }

    Node* find_node(const T& value) const
    {
      if (m_size == 0)
        return nullptr;

      const std::uint32_t h = make_hash32(value);

      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           it = m_table.nextItemWithHash(it))
      {
        Node* n = static_cast<Node*>(it);
        if (m_equal(n->value, value))
          return n;
      }
      return nullptr;
    }

    void link_back(Node* node)
    {
      node->prev_order = m_tail;
      node->next_order = nullptr;
      if (m_tail)
        m_tail->next_order = node;
      else
        m_head = node;
      m_tail = node;
    }

    void unlink(Node* node)
    {
      if (node->prev_order)
        node->prev_order->next_order = node->next_order;
      else
        m_head = node->next_order;

      if (node->next_order)
        node->next_order->prev_order = node->prev_order;
      else
        m_tail = node->prev_order;

      node->prev_order = node->next_order = nullptr;
    }

  public:
    // ----- 조회 -----
    bool contains(const T& value) const
    {
      return find_node(value) != nullptr;
    }

    iterator find(const T& value) const
    {
      Node* n = find_node(value);
      return n ? iterator(n) : end();
    }

    // ----- 삽입 -----
    bool insert(const T& value)
    {
      if (find_node(value))
        return false; // set: 중복 허용 안 함

      const std::uint32_t h = make_hash32(value);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(value);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      link_back(node_mem);
      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(Args&&... args)
    {
      T tmp(std::forward<Args>(args)...);
      if (find_node(tmp))
        return false;

      const std::uint32_t h = make_hash32(tmp);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(std::in_place, std::forward<Args>(args)...);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      link_back(node_mem);
      ++m_size;
      return true;
    }

    // ----- 삭제 -----
    bool erase(const T& value)
    {
      Node* node = find_node(value);
      if (!node)
        return false;

      if (!m_table.removeItem(node))
        return false;

      unlink(node);
      m_pool.destroy(node);
      --m_size;
      return true;
    }

    void clear()
    {
      Node* cur = m_head;
      while (cur)
      {
        Node* next = cur->next_order;
        m_table.removeItem(cur);  // 해시 테이블에서 제거
        m_pool.destroy(cur);      // 소멸 + free list
        cur = next;
      }

      m_head = m_tail = nullptr;
      m_size = 0;
      m_table.removeAllItems();
      m_pool.reset();
    }
  };

} // namespace util_hash

```
---
