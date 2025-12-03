# hash32_ordered_map
- hash32_ordered_map 은 한마디로 말하면:
  - 삽입 순서를 기억하는 해시 기반 map (내부는 Hash32Table + intrusive Node + Hash32PoolAllocator) 입니다.

## 1. Hash32OrderedMap 구조와 특징
템플릿 정의:
```cpp
template<
  typename Key,
  typename T,
  typename Hash  = DefaultHash32<Key>,
  typename Equal = std::equal_to<Key>,
  std::size_t PoolBlockSize = 256
>
class Hash32OrderedMap;
```

- 내부 구성

- Node 구조
```cpp
struct Node : public Hash32Item
{
  Key key;
  T   value;

  Node* prev_order = nullptr;
  Node* next_order = nullptr;
};
```

- Hash32Item 상속 → 해시 테이블에서 쓰는 intrusive node
- prev_order / next_order → 삽입 순서 유지용 이중 연결 리스트

### 해시 테이블
```cpp
Hash32Table m_table;
```
- hash32_set.h에서 만든 intrusive 해시 테이블
- key 해시값으로 bucket 선택 → 체이닝으로 Node 연결

### 메모리 풀
```cpp
using NodePool = Hash32PoolAllocator<Node, PoolBlockSize>;
NodePool m_pool;
```
- Node를 new/delete 대신 풀에서 할당/해제
- 대량 삽입/삭제 시 성능 + 메모리 효율 ↑

### 삽입 순서 리스트

```cpp
Node* m_head = nullptr;
Node* m_tail = nullptr;
```

- 모든 Node를 삽입 순서대로 이중 연결 리스트로 묶어둠
- begin()/end() iterator 는 이 리스트를 따라 순회

## 주요 특징

### 삽입 순서 유지

- insert, insert_or_assign, operator[], emplace 로 들어간 키들은 처음 삽입된 순서대로 유지
- insert_or_assign 으로 값만 바꾸면 순서는 그대로 유지

### Key → Value 맵핑 + O(1) 평균 조회
- find, contains, at, operator[] 모두 해시 기반
- 평균 시간 복잡도: O(1)

### 키는 유일, 값은 수정 가능
- 이미 존재하는 키에 대해 insert → 실패 (false 반환)
- insert_or_assign → 존재하면 값 덮어쓰기

### 메모리 풀 기반 Node 관리
- pool.allocate() + placement new 로 Node 생성
- erase / clear 시에는 pool.destroy(node) 로 소멸자 호출 + free list에 반환
- 컨테이너 수명 동안 Node 메모리를 효율적으로 재사용

--- 

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
```
---

## 테스트 코드 예제 (hash32_ordered_map 전용)
```cpp
#include <iostream>
#include <string>
#include <cassert>

#include "hash_set32.h"          // Hash32Item, Hash32Table, DefaultHash32
#include "hash32_pool_allocator.h"
#include "hash32_ordered_map.h"  // 우리가 만든 Hash32OrderedMap

using util_hash::Hash32OrderedMap;

bool test_insert_and_order()
{
  std::cout << "[TEST] insert & order\n";

  Hash32OrderedMap<std::string, int> m;

  // 삽입 순서: apple, banana, cherry
  bool ok1 = m.insert("apple",  1);
  bool ok2 = m.insert("banana", 2);
  bool ok3 = m.insert("cherry", 3);

  assert(ok1 && ok2 && ok3);
  assert(m.size() == 3);

  // 삽입 순서 확인
  const char* expected_keys[] = { "apple", "banana", "cherry" };
  int         expected_vals[] = { 1, 2, 3 };

  int idx = 0;
  for (auto it = m.begin(); it != m.end(); ++it, ++idx)
  {
    auto pair = *it; // ValueRef { const Key& first, T& second }
    const std::string& k = pair.first;
    int&               v = pair.second;

    assert(k == expected_keys[idx]);
    assert(v == expected_vals[idx]);
  }
  assert(idx == 3);

  std::cout << "  OK\n";
  return true;
}

