## 소스 코드
```cpp
#pragma once

#include "hash_set32.h"          // Hash32Item, Hash32Table, DefaultHash32
#include "hash32_pool_allocator.h"
#include <functional>
#include <iterator>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace util_hash
{
  /// 중복 값 허용 Hash MultiSet
  /// - std::multiset 비슷하지만 해시 기반
  /// - equal_range(value) 지원
  template<
    typename T,
    typename Hash  = DefaultHash32<T>,
    typename Equal = std::equal_to<T>,
    std::size_t PoolBlockSize = 256
  >
  class Hash32MultiSet
  {
    struct Node : public Hash32Item
    {
      T value;

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

    NodePool    m_pool;

  public:
    class const_iterator
    {
      const Hash32Table* m_table = nullptr;
      Hash32Item*        m_item  = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = T;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const T*;
      using reference         = const T&;

      const_iterator() = default;

      const_iterator(const Hash32Table* table, Hash32Item* item)
        : m_table(table), m_item(item)
      {
      }

      reference operator*() const
      {
        Node* n = static_cast<Node*>(m_item);
        return n->value;
      }

      pointer operator->() const
      {
        Node* n = static_cast<Node*>(m_item);
        return &n->value;
      }

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

      Hash32Item* raw_item() const { return m_item; }
    };

    using iterator = const_iterator; // set 특성상 const 값만 노출

    // ----- 생성 / 소멸 -----
    Hash32MultiSet() = default;

    explicit Hash32MultiSet(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32MultiSet()
    {
      clear();
    }

    Hash32MultiSet(const Hash32MultiSet&)            = delete;
    Hash32MultiSet& operator=(const Hash32MultiSet&) = delete;

    // ----- 기본 프로퍼티 -----
    bool        empty() const noexcept { return m_size == 0; }
    std::size_t size()  const noexcept { return m_size; }

    // ----- iterator (테이블 순회 순서) -----
    iterator begin() const
    {
      return iterator(&m_table,
                      const_cast<Hash32Item*>(m_table.firstTableItem()));
    }

    iterator end() const
    {
      return iterator(&m_table, nullptr);
    }

    iterator cbegin() const { return begin(); }
    iterator cend()   const { return end();   }

  private:
    std::uint32_t make_hash32(const T& value) const
    {
      return m_hash(value);
    }

    Node* find_first_node(const T& value) const
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

  public:
    // ----- 조회 -----
    bool contains(const T& value) const
    {
      return find_first_node(value) != nullptr;
    }

    iterator find(const T& value) const
    {
      Node* n = find_first_node(value);
      if (!n) return end();
      return iterator(&m_table, static_cast<Hash32Item*>(n));
    }

    /// 동일 값들을 모두 커버하는 [first,last) 구간
    /// (주의: iterator는 해시테이블 전체 순회에 기반, key 이외 원소가 섞여 있을 수 있으니
    ///  실제 사용 시에는 *it 비교로 필터링하는 편이 안전합니다.)
    std::pair<iterator, iterator> equal_range(const T& value) const
    {
      if (m_size == 0)
        return { end(), end() };

      const std::uint32_t h = make_hash32(value);

      Hash32Item* first_item = nullptr;
      Hash32Item* last_item  = nullptr;

      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           it = m_table.nextItemWithHash(it))
      {
        Node* n = static_cast<Node*>(it);
        if (m_equal(n->value, value))
        {
          if (!first_item)
            first_item = it;
          last_item = it;
        }
      }

      if (!first_item)
        return { end(), end() };

      Hash32Item* next_after_last = m_table.nextTableItem(last_item);

      return {
        iterator(&m_table, first_item),
        iterator(&m_table, next_after_last)
      };
    }

    // ----- 삽입 -----
    bool insert(const T& value)
    {
      const std::uint32_t h = make_hash32(value);
      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(value);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(Args&&... args)
    {
      T tmp(std::forward<Args>(args)...);
      const std::uint32_t h = make_hash32(tmp);

      Node* node_mem = m_pool.allocate();
      new (node_mem) Node(std::in_place, std::forward<Args>(args)...);

      if (!m_table.addItem(h, node_mem))
      {
        m_pool.destroy(node_mem);
        return false;
      }

      ++m_size;
      return true;
    }

    // ----- 삭제 -----
    /// value와 equal()인 모든 요소 삭제, 개수 반환
    std::size_t erase(const T& value)
    {
      if (m_size == 0)
        return 0;

      const std::uint32_t h = make_hash32(value);
      std::size_t removed = 0;

      for (Hash32Item* it = m_table.firstItemWithHash(h);
           it;
           )
      {
        Hash32Item* next_same_hash = m_table.nextItemWithHash(it);
        Node*       n = static_cast<Node*>(it);

        if (m_equal(n->value, value))
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
```

### 테스트 코드
```cpp
#include "hash_set32.h"
#include "hash32_pool_allocator.h"
#include "hash32_ordered_set.h"
#include "hash32_multiset.h"
#include <iostream>
#include <string>

int main()
{
  using util_hash::Hash32OrderedSet;
  using util_hash::Hash32MultiSet;

  // 1) 삽입 순서 유지 Set
  Hash32OrderedSet<std::string> oset;
  oset.insert("apple");
  oset.insert("banana");
  oset.insert("cherry");
  oset.insert("banana"); // 중복 무시

  std::cout << "[OrderedSet]\n";
  for (auto& v : oset)
    std::cout << v << "\n";

  // 2) MultiSet (중복 허용)
  Hash32MultiSet<int> mset;
  mset.insert(10);
  mset.insert(10);
  mset.insert(20);
  mset.insert(10);

  std::cout << "\n[MultiSet all]\n";
  for (auto& v : mset)
    std::cout << v << "\n";

  auto [first, last] = mset.equal_range(10);
  std::cout << "\n[MultiSet equal_range(10)]\n";
  for (auto it = first; it != last; ++it)
  {
    if (*it == 10) // 실제 값 필터링 권장
      std::cout << *it << "\n";
  }

  return 0;
}
```
### 테스트 결과
```
[OrderedSet]
apple
banana
cherry

[MultiSet all]
20
10
10
10

[MultiSet equal_range(10)]
10
10
10
```
