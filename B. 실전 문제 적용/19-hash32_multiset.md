# hash32_multiset

- Hash32MultiSet은 한 줄로 말하면:
  - 같은 값을 여러 번 넣을 수 있는 해시 기반 멀티셋 (std::unordered_multiset 비슷한데, 내부는 intrusive Hash32Table +  
    Hash32PoolAllocator) 입니다.


## 1. Hash32MultiSet 구조와 특징 정리
템플릿 정의(우리가 만든 버전 기준):
```cpp
template<
    typename T,
    typename Hash  = DefaultHash32<T>,
    typename Equal = std::equal_to<T>,
    std::size_t PoolBlockSize = 256
>
class Hash32MultiSet;
```

- 내부 Node
```cpp
struct Node : public Hash32Item
{
  T value;

  explicit Node(const T& v) : value(v) {}

  template<typename... Args>
  explicit Node(std::in_place_t, Args&&... args)
    : value(std::forward<Args>(args)...)
  {}
};
```
- Hash32Item 상속 → Hash32Table에서 사용하는 intrusive 노드.
- 값 T 하나만 저장.
- 멀티셋이므로 동일한 값(T)을 여러 개 저장 가능.

### 핵심 멤버
```cpp
Hash32Table m_table;                   // 해시 테이블
std::size_t m_size;                    // 전체 원소 개수
Hash        m_hash;                    // 해시 함수
Equal       m_equal;                   // 동등 비교
Hash32PoolAllocator<Node, PoolBlockSize> m_pool; // 노드 메모리 풀
```

### 동작 특징
- 중복 값 허용
  - insert(10), insert(10) 여러 번 해도 각각 별개의 노드로 저장.
  - size()는 입력한 개수만큼 증가.

- 해시 기반 O(1) 평균 연산
  - 내부 구조는 Hash32Table → bucket + 체이닝
  - contains, find, insert, erase 모두 평균 O(1)에 가깝게 동작.
- 값은 곧 Key이므로 외부에는 const T&만 노출
  - set 계열이라 iterator를 const_iterator 타입으로만 제공.
  - *it으로 값을 읽을 수만 있고, 수정은 불가 (수정 시 해시 테이블 깨짐).

- equal_range(value) 제공
  - 특정 값과 동일한 값들 전체를 순회할 수 있도록 [first, last) 구간을 반환.
  - 내부 구현 특성상, first~last 구간에 다른 값이 섞여 들어갈 수 있으므로 실제 사용할 때는 if (*it == value) 식으로
    값 필터링해 주는 것이 안전하게 구현되어 있음.

- 메모리 풀 기반
  - Hash32PoolAllocator<Node> 사용 → Node를 블록 단위로 할당/해제.
  - insert 시 allocate()+placement new, erase/clear 시 destroy()로 소멸자 호출 + free list 재사용.
  - 멀티셋 특성상 원소 수가 많고 출입이 잦을 때 일반 new/delete보다 훨씬 효율적.

## 2. 주요 API 요약
- bool insert(const T& value)
- template<typename... Args> bool emplace(Args&&... args)
- bool contains(const T& value) const
- iterator find(const T& value) const
- std::pair<iterator, iterator> equal_range(const T& value) const
- std::size_t erase(const T& value)
  - value와 Equal인 모든 원소 삭제, 개수 반환
- void clear()
- iterator begin() const, iterator end() const  
  (해시 테이블 전체 순회 순서)

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

---

## 테스트 코드 예제

```cpp
#include <iostream>
#include <string>
#include <cassert>

#include "hash_set32.h"            // Hash32Item, Hash32Table, DefaultHash32
#include "hash32_pool_allocator.h" // Hash32PoolAllocator
#include "hash32_multiset.h"       // 우리가 만든 Hash32MultiSet

using util_hash::Hash32MultiSet;

bool test_insert_and_duplicates()
{
  std::cout << "[TEST] insert & duplicates\n";

  Hash32MultiSet<int> ms;

  bool ok1 = ms.insert(10);
  bool ok2 = ms.insert(10);
  bool ok3 = ms.insert(20);
  bool ok4 = ms.insert(10);

  assert(ok1 && ok2 && ok3 && ok4);
  assert(ms.size() == 4);

  // 최소한 contains가 제대로 동작하는지
  assert(ms.contains(10));
  assert(ms.contains(20));
  assert(!ms.contains(30));

  std::cout << "  OK\n";
  return true;
}

bool test_find_and_equal_range()
{
  std::cout << "[TEST] find & equal_range\n";

  Hash32MultiSet<int> ms;

  ms.insert(10);
  ms.insert(10);
  ms.insert(20);
  ms.insert(10);
  ms.insert(30);

  // find: 첫 매칭 (10)
  auto it = ms.find(10);
  assert(it != ms.end());
  int v = *it;
  assert(v == 10);

  // equal_range(10): 10인 것들 전부
  auto range = ms.equal_range(10);
  auto first = range.first;
  auto last  = range.second;

  int count10 = 0;
  for (auto it2 = first; it2 != last; ++it2)
  {
    int val = *it2;
    if (val == 10)  // 실제로는 이렇게 필터링하는 것이 안전
      ++count10;
  }
  // 10을 총 3번 넣었음
  assert(count10 == 3);

  std::cout << "  OK\n";
  return true;
}

bool test_erase_value()
{
  std::cout << "[TEST] erase(value)\n";

  Hash32MultiSet<std::string> ms;

  ms.insert("apple");
  ms.insert("apple");
  ms.insert("banana");
  ms.insert("apple");

  assert(ms.size() == 4);
  assert(ms.contains("apple"));

  // "apple" 전부 삭제
  std::size_t removed = ms.erase("apple");
  std::cout << "  removed apple count = " << removed << "\n";

  assert(removed == 3);
  assert(ms.size() == 1);
  assert(!ms.contains("apple"));
  assert(ms.contains("banana"));

  // 없는 값 삭제
  removed = ms.erase("orange");
  assert(removed == 0);
  assert(ms.size() == 1);

  std::cout << "  OK\n";
  return true;
}

bool test_clear_and_reuse()
{
  std::cout << "[TEST] clear & reuse\n";

  Hash32MultiSet<std::string> ms;

  ms.insert("a");
  ms.insert("a");
  ms.insert("b");
  assert(ms.size() == 3);

  ms.clear();
  assert(ms.size() == 0);
  assert(ms.begin() == ms.end());

  // 다시 사용 (pool 재사용)
  ms.insert("x");
  ms.insert("x");
  ms.insert("y");

  assert(ms.size() == 3);
  assert(ms.contains("x"));
  assert(ms.contains("y"));

  int count_x = 0;
  for (auto it = ms.begin(); it != ms.end(); ++it)
  {
    if (*it == std::string("x"))
      ++count_x;
  }
  assert(count_x == 2);

  std::cout << "  OK\n";
  return true;
}
```
```cpp
int main()
{
  bool ok = true;

  ok = ok && test_insert_and_duplicates();
  ok = ok && test_find_and_equal_range();
  ok = ok && test_erase_value();
  ok = ok && test_clear_and_reuse();

  if (ok)
  {
    std::cout << "\n=== ALL Hash32MultiSet TESTS PASSED ===\n";
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