bool test_insert_or_assign_and_order()
{
  std::cout << "[TEST] insert_or_assign keeps order\n";

  Hash32OrderedMap<std::string, int> m;

  m.insert("apple",  1);
  m.insert("banana", 2);
  m.insert("cherry", 3);

  // 기존 키 banana의 값을 변경 (순서는 바뀌면 안 됨)
  bool inserted_new = m.insert_or_assign("banana", 20);
  assert(inserted_new == false); // 새로 삽입된 것은 아니다
  assert(m.size() == 3);

  const char* expected_keys[] = { "apple", "banana", "cherry" };
  int         expected_vals[] = { 1, 20, 3 };

  int idx = 0;
  for (auto it = m.begin(); it != m.end(); ++it, ++idx)
  {
    auto pair = *it;
    const std::string& k = pair.first;
    int&               v = pair.second;

    assert(k == expected_keys[idx]);
    assert(v == expected_vals[idx]);
  }
  assert(idx == 3);

  std::cout << "  OK\n";
  return true;
}

bool test_operator_brackets()
{
  std::cout << "[TEST] operator[] behavior\n";

  Hash32OrderedMap<std::string, int> m;

  // 없는 키에 대해 [] 사용 -> { key, 0 } 삽입 후 참조
  m["apple"] = 10;
  m["banana"] = 20;

  assert(m.size() == 2);
  assert(m.at("apple")  == 10);
  assert(m.at("banana") == 20);

  // 이미 존재하는 키에 대해 [] -> 덮어쓰기
  m["apple"] = 100;

  assert(m.at("apple") == 100);

  // 삽입 순서 체크: apple, banana
  const char* expected_keys[] = { "apple", "banana" };
  int         expected_vals[] = { 100, 20 };

  int idx = 0;
  for (auto it = m.begin(); it != m.end(); ++it, ++idx)
  {
    auto pair = *it;
    const std::string& k = pair.first;
    int&               v = pair.second;

    assert(k == expected_keys[idx]);
    assert(v == expected_vals[idx]);
  }
  assert(idx == 2);

  std::cout << "  OK\n";
  return true;
}

bool test_find_contains_erase()
{
  std::cout << "[TEST] find / contains / erase\n";

  Hash32OrderedMap<std::string, int> m;

  m.insert("apple",  1);
  m.insert("banana", 2);
  m.insert("cherry", 3);

  assert(m.contains("banana"));
  assert(!m.contains("orange"));

  {
    auto it = m.find("banana");
    assert(it != m.end());
    auto pair = *it;
    assert(pair.first == "banana");
    assert(pair.second == 2);
  }

  // erase 존재하는 키
  bool removed = m.erase("banana");
  assert(removed);
  assert(m.size() == 2);
  assert(!m.contains("banana"));

  // erase 없는 키
  removed = m.erase("banana");
  assert(!removed);
  assert(m.size() == 2);

  // 남은 순서: apple, cherry
  const char* expected_keys[] = { "apple", "cherry" };
  int         expected_vals[] = { 1, 3 };
  int idx = 0;
  for (auto it = m.begin(); it != m.end(); ++it, ++idx)
  {
    auto pair = *it;
    const std::string& k = pair.first;
    int&               v = pair.second;

    assert(k == expected_keys[idx]);
    assert(v == expected_vals[idx]);
  }
  assert(idx == 2);

  std::cout << "  OK\n";
  return true;
}

bool test_clear_and_reuse()
{
  std::cout << "[TEST] clear & reuse\n";

  Hash32OrderedMap<std::string, int> m;

  m.insert("apple",  1);
  m.insert("banana", 2);
  m.insert("cherry", 3);

  assert(m.size() == 3);

  m.clear();
  assert(m.size() == 0);
  assert(m.begin() == m.end());

  // 다시 사용 (pool 재사용 확인)
  m.insert("dog",   10);
  m.insert("eleph", 20);

  assert(m.size() == 2);
  assert(m.contains("dog"));
  assert(m.contains("eleph"));

  std::cout << "  OK\n";
  return true;
}

int main()
{
  bool ok = true;

  ok = ok && test_insert_and_order();
  ok = ok && test_insert_or_assign_and_order();
  ok = ok && test_operator_brackets();
  ok = ok && test_find_contains_erase();
  ok = ok && test_clear_and_reuse();

  if (ok)
  {
    std::cout << "\n=== ALL Hash32OrderedMap TESTS PASSED ===\n";
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
