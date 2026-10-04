## 📘 HashSet32

### 📌 HashSet 개념 및 특징 정리
#### 🔹 1. 개요
##### ✔ HashSet
- 중복 없는 데이터 집합(Set) 자료구조
- 저장되는 것은 값(value)만 값 자체가 키(key) 역할을 하며, 동일성 판단은 hash(value) + equal(value1, value2)로 수행
- 예: { 10, 20, 30 }

#### 🔹 2. 내부 구조 비교
- 저장되는 요소	Value 단일값
- Key 역할	Value 자체가 Key
- 저장 노드	Node(Value) 형태
- 중복 처리	동일 값 저장 불가
- 비교 방식	hash(value)

#### 🔹 3. 동작 원리
- 요소(또는 Key)에 대해 **해시 함수(hash)** 를 계산
- 테이블 크기(capacity) 모듈로써 bucket index 선택
- 해당 bucket의 체인(Linked List) 또는 Open Addressing 방식에서 요소 검색
- 요소가 없으면 삽입, 있으면 중복 처리
- 대부분 O(1) 평균 시간으로 처리되지만,
- 해시 충돌이 지나치게 많을 경우 최악의 경우 O(n)까지 늘어날 수 있다.

#### 🔹 4. HashSet 특징
##### ✔ 장점
- 중복 자동 제거
- 많은 데이터에서 unique 처리에 매우 빠름.
- 검색 속도 빠름
- 평균 O(1)으로 존재 여부 체크 가능.
- 메모리 절약 (intrusive 구조일 경우)
- Node를 객체 내부에 넣는 intrusive 방식에서는 추가 노드 할당이 필요 없음.
##### ✔ 단점
- 정렬된 순서가 없음
- 항상 삽입 순서도 유지되지 않는다.
- 해시 충돌이 많으면 성능 저하
- 잘못된 해시 함수 사용 시 오래 걸릴 수 있음.
- Value = Key라서 키 값을 변경하는 구조를 허용하면 안 됨  
  - Key를 변경하면 해시 위치가 달라져 테이블이 깨짐

#### 🔹 5. 해시 충돌 처리 방식

- HashSet 은 두 가지 방식 중 하나를 사용한다.
##### ✔ 1) Separate Chaining (체이닝)
- 같은 버킷에 여러 요소가 들어가면 연결 리스트로 연결
- 장점: 구현 간단, 확장 용이
- 단점: 메모리 증가

##### ✔ 2) Open Addressing (개방 주소법)
- 충돌 시 테이블 내 다른 위치를 탐색하여 저장
- 장점: 포인터가 없어 메모리 효율적
- 단점: 삭제 처리 어렵고, 클러스터링 문제 발생
  
- Hash 구조는 Separate Chaining 기반이며, intrusive 구조라서 Node 메모리 효율이 높다.

#### 🔹 6. intrusive 특징
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

#### 🔹 7. HashSet성능적 중요성
- NURBS, BRep, Mesh 엔진에서는 다음 작업에 HashSet이 필수적:
- ✔ 중복 Vertex/Edge 제거
  - (Topology 생성)
- ✔ Trim Curve 중복 관리 / 빠른 lookup
  - (surfaces / brep loops)
- ✔ Mesh face, edge indexing
- ✔ Curve / Surface intersection 시 seed 관리
- ✔ Boolean 연산 시 새로운 Edge/Vertex 병합
- 이런 작업에서 O(1) 평균 시간 lookup은 큰 성능 차이를 만든다.

---

