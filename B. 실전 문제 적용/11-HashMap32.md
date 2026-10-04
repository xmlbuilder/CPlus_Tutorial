## 📘 HashMap 개념 및 특징 정리
### 📌 1. 개요
#### ✔ HashMap
- Key → Value의 매핑 구조 (연관 배열)
- 키는 유일해야 하고, 각각의 키는 하나의 값에 매핑됨
- 예: { "apple": 10, "banana": 20 }

- **해시 테이블(hash table)** 을 내부 구현으로 사용하며, O(1)에 가까운 평균 접근 시간을 갖는 것이 핵심이다.

### 📌 2. 내부 구조 비교
- 아래 표는 HashMap의 내부 구조 관점.
  - 저장되는 요소:	Value 단일값	Key + Value 쌍
  - Key 역할:	Key가 별도로 존재
  - 저장 노드:	Node(Key, Value) 형태
  - 중복 처리: 동일 키 중복 저장 불가, value는 변경 가능
  - 비교 방식:	hash(key) / equal(key1, key2)

### 📌 3. 동작 원리

- 다음 과정을 공유한다:
- 요소(또는 Key)에 대해 **해시 함수(hash)** 를 계산
- 테이블 크기(capacity) 모듈로써 bucket index 선택
- 해당 bucket의 체인(Linked List) 또는 Open Addressing 방식에서 요소 검색
- 요소가 없으면 삽입, 있으면 중복 처리
- 대부분 O(1) 평균 시간으로 처리되지만,
- 해시 충돌이 지나치게 많을 경우 최악의 경우 O(n)까지 늘어날 수 있다.

### 📌 4. HashMap 특징
#### ✔ 장점
- Key ↔ Value 구조로 접근성이 뛰어남
  - O(1) 평균 시간으로 찾고, 수정하고, 삽입 가능.
- set과 다르게 Value는 마음대로 변경 가능
 - Key만 변경하면 안 됨.
- 실제 데이터 모델링에 가장 자주 사용하는 데이터 구조

#### ✔ 단점
- 순서가 없음
- 해시 함수가 성능의 핵심
- Key 변경 금지
  - Key 수정 시 그 Key가 저장된 버킷 위치가 바뀌므로, 구조가 깨짐 → 원칙적으로 허용하면 안 됨.

### 📌 5. 해시 충돌 처리 방식
- HashMap은 두 가지 방식 중 하나를 사용한다.
- Hash 구조는 Separate Chaining 기반이며, intrusive 구조라서 Node 메모리 효율이 높다.

#### ✔ 1) Separate Chaining (체이닝)

- 같은 버킷에 여러 요소가 들어가면 연결 리스트로 연결
- 장점: 구현 간단, 확장 용이
- 단점: 메모리 증가

#### ✔ 2) Open Addressing (개방 주소법)

- 충돌 시 테이블 내 다른 위치를 탐색하여 저장
- 장점: 포인터가 없어 메모리 효율적
- 단점: 삭제 처리 어렵고, 클러스터링 문제 발생

### 📌 6. HashSet vs HashMap: 어떤 경우 사용?
- Key → Value 매핑 필요		✔
- Key 중복 허용 불가	✔	✔
- Value와 함께 부가 정보 필요		✔

### 📌 7. intrusive 방식 특징
- Hash32Map은 intrusive hash table이다.
#### intrusive 특징
- Node가 사용자 클래스에 직접 포함됨
- 외부에서 노드 메모리를 따로 할당하지 않음
- 메모리 단편화 및 할당/해제 오버헤드가 없음
- 속도가 매우 빠름
- 장점
  - 고성능, 메모리 효율적
  - CAD 엔진에서 수만~수백만 단위 데이터 처리에 적합
- 단점
  - 자료구조가 객체 내부에 결합되므로 다소 복잡
  - 키 변경 금지

### 📌 8. HashSet / HashMap의 성능적 중요성

