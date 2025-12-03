## 📌 메모리 사용 관점
- 원본(OpenNurbs)
- 직접 onmalloc/onfree로 버킷 배열 관리
- 아이템은 외부에서 생성·소멸 책임 → 테이블은 포인터만 관리
- 메모리 풀(ON_FixedSizePool)과 연동 가능 → CAD처럼 대량 객체 관리에 최적화
- 재구성 버전
- std::vector<std::list<>>로 버킷 관리 → 자동 메모리 관리(RAII)
- 아이템은 std::list 안에 직접 저장 → 별도 포인터 관리 불필요
- 필요 시 capacity 자동 확장 → 평균 버킷 길이 유지
- 메모리 누수 위험 없음, 코드 단순화

👉 즉, 메모리 효율성 + 안전성을 동시에 확보했습니다.

## 📌 만든 취지의 강점 유지
- 체인 방식 충돌 처리: 해시 충돌 시 연결 리스트로 관리 → 원본과 동일
- 자동 리사이즈: 아이템 수 증가 시 capacity 확장 → 평균 버킷 길이 8 이하 유지 (원본 취지 반영)
- 고품질 해시값 전제: 여전히 32비트 해시 기반으로 설계 → UUID, SHA1 등과 잘 맞음
- 순회 기능: firstItemWithHash, 전체 순회 가능 → 원본과 동일한 사용 패턴 제공

## 📌 차별점 (현대화된 강점)
- RAII 기반 메모리 관리 → 메모리 누수 위험 제거
- 템플릿 기반 → 어떤 타입에도 적용 가능 (원본은 특정 클래스 전용)
- 간결한 코드 → 복잡한 내부 포인터 관리 제거, 유지보수 용이

- ✅ 결론: 네, 메모리 사용과 원래 만든 취지(충돌 처리 단순화, 평균 버킷 길이 유지, 고품질 해시 기반 탐색)는 그대로 살렸습니다.  
  대신 현대 C++ 스타일로 안전성과 범용성을 강화했기 때문에, CAD/시뮬레이션 같은 대량 데이터 처리뿐 아니라 일반 프로젝트에서도 바로 쓸 수 있습니다.

---

## 소스 코드
```cpp
#include <vector>
#include <list>
#include <functional>
#include <numeric>
#include <cmath>

// -------------------- HashTableItem --------------------
template <typename T>
class HashTableItem {
public:
    HashTableItem(const T& value, uint32_t hash)
        : value(value), hash32(hash) {}

    uint32_t getHash() const { return hash32; }
    const T& getValue() const { return value; }

private:
    T value;
    uint32_t hash32;
};

// -------------------- HashTable --------------------
template <typename T>
class HashTable {
public:
    explicit HashTable(size_t initial_capacity = 64)
        : capacity(initial_capacity), item_count(0) {
        table.resize(capacity);
    }

    bool addItem(uint32_t hash32, const T& value) {
        resizeIfNeeded();
        size_t idx = hash32 % capacity;
        table[idx].emplace_back(value, hash32);
        ++item_count;
        return true;
    }

    bool removeItem(uint32_t hash32, const T& value) {
        size_t idx = hash32 % capacity;
        auto& bucket = table[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->getHash() == hash32 && it->getValue() == value) {
                bucket.erase(it);
                --item_count;
                return true;
            }
        }
        return false;
    }

    const T* firstItemWithHash(uint32_t hash32) const {
        size_t idx = hash32 % capacity;
        for (const auto& item : table[idx]) {
            if (item.getHash() == hash32)
                return &item.getValue();
        }
        return nullptr;
    }

    size_t itemCount() const { return item_count; }

    void clear() {
        for (auto& bucket : table) bucket.clear();
        item_count = 0;
    }

private:
    void resizeIfNeeded() {
        const size_t target_list_length = 8;
        if (item_count / target_list_length >= capacity) {
            capacity *= 2;
            std::vector<std::list<HashTableItem<T>>> new_table(capacity);
            for (auto& bucket : table) {
                for (auto& item : bucket) {
                    size_t idx = item.getHash() % capacity;
                    new_table[idx].push_back(item);
                }
            }
            table.swap(new_table);
        }
    }

    size_t capacity;
    size_t item_count;
    std::vector<std::list<HashTableItem<T>>> table;
};

// -------------------- Utility: Hash function for vector<double> --------------------
uint32_t hashVector(const std::vector<double>& vec) {
    double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
    uint32_t h = static_cast<uint32_t>(std::fabs(sum) * 1000) ^ static_cast<uint32_t>(vec.size());
    return h;
}

```

## 메인 코드
```cpp
#include <iostream>
#include "hash_table.h"

int main() {
    HashTable<std::vector<double>> accelTable;

    std::vector<double> accel1 = {0.1, 0.2, 0.3, 0.4};
    std::vector<double> accel2 = {1.0, 2.0, 3.0, 4.0};
    std::vector<double> accel3 = {0.1, 0.2, 0.3, 0.4}; // accel1과 동일

    uint32_t h1 = hashVector(accel1);
    uint32_t h2 = hashVector(accel2);
    uint32_t h3 = hashVector(accel3);

    accelTable.addItem(h1, accel1);
    accelTable.addItem(h2, accel2);

    std::cout << "Item count: " << accelTable.itemCount() << "\n";
    const auto* found = accelTable.firstItemWithHash(h1);
    if (found) {
        std::cout << "First item with hash " << h1 << ": ";
        for (double v : *found) std::cout << v << " ";
        std::cout << "\n";
    }

    bool removed = accelTable.removeItem(h2, accel2);
    std::cout << "Removed accel2? " << (removed ? "Yes" : "No") << "\n";
    std::cout << "Item count after removal: " << accelTable.itemCount() << "\n";

    const auto* dup = accelTable.firstItemWithHash(h3);
    if (dup) {
        std::cout << "Duplicate accel3 found: ";
        for (double v : *dup) std::cout << v << " ";
        std::cout << "\n";
    } else {
        accelTable.addItem(h3, accel3);
    }
    std::cout << "Final item count: " << accelTable.itemCount() << "\n";
    {
        HashTable<std::string> htable;
        htable.addItem(1234, std::string("Hello"));
        htable.addItem(5678, std::string("World"));
        htable.addItem(1234, std::string("Collision"));

        std::cout << "Item count: " << htable.itemCount() << "\n";

        const auto* found = htable.firstItemWithHash(1234);
        if (found) {
            std::cout << "First item with hash 1234: " << *found << "\n";
        }

        bool removed = htable.removeItem(5678, std::string("World"));
        std::cout << "Removed 'World'? " << (removed ? "Yes" : "No") << "\n";
        std::cout << "Item count after removal: " << htable.itemCount() << "\n";

    }
    return 0;
}

```
### 출력 결과
```
Item count: 2
First item with hash 1004: 0.1 0.2 0.3 0.4
Removed accel2? Yes
Item count after removal: 1
Duplicate accel3 found: 0.1 0.2 0.3 0.4
Final item count: 1
Item count: 3
First item with hash 1234: Hello
Removed 'World'? Yes
Item count after removal: 2
```