### 📌 소스 코드
#### 🔹 1. hash_set32.h
```cpp
#pragma once

#include <cstdint>
#include <cstddef>

namespace util_hash
{
  /// 32비트 해시를 사용하는 intrusive hash set용 기본 노드
  class Hash32Item
  {
  public:
    Hash32Item() = default;
    ~Hash32Item() = default;

    Hash32Item(const Hash32Item&) = default;
    Hash32Item& operator=(const Hash32Item&) = default;

    /// 이 아이템이 속한 테이블의 시리얼 번호 (없으면 0)
    std::uint32_t tableSerialNumber() const noexcept { return m_table_sn; }

    /// 테이블이 이 아이템에 대해 사용하는 32비트 해시값
    std::uint32_t itemHash() const noexcept { return m_hash32; }

    /// 복사한 아이템을 새 테이블에 넣기 전에 참조를 끊고 싶을 때 사용
    void clearTableSerialNumber() noexcept { m_table_sn = 0; }

  private:
    friend class Hash32Table;

    mutable Hash32Item*  m_next     = nullptr;
    mutable std::uint32_t m_hash32  = 0;
    mutable std::uint32_t m_table_sn = 0;
  };

  /// 32비트 해시를 키로 사용하는 intrusive hash set
  class Hash32Table
  {
  public:
    Hash32Table();
    ~Hash32Table();

    Hash32Table(const Hash32Table&)            = delete;
    Hash32Table& operator=(const Hash32Table&) = delete;

    /// 이 테이블의 고유 시리얼 번호
    std::uint32_t tableSerialNumber() const noexcept { return m_table_sn; }

    /// 아이템 추가 (성공 시 true)
    bool addItem(std::uint32_t hash32, Hash32Item* item);

    /// 아이템 제거 (성공 시 true)
    bool removeItem(Hash32Item* item);

    /// 모든 아이템 제거 (아이템 메모리는 사용자가 해제)
    std::uint32_t removeAllItems();

    /// 전체 아이템 수
    std::uint32_t itemCount() const noexcept { return m_item_count; }

    /// 주어진 해시값을 가진 첫 아이템
    Hash32Item* firstItemWithHash(std::uint32_t hash32) const;

    /// 같은 해시값을 가진 다음 아이템 (체인 내)
    Hash32Item* nextItemWithHash(const Hash32Item* current) const;

    /// 테이블 전체에서 첫 아이템 (순회용)
    Hash32Item* firstTableItem() const;

    /// 테이블 전체에서 다음 아이템 (순회용)
    Hash32Item* nextTableItem(const Hash32Item* item) const;

    /// 디버그용: 내부 구조가 일관적인지 검사 (OK면 true)
    bool isValid() const;

  private:
    static std::uint32_t newSerialNumber();

    void adjustCapacity(std::uint32_t desired_item_count);

  private:
    const std::uint32_t m_table_sn;          ///< 고유 시리얼
    std::uint32_t       m_reserved  = 0;
    std::uint32_t       m_capacity  = 0;     ///< 버킷 개수
    std::uint32_t       m_item_count = 0;    ///< 아이템 수
    Hash32Item**        m_buckets   = nullptr;
  };

} // namespace util_hash
```

