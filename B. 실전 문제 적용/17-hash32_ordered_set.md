# hash32_ordered_set

- Hash32OrderedSet은 한 줄로 요약하면:
  - 해시 기반인데, 삽입 순서를 그대로 기억하는 Set  
    (std::unordered_set + std::vector 느낌을 intrusive 방식으로 합쳐놓은 구조) 입니다.


## 1. Hash32OrderedSet 구조와 특징
템플릿 정의(앞에서 만든 버전 기준):
```cpp
template<
  typename T,
  typename Hash  = DefaultHash32<T>,
  typename Equal = std::equal_to<T>,
  std::size_t PoolBlockSize = 256
>
class Hash32OrderedSet;
```
- 내부 Node
```cpp
struct Node : public Hash32Item
{
  T value;

  Node* prev_order = nullptr;
  Node* next_order = nullptr;

  explicit Node(const T& v) : value(v) {}
  template<typename... Args>
  explicit Node(std::in_place_t, Args&&... args)
    : value(std::forward<Args>(args)...)
  {}
};
```
- Hash32Item 상속 → 해시테이블(Hash32Table)에 들어가는 intrusive 노드
- prev_order / next_order → 삽입 순서를 유지하기 위한 이중 연결 리스트 포인터

- 주요 멤버
```cpp
Hash32Table m_table;                   // 해시 테이블 (bucket + 체이닝)
std::size_t m_size;                    // 원소 개수
Hash        m_hash;                    // 해시 함수
Equal       m_equal;                   // 동등 비교

Node*       m_head = nullptr;          // 삽입 순서 리스트 head
Node*       m_tail = nullptr;          // 삽입 순서 리스트 tail

Hash32PoolAllocator<Node, PoolBlockSize> m_pool; // 노드 전용 메모리 풀
```

### 동작 특징

- 삽입 순서 유지
  - insert / emplace로 들어간 값은 처음 삽입된 순서대로 연결 리스트에 저장됩니다.
  - begin() ~ end() iterator는 이 연결 리스트를 따라 순회하므로, 항상 삽입 순서대로 나옵니다.

- Set 특성 (중복 없음)
  - 이미 존재하는 값에 대해 insert(value) → false 반환, Set이므로 중복은 저장되지 않습니다.
  - contains(value) / find(value) 로 존재 여부 확인.

- 해시 기반 O(1) 평균 조회
  - 내부는 Hash32Table(해시 테이블)이므로 평균적으로 contains, find, insert, erase 모두 O(1) 근처의 시간 복잡도를 가집니다.

- 값 수정 금지 (const iterator)
  - Set이기 때문에, 외부에는 const T&만 노출합니다.
  - 값이 곧 Key 역할을 하므로, 값을 바꾸면 해시 테이블이 깨지기 때문에 수정을 허용하지 않습니다.
  - 그래서 iterator를 using iterator = const_iterator; 형태로 제공했습니다.

- 메모리 풀 기반 관리
  - Hash32PoolAllocator<Node>를 사용하여 Node를 블록 단위로 할당/해제합니다.
  - insert → allocate() + placement new, erase/clear → destroy()를 통해 소멸자 + free list 재사용.
  - 해시 컨테이너 특성상 Node 삶이 컨테이너와 같이 움직이기 때문에, 일반 new/delete보다 훨씬 효율적입니다.

---

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

## 테스트 코드 예제

아래 코드는 Hash32OrderedSet의 기본 동작을 검증하는 단독 실행용 main입니다.

- 삽입 & 순서 유지
- 중복 삽입 무시
- contains / find / erase
- clear 후 재사용

```cpp
#include <iostream>
#include <string>
#include <cassert>

#include "hash_set32.h"            // Hash32Item, Hash32Table, DefaultHash32
#include "hash32_pool_allocator.h" // Hash32PoolAllocator
#include "hash32_ordered_set.h"    // 우리가 만든 Hash32OrderedSet

using util_hash::Hash32OrderedSet;

bool test_insert_and_order()
{
  std::cout << "[TEST] insert & order\n";

  Hash32OrderedSet<std::string> s;

  bool ok1 = s.insert("apple");
  bool ok2 = s.insert("banana");
  bool ok3 = s.insert("cherry");

  assert(ok1 && ok2 && ok3);
  assert(s.size() == 3);

  // 삽입 순서: "apple", "banana", "cherry"
  const char* expected[] = { "apple", "banana", "cherry" };

  int idx = 0;
  for (auto it = s.begin(); it != s.end(); ++it, ++idx)
  {
    const std::string& v = *it;
    assert(v == expected[idx]);
  }
  assert(idx == 3);

  std::cout << "  OK\n";
  return true;
}

bool test_no_duplicates()
{
  std::cout << "[TEST] no duplicates\n";

  Hash32OrderedSet<int> s;

  bool a1 = s.insert(10);
  bool a2 = s.insert(20);
  bool a3 = s.insert(10); // 중복

  assert(a1 && a2);
  assert(!a3); // 중복 삽입은 실패
  assert(s.size() == 2);

  // 순서: 10, 20
  int expected[] = { 10, 20 };
  int idx = 0;
  for (auto it = s.begin(); it != s.end(); ++it, ++idx)
  {
    int v = *it;
    assert(v == expected[idx]);
  }
  assert(idx == 2);

  std::cout << "  OK\n";
  return true;
}

bool test_contains_and_find_and_erase()
{
  std::cout << "[TEST] contains / find / erase\n";

  Hash32OrderedSet<std::string> s;

  s.insert("apple");
  s.insert("banana");
  s.insert("cherry");

  assert(s.size() == 3);
  assert(s.contains("banana"));
  assert(!s.contains("orange"));

  {
    auto it = s.find("banana");
    assert(it != s.end());
    const std::string& v = *it;
    assert(v == "banana");
  }

  // "banana" 삭제
  bool removed = s.erase("banana");
  assert(removed);
  assert(s.size() == 2);
  assert(!s.contains("banana"));

  // 남은 순서: apple, cherry
  const char* expected[] = { "apple", "cherry" };
  int idx = 0;
  for (auto it = s.begin(); it != s.end(); ++it, ++idx)
  {
    const std::string& v = *it;
    assert(v == expected[idx]);
  }
  assert(idx == 2);

  // 없는 값 삭제
  removed = s.erase("banana");
  assert(!removed);
  assert(s.size() == 2);

  std::cout << "  OK\n";
  return true;
}

bool test_clear_and_reuse()
{
  std::cout << "[TEST] clear & reuse\n";

  Hash32OrderedSet<std::string> s;

  s.insert("a");
  s.insert("b");
  s.insert("c");
  assert(s.size() == 3);

  s.clear();
  assert(s.size() == 0);
  assert(s.begin() == s.end());

  // 다시 사용 (내부 pool 재사용)
  s.insert("x");
  s.insert("y");

  assert(s.size() == 2);
  assert(s.contains("x"));
  assert(s.contains("y"));

  // 순서: x, y
  const char* expected[] = { "x", "y" };
  int idx = 0;
  for (auto it = s.begin(); it != s.end(); ++it, ++idx)
  {
    const std::string& v = *it;
    assert(v == expected[idx]);
  }
  assert(idx == 2);

  std::cout << "  OK\n";
  return true;
}

int main()
{
  bool ok = true;

  ok = ok && test_insert_and_order();
  ok = ok && test_no_duplicates();
  ok = ok && test_contains_and_find_and_erase();
  ok = ok && test_clear_and_reuse();

  if (ok)
  {
    std::cout << "\n=== ALL Hash32OrderedSet TESTS PASSED ===\n";
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