- NURBS, BREP, Mesh 엔진에서는 다음 작업에 HashSet/HashMap이 필수적이다:
- ✔ 중복 Vertex/Edge 제거
  - (Topology 생성)
- ✔ Trim Curve 중복 관리 / 빠른 lookup
  - (surfaces / brep loops)
- ✔ Mesh face, edge indexing
- ✔ Curve / Surface intersection 시 seed 관리
- ✔ Boolean 연산 시 새로운 Edge/Vertex 병합
- 이런 작업에서 O(1) 평균 시간 lookup은 큰 성능 차이를 만든다.

### 📌 9. 정리표
- 저장 구조:	Value pair
- Key: 별도 제공
- 중복:	Key 중복 X
- Value: 	Value는 변경 가능, Key는 금지
- 주 용도:	fast lookup	연관 배열, 데이터 맵핑
- 내부 노드:	Node(Key, Value)
- intrusive 지원:가능
- 평균 시간: O(1)

### 📌 10. 결론
- HashMap은 키 → 값 구조
- CAD 엔진이나 OpenNURBS처럼 대규모 데이터 처리에서는 intrusive hash 구조가 메모리 효율 및 성능 면에서 매우 유리

---
### 📌 소스 코드
```cpp
#pragma once

#include "hash_set32.h"  // Hash32Item / Hash32Table / DefaultHash32
#include <functional>
#include <iterator>
#include <cstdint>
#include <cstddef>
#include <utility>

namespace util_hash
{
  /// Hash32Map<Key, T>
  /// - 내부는 Hash32Table + intrusive Node
  /// - 키는 고정(변경 금지), 값은 변경 가능
  template<
    typename Key,
    typename T,
    typename Hash  = DefaultHash32<Key>,
    typename Equal = std::equal_to<Key>
  >
  class Hash32Map
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
        construct_value(std::move(vtuple),
                        std::index_sequence_for<Args...>{});
      }

      template<typename... Args, std::size_t... Is>
      void construct_value(std::tuple<Args...>&& t, std::index_sequence<Is...>)
      {
        new (&value) T(std::forward<Args>(std::get<Is>(t))...);
      }
    };

    Hash32Table m_table;
    std::size_t m_size  = 0;
    Hash        m_hash;
    Equal       m_equal;

    // iterator에서 반환할 pair-like reference
    struct ValueRef
    {
      const Key& first;
      T&         second;
    };

  public:
    class const_iterator;

    class iterator
    {
      using Table = Hash32Table;
      using Item  = Hash32Item;

      Table* m_table = nullptr;
      Item*  m_item  = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = ValueRef*;   // 실제 포인터는 아니고, 사용 시 주의
      using reference         = ValueRef;

      iterator() = default;

      iterator(Table* table, Item* item)
        : m_table(table), m_item(item)
      {
      }

      reference operator*() const
      {
        Node* n = static_cast<Node*>(m_item);
        return ValueRef{ n->key, n->value };
      }

      // pointer 연산은 약간 애매해서 제공하지 않는 편이 안전하지만,
      // 최소한의 호환성을 위해 주소를 못 주는 fake pointer 타입 사용도 가능.
      // 여기서는 map-like range-for 사용만 가정하고 pointer는 사용하지 않도록.
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
    };

    class const_iterator
    {
      using Table = Hash32Table;
      using Item  = Hash32Item;

      const Table* m_table = nullptr;
      Item*        m_item  = nullptr;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = ValueRef;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const ValueRef*; // 마찬가지로 pointer는 쓰지 않는게 안전
      using reference         = ValueRef;

      const_iterator() = default;

      const_iterator(const Table* table, Item* item)
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
          m_item = const_cast<Table*>(m_table)->nextTableItem(m_item);
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
    Hash32Map() = default;

    explicit Hash32Map(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32Map()
    {
      clear();
    }

    Hash32Map(const Hash32Map&)            = delete;
    Hash32Map& operator=(const Hash32Map&) = delete;

    // ----- 기본 프로퍼티 -----
    bool empty() const noexcept { return m_size == 0; }
    std::size_t size() const noexcept { return m_size; }

    // ----- iterator -----
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

  public:
    // ----- 조회 -----
    bool contains(const Key& key) const
    {
      return find_node(key) != nullptr;
    }

    iterator find(const Key& key)
    {
      Node* n = find_node(key);
      if (!n)
        return end();

      return iterator(&m_table, static_cast<Hash32Item*>(n));
    }

    const_iterator find(const Key& key) const
    {
      Node* n = find_node(key);
      if (!n)
        return end();

      return const_iterator(&m_table,
                            static_cast<Hash32Item*>(n));
    }

    // ----- at / operator[] -----
    T& at(const Key& key)
    {
      Node* n = find_node(key);
      if (!n)
        throw std::out_of_range("Hash32Map::at: key not found");
      return n->value;
    }

    const T& at(const Key& key) const
    {
      Node* n = find_node(key);
      if (!n)
        throw std::out_of_range("Hash32Map::at: key not found");
      return n->value;
    }

    // 존재하지 않으면 default-constructed 값으로 삽입 후 참조 반환
    T& operator[](const Key& key)
    {
      Node* n = find_node(key);
      if (n)
        return n->value;

      std::uint32_t h = make_hash32(key);
      Node* node = new Node(key, T{});

      if (!m_table.addItem(h, node))
      {
        delete node;
        throw std::bad_alloc();
      }

      ++m_size;
      return node->value;
    }

    // ----- 삽입 -----
    // 이미 존재하면 삽입 실패 (false)
    bool insert(const Key& key, const T& value)
    {
      if (find_node(key))
        return false;

      std::uint32_t h = make_hash32(key);
      Node* node = new Node(key, value);

      if (!m_table.addItem(h, node))
      {
        delete node;
        return false;
      }

      ++m_size;
      return true;
    }

    // 존재하면 값 덮어쓰기, 존재 여부 반환 (true: 새로 삽입, false: 덮어쓰기)
    bool insert_or_assign(const Key& key, const T& value)
    {
      Node* n = find_node(key);
      if (n)
      {
        n->value = value;
        return false; // 이미 있던 키
      }

      std::uint32_t h = make_hash32(key);
      Node* node = new Node(key, value);

      if (!m_table.addItem(h, node))
      {
        delete node;
        throw std::bad_alloc();
      }

      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(const Key& key, Args&&... args)
    {
      if (find_node(key))
        return false;

      std::uint32_t h = make_hash32(key);
      Node* node = new Node(std::piecewise_construct,
                            std::make_tuple(key),
                            std::make_tuple(std::forward<Args>(args)...));

      if (!m_table.addItem(h, node))
      {
        delete node;
        return false;
      }

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

      delete node;
      --m_size;
      return true;
    }

    void clear()
    {
      // 노드 delete
      for (Hash32Item* it = m_table.firstTableItem();
           it;
           )
      {
        Hash32Item* next = m_table.nextTableItem(it);
        Node*       node = static_cast<Node*>(it);
        delete node;
        it = next;
      }

      m_table.removeAllItems();
      m_size = 0;
    }
  };

} // namespace util_hash
```

### 📌  테스트 코드
```cpp
#include "hash_set32.h"
#include "hash32_map.h"
#include <string>
#include <iostream>

int main()
{
  util_hash::Hash32Map<std::string, int> m;

  m.insert("apple",  10);
  m.insert("banana", 20);
  m.insert_or_assign("banana", 30); // 값 덮어쓰기

  m["orange"] = 40; // 없으면 { "orange", 0 } 삽입 후 40 대입

  if (m.contains("banana"))
    std::cout << "banana = " << m.at("banana") << "\n";

  for (auto it = m.begin(); it != m.end(); ++it)
  {
    auto [k, v] = *it;   // ValueRef: const Key& first, T& second
    std::cout << k << " -> " << v << "\n";
  }

  m.erase("apple");

  std::cout << "size = " << m.size() << "\n";

  return 0;
}
```
---