#### 🔹 2. hash_set32.cpp
```cpp
#include "hash_set32.h"

#include <cstdlib>  // std::malloc, std::free
#include <cstring>  // std::memset

namespace util_hash
{
  // ===============================
  //  Hash32Table - 구현부
  // ===============================

  std::uint32_t Hash32Table::newSerialNumber()
  {
    static std::uint32_t sn = 0;
    if (sn == 0)
      sn = 1;          // 0은 "테이블에 없음"으로 예약
    return sn++;
  }

  Hash32Table::Hash32Table()
    : m_table_sn(newSerialNumber())
  {
  }

  Hash32Table::~Hash32Table()
  {
    if (m_buckets)
      std::free(m_buckets);
  }

  void Hash32Table::adjustCapacity(std::uint32_t desired_item_count)
  {
    const std::uint32_t max_capacity        = 256u * 1024u;
    const std::uint32_t target_list_length  = 8u;

    if (m_capacity >= max_capacity)
      return;

    // 평균 체인 길이가 target_list_length 를 넘기 시작하면 확장
    if (m_capacity == 0 || desired_item_count / target_list_length >= m_capacity)
    {
      std::uint32_t new_capacity = m_capacity;
      if (new_capacity < 64u)
        new_capacity = 64u;

      while (new_capacity < max_capacity &&
             desired_item_count / target_list_length > new_capacity)
      {
        new_capacity *= 2u;
      }

      // 새 버킷 배열 할당
      const std::size_t bytes = static_cast<std::size_t>(new_capacity) * sizeof(m_buckets[0]);
      auto** new_buckets = static_cast<Hash32Item**>(std::malloc(bytes));
      if (!new_buckets)
        return; // 메모리 부족이면 기존 상태 유지

      std::memset(new_buckets, 0, bytes);

      // 기존 아이템 재해싱
      if (m_item_count > 0 && m_buckets)
      {
        for (std::uint32_t i = 0; i < m_capacity; ++i)
        {
          Hash32Item* item = m_buckets[i];
          while (item)
          {
            Hash32Item* next = item->m_next;
            const std::uint32_t j = item->m_hash32 % new_capacity;
            item->m_next = new_buckets[j];
            new_buckets[j] = item;
            item = next;
          }
        }
        std::free(m_buckets);
      }

      m_buckets  = new_buckets;
      m_capacity = new_capacity;
    }
  }

  bool Hash32Table::addItem(std::uint32_t hash32, Hash32Item* item)
  {
    if (!item)
      return false;

    // 이미 다른 테이블에 들어가 있는 아이템이면 안 됨
    if (item->m_table_sn != 0)
      return false;

    item->m_table_sn = m_table_sn;
    item->m_hash32   = hash32;

    adjustCapacity(m_item_count + 1);

    if (m_capacity == 0)
      adjustCapacity(1); // 방어적 코드

    const std::uint32_t bucket_index = (m_capacity > 0)
                                     ? (hash32 % m_capacity)
                                     : 0u;

    item->m_next = m_buckets ? m_buckets[bucket_index] : nullptr;
    if (m_buckets)
      m_buckets[bucket_index] = item;
    ++m_item_count;

    return true;
  }

  bool Hash32Table::removeItem(Hash32Item* item)
  {
    if (!item)
      return false;

    if (item->m_table_sn != m_table_sn || m_item_count == 0 || m_capacity == 0 || !m_buckets)
      return false;

    const std::uint32_t bucket_index = item->m_hash32 % m_capacity;
    Hash32Item* prev = nullptr;
    for (Hash32Item* p = m_buckets[bucket_index]; p; p = p->m_next)
    {
      if (p == item)
      {
        if (!prev)
          m_buckets[bucket_index] = p->m_next;
        else
          prev->m_next = p->m_next;

        --m_item_count;
        item->m_table_sn = 0;
        item->m_hash32   = 0;
        item->m_next     = nullptr;
        return true;
      }
      prev = p;
    }

    // 못 찾음
    return false;
  }

  std::uint32_t Hash32Table::removeAllItems()
  {
    const std::uint32_t removed = m_item_count;
    if (m_buckets && m_capacity > 0)
    {
      std::memset(m_buckets, 0,
                  static_cast<std::size_t>(m_capacity) * sizeof(m_buckets[0]));
    }
    m_item_count = 0;
    // 아이템 내부의 m_table_sn / m_hash32 / m_next 는 사용자가 직접 초기화하거나
    // 필요하면 순회하면서 0으로 세팅해도 됩니다.
    return removed;
  }

  Hash32Item* Hash32Table::firstItemWithHash(std::uint32_t hash32) const
  {
    if (m_capacity == 0 || !m_buckets || m_item_count == 0)
      return nullptr;

    const std::uint32_t bucket_index = hash32 % m_capacity;
    for (Hash32Item* item = m_buckets[bucket_index]; item; item = item->m_next)
    {
      if (item->m_hash32 == hash32 && item->m_table_sn == m_table_sn)
        return item;
    }
    return nullptr;
  }

  Hash32Item* Hash32Table::nextItemWithHash(const Hash32Item* current) const
  {
    if (!current || current->m_table_sn != m_table_sn)
      return nullptr;

    const std::uint32_t hash32 = current->m_hash32;

    for (Hash32Item* item = current->m_next; item; item = item->m_next)
    {
      if (item->m_table_sn != m_table_sn)
        break;
      if (item->m_hash32 == hash32)
        return item;
    }
    return nullptr;
  }

  Hash32Item* Hash32Table::firstTableItem() const
  {
    if (m_item_count == 0 || m_capacity == 0 || !m_buckets)
      return nullptr;

    for (std::uint32_t i = 0; i < m_capacity; ++i)
    {
      if (m_buckets[i])
        return m_buckets[i];
    }
    return nullptr;
  }

  Hash32Item* Hash32Table::nextTableItem(const Hash32Item* item) const
  {
    if (!item || item->m_table_sn != m_table_sn ||
        m_item_count == 0 || m_capacity == 0 || !m_buckets)
      return nullptr;

    // 같은 체인에서 다음
    if (item->m_next)
      return item->m_next;

    // 다음 버킷에서 첫 아이템 찾기
    const std::uint32_t start_bucket = (m_capacity > 0)
                                     ? (item->m_hash32 % m_capacity) + 1u
                                     : 0u;
    for (std::uint32_t i = start_bucket; i < m_capacity; ++i)
    {
      if (m_buckets[i])
        return m_buckets[i];
    }
    return nullptr;
  }

  bool Hash32Table::isValid() const
  {
    if (m_table_sn == 0)
      return false;

    if (m_capacity == 0)
    {
      if (m_buckets != nullptr)
        return false;
    }
    else
    {
      if (m_buckets == nullptr)
        return false;
    }

    std::uint32_t counted = 0;
    if (m_buckets && m_capacity > 0)
    {
      for (std::uint32_t i = 0; i < m_capacity; ++i)
      {
        for (const Hash32Item* item = m_buckets[i]; item; item = item->m_next)
        {
          if (item->m_table_sn != m_table_sn)
            return false;
          const std::uint32_t j = item->m_hash32 % m_capacity;
          if (j != i)
            return false;
          ++counted;
        }
      }
    }

    return (counted == m_item_count);
  }

} // namespace util_hash
```

