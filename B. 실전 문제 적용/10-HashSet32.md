# HashSet32

## 📌 메모리 사용 관점
- 원본(OpenNurbs)
- 직접 onmalloc/onfree로 버킷 배열 관리
- 아이템은 외부에서 생성·소멸 책임 → 테이블은 포인터만 관리
- 메모리 풀(ON_FixedSizePool)과 연동 가능 → CAD처럼 대량 객체 관리에 최적화

## 소스 코드
### 1. hash_set32.h
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
### 2. hash_set32.cpp
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

### 3. 메인 코드
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
