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

namespace util_hash
{
  /// 중복 키 허용 Hash MultiMap
  /// - std::multimap 유사 (단, 해시 기반)
  template<
    typename Key,
    typename T,
    typename Hash  = DefaultHash32<Key>,
    typename Equal = std::equal_to<Key>,
    std::size_t PoolBlockSize = 256
  >
  class Hash32MultiMap
  {
    struct Node : public Hash32Item
    {
      Key key;
      T   value;

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
      Hash32Table* m_table = nullptr;
      Hash32Item*  m_item  = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = ValueRef*;
      using reference         = ValueRef;

      iterator() = default;

      iterator(Hash32Table* table, Hash32Item* item)
        : m_table(table), m_item(item)
      {
      }

      reference operator*() const
      {
        Node* n = static_cast<Node*>(m_item);
        return ValueRef{ n->key, n->value };
      }

      pointer operator->() const = delete;

      iterator& operator++()
      {
        if (m_table && m_item)
          m_item = m_table->nextTableItem(m_item);
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
        return a.m_item == b.m_item && a.m_table == b.m_table;
      }

      friend bool operator!=(const iterator& a, const iterator& b)
      {
        return !(a == b);
      }

      Hash32Item* item() const { return m_item; }
    };

    class const_iterator
    {
      const Hash32Table* m_table = nullptr;
      Hash32Item*        m_item  = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const ValueRef*;
      using reference         = ValueRef;

      const_iterator() = default;

      const_iterator(const Hash32Table* table, Hash32Item* item)
        : m_table(table), m_item(item)
      {
      }

      reference operator*() const
      {
        Node* n = static_cast<Node*>(m_item);
        return ValueRef{ n->key, n->value };
      }

      pointer operator->() const = delete;

      const_iterator& operator++()
      {
        if (m_table && m_item)
          m_item = const_cast<Hash32Table*>(m_table)->nextTableItem(m_item);
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
        return a.m_item == b.m_item && a.m_table == b.m_table;
      }

      friend bool operator!=(const const_iterator& a, const const_iterator& b)
      {
        return !(a == b);
      }
    };

    // ----- 생성 / 소멸 -----
    Hash32MultiMap() = default;

    explicit Hash32MultiMap(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32MultiMap()
    {
      clear();
    }

    Hash32MultiMap(const Hash32MultiMap&)            = delete;
    Hash32MultiMap& operator=(const Hash32MultiMap&) = delete;

    // ----- 기본 프로퍼티 -----
    bool empty() const noexcept { return m_size == 0; }
    std::size_t size() const noexcept { return m_size; }

    // ----- iterator (테이블 순회 순서) -----
    iterator begin()
    {
      return iterator(&m_table, m_table.firstTableItem());
    }

    iterator end()
    {
      return iterator(&m_table, nullptr);
    }

    const_iterator begin() const
    {
      return const_iterator(&m_table,
                            const_cast<Hash32Item*>(m_table.firstTableItem()));
    }

    const_iterator end() const
    {
      return const_iterator(&m_table, nullptr);
    }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end();   }

  private:
    std::uint32_t make_hash32(const Key& key) const
    {
      std::uint32_t h = m_hash(key);
      return h;
    }

    Node* find_first_node(const Key& key) const
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

  public:
    // ----- 조회 -----
    /// key에 해당하는 첫 노드 (없으면 end())
    iterator find(const Key& key)
    {
      Node* n = find_first_node(key);
      if (!n) return end();
      return iterator(&m_table, static_cast<Hash32Item*>(n));
    }

    const_iterator find(const Key& key) const
    {
      Node* n = find_first_node(key);
      if (!n) return end();
      return const_iterator(&m_table, static_cast<Hash32Item*>(n));
    }

    /// key가 하나라도 존재하는지 여부
    bool contains(const Key& key) const
    {
      return find_first_node(key) != nullptr;
    }

    /// 동일 key를 가진 모든 요소의 [first,last) 구간
    /// - first: 첫 매칭
    /// - last: 마지막 매칭 바로 다음 iterator
    std::pair<iterator, iterator> equal_range(const Key& key)
    {
      if (m_size == 0)
        return { end(), end() };

      const std::uint32_t h = make_hash32(key);

      Hash32Item* first_item = nullptr;
      Hash32Item* last_item  = nullptr;

      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           it = m_table.nextItemWithHash(it))
      {
        Node* n = static_cast<Node*>(it);
        if (m_equal(n->key, key))
        {
          if (!first_item)
            first_item = it;
          last_item = it;
        }
      }

      if (!first_item)
        return { end(), end() };

      // last_item의 다음 전체순회 요소
      Hash32Item* next_after_last = m_table.nextTableItem(last_item);

      return {
        iterator(&m_table, first_item),
        iterator(&m_table, next_after_last)
      };
    }

    // const 버전 equal_range는 필요시 비슷하게 구현 가능

    // ----- 삽입 -----
    bool insert(const Key& key, const T& value)
    {
      std::uint32_t h = make_hash32(key);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(key, value);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(const Key& key, Args&&... args)
    {
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

      ++m_size;
      return true;
    }

    // ----- 삭제 -----
    /// key에 해당하는 모든 요소 삭제, 삭제된 개수 반환
    std::size_t erase(const Key& key)
    {
      if (m_size == 0)
        return 0;

      const std::uint32_t h = make_hash32(key);
      std::size_t removed = 0;

      // 같은 hash 버킷에서만 검사
      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           )
      {
        Hash32Item* next_same_hash = m_table.nextItemWithHash(it);
        Node*       n = static_cast<Node*>(it);

        if (m_equal(n->key, key))
        {
          m_table.removeItem(n);
          m_pool.destroy(n);
          --m_size;
          ++removed;
        }

        it = next_same_hash;
      }

      return removed;
    }

    void clear()
    {
      for (Hash32Item* it = m_table.firstTableItem();
           it;
           )
      {
        Hash32Item* next = m_table.nextTableItem(it);
        Node*       node = static_cast<Node*>(it);
        m_pool.destroy(node);
        it = next;
      }

      m_table.removeAllItems();
      m_size = 0;
      m_pool.reset();
    }
  };

} // namespace util_hash

````

### 테스트 코드
```cpp
#include "hash_set32.h"
#include "hash32_pool_allocator.h"
#include "hash32_ordered_map.h"
#include "hash32_multimap.h"
#include <string>
#include <iostream>

int main()
{
  using util_hash::Hash32OrderedMap;
  using util_hash::Hash32MultiMap;

  // 1) 삽입 순서 유지 맵
  Hash32OrderedMap<std::string, int> omap;
  omap.insert("apple",  1);
  omap.insert("banana", 2);
  omap.insert("cherry", 3);

  omap.insert_or_assign("banana", 20); // 값 덮어쓰기, 순서는 그대로

  std::cout << "[OrderedMap]\n";
  for (auto it = omap.begin(); it != omap.end(); ++it)
  {
    auto [k, v] = *it;
    std::cout << k << " -> " << v << "\n";
  }

  // 2) 중복 키 허용 멀티맵
  Hash32MultiMap<std::string, int> mmap;
  mmap.insert("tag", 1);
  mmap.insert("tag", 2);
  mmap.insert("tag", 3);
  mmap.insert("other", 100);

  auto [first, last] = mmap.equal_range("tag");
  std::cout << "\n[MultiMap: tag]\n";
  for (auto it = first; it != last; ++it)
  {
    auto [k, v] = *it;
    std::cout << k << " -> " << v << "\n";
  }

  std::cout << "\nDone.\n";
  return 0;
}

```