#### 🔹 3. 메인 코드
```cpp
#include "hash_set32.h"
#include <string>
#include <functional>

// 사용자가 저장하고 싶은 구조체가 Hash32Item 상속
struct StringKeyItem : public util_hash::Hash32Item
{
  std::string key;
  int         value;
};

int main()
{
  util_hash::Hash32Table table;

  StringKeyItem a{ {}, "apple",  1 };
  StringKeyItem b{ {}, "banana", 2 };
  StringKeyItem c{ {}, "apple",  3 }; // 같은 key라도 hash32로만 판단하면 충돌 가능

  auto hash32 = [](const std::string& s) -> std::uint32_t
  {
    // 예시용: std::hash 결과를 32bit로 슬라이스
    return static_cast<std::uint32_t>(std::hash<std::string>{}(s));
  };

  table.addItem(hash32(a.key), &a);
  table.addItem(hash32(b.key), &b);
  table.addItem(hash32(c.key), &c); // 같은 bucket/같은 hash일 수도 있음

  // "apple" 찾기 (충돌 대비해서 key 비교)
  std::uint32_t h = hash32("apple");
  for (auto* it = static_cast<StringKeyItem*>(table.firstItemWithHash(h));
       it;
       it = static_cast<StringKeyItem*>(table.nextItemWithHash(it)))
  {
    if (it->key == "apple")
    {
      // 여기서 it 가 "apple"인 아이템
    }
  }

  // 전체 순회
  for (auto* it = static_cast<StringKeyItem*>(table.firstTableItem());
       it;
       it = static_cast<StringKeyItem*>(table.nextTableItem(it)))
  {
    // it->key, it->value 사용
  }

  return 0;
}
```
---

