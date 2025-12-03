## 소스 코드
```cpp
#pragma once

#include "hash_set32.h"          // Hash32Item / Hash32Table / DefaultHash32
#include "hash32_pool_allocator.h"
#include <functional>
#include <iterator>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <tuple>
#include <stdexcept>

namespace util_hash
{
  /// 삽입 순서를 유지하는 Hash Map
  /// - iterator는 삽입 순서대로 순회
  template<
    typename Key,
    typename T,
    typename Hash  = DefaultHash32<Key>,
    typename Equal = std::equal_to<Key>,
    std::size_t PoolBlockSize = 256
  >
  class Hash32OrderedMap
  {
    struct Node : public Hash32Item
    {
      Key key;
      T   value;

      Node* prev_order = nullptr;
      Node* next_order = nullptr;

      Node(const Key& k, const T& v)
        : key(k), value(v)
      {
      }

      template<typename... Args>
      Node(std::piecewise_construct_t,
           std::tuple<Key> ktuple,
           std::tuple<Args...> vtuple)
        : key(std::move(std::get<0>(ktuple)))
      {
        construct_value(std::move(vtuple), std::index_sequence_for<Args...>{});
      }

      template<typename... Args, std::size_t... Is>
      void construct_value(std::tuple<Args...>&& t, std::index_sequence<Is...>)
      {
        new (&value) T(std::forward<Args>(std::get<Is>(t))...);
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

    struct ValueRef
    {
      const Key& first;
      T&         second;
    };

  public:
    class const_iterator;

    class iterator
    {
      Node* m_node = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = ValueRef*;   // 실제 포인터는 사용하지 않는 게 좋음
      using reference         = ValueRef;

      iterator() = default;
      explicit iterator(Node* n) : m_node(n) {}

      reference operator*() const
      {
        return ValueRef{ m_node->key, m_node->value };
      }

      pointer operator->() const = delete;

      iterator& operator++()
      {
        if (m_node) m_node = m_node->next_order;
        return *this;
      }

      iterator operator++(int)
      {
        iterator tmp(*this);
        ++(*this);
        return tmp;
      }

      friend bool operator==(const iterator& a, const iterator& b)
      {
        return a.m_node == b.m_node;
      }

      friend bool operator!=(const iterator& a, const iterator& b)
      {
        return !(a == b);
      }

      Node* node() const { return m_node; }
    };

    class const_iterator
    {
      const Node* m_node = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const ValueRef*;
      using reference         = ValueRef;

      const_iterator() = default;
      explicit const_iterator(const Node* n) : m_node(n) {}

      reference operator*() const
      {
        Node* nn = const_cast<Node*>(m_node);
        return ValueRef{ nn->key, nn->value };
      }

      pointer operator->() const = delete;

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

    // ----- 생성 / 소멸 -----
    Hash32OrderedMap() = default;

    explicit Hash32OrderedMap(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32OrderedMap()
    {
      clear();
    }

    Hash32OrderedMap(const Hash32OrderedMap&)            = delete;
    Hash32OrderedMap& operator=(const Hash32OrderedMap&) = delete;

    // ----- 기본 프로퍼티 -----
    bool empty() const noexcept { return m_size == 0; }
    std::size_t size() const noexcept { return m_size; }

    // ----- iterator (삽입 순서) -----
    iterator begin() { return iterator(m_head); }
    iterator end()   { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(m_head); }
    const_iterator end()   const { return const_iterator(nullptr); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end();   }

  private:
    std::uint32_t make_hash32(const Key& key) const
    {
      std::uint32_t h = m_hash(key);
      return h;
    }

    Node* find_node(const Key& key) const
    {
      if (m_size == 0)
        return nullptr;

      const std::uint32_t h = make_hash32(key);

      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           it = m_table.nextItemWithHash(it))
      {
        Node* n = static_cast<Node*>(it);
        if (m_equal(n->key, key))
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
    bool contains(const Key& key) const
    {
      return find_node(key) != nullptr;
    }

    iterator find(const Key& key)
    {
      Node* n = find_node(key);
      return n ? iterator(n) : end();
    }

    const_iterator find(const Key& key) const
    {
      Node* n = find_node(key);
      return n ? const_iterator(n) : end();
    }

    T& at(const Key& key)
    {
      Node* n = find_node(key);
      if (!n)
        throw std::out_of_range("Hash32OrderedMap::at: key not found");
      return n->value;
    }

    const T& at(const Key& key) const
    {
      Node* n = find_node(key);
      if (!n)
        throw std::out_of_range("Hash32OrderedMap::at: key not found");
      return n->value;
    }

    T& operator[](const Key& key)
    {
      Node* n = find_node(key);
      if (n) return n->value;

      std::uint32_t h = make_hash32(key);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(key, T{});

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        throw std::bad_alloc();
      }

      link_back(node_mem);
      ++m_size;
      return node_mem->value;
    }

    // ----- 삽입 -----
    bool insert(const Key& key, const T& value)
    {
      if (find_node(key))
        return false;

      std::uint32_t h = make_hash32(key);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(key, value);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      link_back(node_mem);
      ++m_size;
      return true;
    }

    bool insert_or_assign(const Key& key, const T& value)
    {
      Node* n = find_node(key);
      if (n)
      {
        n->value = value;
        return false;
      }

      std::uint32_t h = make_hash32(key);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(key, value);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        throw std::bad_alloc();
      }

      link_back(node_mem);
      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(const Key& key, Args&&... args)
    {
      if (find_node(key))
        return false;

      std::uint32_t h = make_hash32(key);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(std::piecewise_construct,
                          std::make_tuple(key),
                          std::make_tuple(std::forward<Args>(args)...));

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
    bool erase(const Key& key)
    {
      Node* node = find_node(key);
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
      // 삽입 순서 리스트 따라가며 노드 소멸
      Node* cur = m_head;
      while (cur)
      {
        Node* next = cur->next_order;
        m_table.removeItem(cur);  // 내부 연결도 끊어줌
        m_pool.destroy(cur);
        cur = next;
      }

      m_head = m_tail = nullptr;
      m_size = 0;
      m_table.removeAllItems();
      m_pool.reset();
    }
  };

} // namespace util_hash
````