### 📌 Template 버전
#### 🔹 1. hash_set32_template.h
```cpp
#pragma once

#include "hash_set32.h"  // 앞에서 만든 Hash32Item / Hash32Table 정의
#include <functional>
#include <iterator>
#include <cstdint>
#include <cstddef>
#include <utility>

namespace util_hash
{
  /// 기본 32bit 해시:
  /// std::hash<T>의 결과를 32bit로 잘라서 사용
  template<typename T>
  struct DefaultHash32
  {
    std::uint32_t operator()(const T& v) const
    {
      using H = std::hash<T>;
      return static_cast<std::uint32_t>(H{}(v));
    }
  };

  /// T값을 보관하는 non-intrusive hash set 래퍼
  /// - 내부적으로 Hash32Table + intrusive 노드(Node) 사용
  /// - 인터페이스는 unordered_set<T>와 비슷하게 구성 (간단 버전)
  template<
    typename T,
    typename Hash  = DefaultHash32<T>,
    typename Equal = std::equal_to<T>
  >
  class Hash32Set
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

    Hash32Table m_table;
    std::size_t m_size  = 0;
    Hash        m_hash;
    Equal       m_equal;

  public:
    // ----- iterator: set 특성상 const T&만 제공 -----
    class const_iterator
    {
      using Table = Hash32Table;
      using Item  = Hash32Item;

      const Table* m_table = nullptr;
      Item*        m_item  = nullptr; // non-const Node 접근은 금지 (set 특성상 수정하면 안 됨)

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = T;
      using difference_type   = std::ptrdiff_t;
      using pointer           = const T*;
      using reference         = const T&;

      const_iterator() = default;

      const_iterator(const Table* table, Item* item)
        : m_table(table), m_item(item)
      {
      }

      reference operator*() const
      {
        const Node* n = static_cast<const Node*>(m_item);
        return n->value;
      }

      pointer operator->() const
      {
        const Node* n = static_cast<const Node*>(m_item);
        return &n->value;
      }

      const_iterator& operator++()
      {
        if (m_table && m_item)
          m_item = const_cast<Item*>(m_table->nextTableItem(m_item));
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

    using iterator = const_iterator;

    // ----- 생성 / 소멸 -----
    Hash32Set() = default;

    explicit Hash32Set(const Hash& h, const Equal& eq = Equal())
      : m_hash(h), m_equal(eq)
    {
    }

    ~Hash32Set()
    {
      clear();
    }

    Hash32Set(const Hash32Set&)            = delete;
    Hash32Set& operator=(const Hash32Set&) = delete;

    // ----- 기본 프로퍼티 -----
    bool empty() const noexcept { return m_size == 0; }
    std::size_t size() const noexcept { return m_size; }

    // ----- 반복자 -----
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

    // ----- 내부 유틸 -----
  private:
    std::uint32_t make_hash32(const T& value) const
    {
      std::uint32_t h = m_hash(value);
      // 해시값이 0이어도 문제는 없지만, 0을 특별히 피하고 싶다면 여기서 조정 가능
      return h;
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

  public:
    // ----- 조회 -----
    bool contains(const T& value) const
    {
      return find_node(value) != nullptr;
    }

    const_iterator find(const T& value) const
    {
      Node* n = find_node(value);
      if (!n)
        return end();

      return const_iterator(&m_table,
                            static_cast<Hash32Item*>(n));
    }

    // ----- 삽입 -----
    // 이미 있는 값이면 삽입 실패(false) / 포기
    bool insert(const T& value)
    {
      if (find_node(value))
        return false; // set 특성: 중복 허용 안 함

      std::uint32_t h = make_hash32(value);
      Node* node = new Node(value);

      if (!m_table.addItem(h, node))
      {
        delete node;
        return false;
      }

      ++m_size;
      return true;
    }

    template<typename... Args>
    bool emplace(Args&&... args)
    {
      // emplace는 key가 args로부터 만들어지므로,
      // 중복 체크를 하려면 일단 임시 T를 만들어야 함
      // (성능이 크게 중요하지 않다면 이렇게 단순하게 가도 무방)
      T tmp(std::forward<Args>(args)...);
      if (find_node(tmp))
        return false;

      std::uint32_t h = make_hash32(tmp);
      Node* node = new Node(std::in_place, std::forward<Args>(args)...);

      if (!m_table.addItem(h, node))
      {
        delete node;
        return false;
      }

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

      delete node;
      --m_size;
      return true;
    }

    void clear()
    {
      // 먼저 모든 노드 delete
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

#### 🔹 테스트 코드
```cpp
#include <iostream>
#include "hash_set32_template.h"
int main()
{

    util_hash::Hash32Set<int> int_set;
    int_set.insert(10);
    int_set.insert(20);
    int_set.insert(10); 

    if (int_set.contains(20))
        std::cout << "20 is in the set\n";

    for (auto&& v : int_set)
        std::cout << "int_set: " << v << "\n";


    util_hash::Hash32Set<std::string> str_set;
    str_set.emplace("apple");
    str_set.emplace("banana");
    str_set.emplace("apple"); // 중복 무시

    if (auto it = str_set.find("banana"); it != str_set.end())
        std::cout << "found: " << *it << "\n";

    str_set.erase("banana");

    for (auto&& s : str_set)
        std::cout << "str_set: " << s << "\n";

    return 0;
}
```

---

