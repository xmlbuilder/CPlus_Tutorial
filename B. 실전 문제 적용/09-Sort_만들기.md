# Sort
## 개발 취지
### 1. C 기반 + 템플릿 없는 시대의 유니버설 정렬기
- C 스타일이 강하고, 템플릿도 거의 안 쓰는 구조라면
- std::sort<T> 같은 템플릿 기반 정렬 std::vector<T> 기반의 고급 컨테이너 에 의존하기가 애매함.
- **void + element size + 함수 포인터 비교자** 기반의 유니버설 정렬기를 하나 만들어 두면, C에서도 쓰고 C++에서도 쓰고  
  템플릿 없이, struct 배열이든, double 배열이든 전부 같은 함수로 처리 heap_sort(base, nel, width, compar) 같은 형태.

### 2. 플랫폼별 qsort / qsort_r / qsort_s 차이 피하기
- 플랫폼마다: qsort는 서명은 같지만,
- 컨텍스트를 받는 qsort_r / qsort_s는
- POSIX: int compar(const void*, const void*, void*)
- MSVC: int compar(void*, const void*, const void*)
- 그 외 구현도 제각각 이라서, **컨텍스트 있는 정렬** 을 쓴다고 하면 정말 골치 아픕니다.

#### ON_hsort(heap sort with context) 구현
- cmp(context, a, b) 형태의 비교 함수 를 써서, 어느 컴파일러에서도 같은 코드가 돌아가도록 만듬.  
  (CompareFuncContext 버전으로 뽑아냄.)

### 3. 힙 정렬: 비재귀 + 최악에도 O(n log n) 보장

- 표준 qsort 구현은 보통 퀵소트 기반이라서:
  - 평균: O(n log n)
  - 최악: O(n²)
  - 깊은 재귀 호출로 스택 문제 가능
  - CAD 라이브러리 입장에서는:
    - 수만~수십만 개짜리 배열 정렬이 흔하고
    - 입력이 어느 정도 패턴을 갖는 경우도 많아서 이미 거의 정렬, 정렬된 데이터에 조금만 변경하는 구조
    - 퀵소트의 최악 케이스에 맞아떨어지기 딱 좋은 환경입니다.
- **비재귀 힙 정렬** 을 택해서:
  - 항상 O(n log n)
  - 스택 사용 없음
  - 동작이 플랫폼에 관계없이 결정적(deterministic)
### 4. 커스텀 메모리 관리 / 디버깅과의 통합

- 원본 코드를 보면: onmalloc, onfree 같은 커스텀 할당 함수
- 디버그 빌드에서 메모리 추적, ASSERT 와 엮여 있는 경우가 많습니다.
- 정렬 중에는 작은 워크 버퍼를 많이 할당/해제하게 되는데, 진접 만든 allocator로 통제하면:
  - 메모리 누수 추적
  - 커스텀 풀 할당자와의 연동
  - 디버그 로그 같은 걸 훨씬 쉽게 할 수 있습니다.
  - 표준 qsort 안쪽에서 어떤 할당을 하는지, CRT가 어떻게 동작하는지는 제어할 수 없으니, 직접 제어.

### 5. 역사 + 이식성: “어디서나 같은 동작”
- 오래된 컴파일러도 지원해야 하고 Windows, macOS, Linux, 유닉스 계열까지 광범위한 환경에서 돌아가는 게 목표
- **어떤 환경의 qsort가 어떻게 구현돼 있을지 믿기 힘들다**
  - 그냥 우리가 검증한 정렬기를 하나 들고 다니는 것을 선택.
- 특히 CAD 쪽은 수치적 재현성도 중요해서, 정렬 순서가 미묘하게 달라서 엡실론 비교나  
  Topology 생성 결과가 달라지는 걸 피할 수 있음.
- 정리하면
  - 템플릿 없는 C 스타일 코드에서 모든 타입에 쓸 수 있는 유니버설 정렬기 필요
  - qsort_r / qsort_s 플랫폼별 서명 차이, 이식성 문제 회피
  - 힙 정렬로 항상 O(n log n) + 비재귀 + 결정적 동작
  - 커스텀 메모리 관리와 디버깅을 정렬 내부까지 통제
  - 오래된/다양한 컴파일러 및 OS에서 동일한 행태를 보장하고 싶었음

---
## 소스 코드
```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace util_sort
{
    // 어떤 알고리즘을 쓸지 선택
    enum class SortAlgorithm
    {
        HeapSort,
        QuickSort
      };

    using CompareFunc = int(*)(const void*, const void*);
    using CompareFuncContext = int(*)(void*, const void*, const void*);


    void quick_sort(
        void* base,
        std::size_t nel,
        std::size_t width,
        CompareFunc compare_ctx
    );


    void quick_sort(
        void* base,
        std::size_t nel,
        std::size_t width,
        CompareFuncContext compare_ctx,
        void* context
    );


    void heap_sort(
        void* base,
        std::size_t nel,
        std::size_t width,
        CompareFunc compare_ctx
    );

    // 힙 정렬 (컨텍스트 있음)
    void heap_sort(
        void* base,
        std::size_t nel,
        std::size_t width,
        CompareFuncContext compare_ctx,
        void* context
    );

    // ===== 편의용 타입별 래퍼 =====

    void sort_double(
        double* a,
        std::size_t nel,
        SortAlgorithm algo = SortAlgorithm::QuickSort
    );

    void sort_double_increasing(
        double* a,
        std::size_t nel
    );

    void sort_double_decreasing(
        double* a,
        std::size_t nel
    );

    void sort_float(
        float* a,
        std::size_t nel,
        SortAlgorithm algo = SortAlgorithm::QuickSort
    );

    void sort_int(
        int* a,
        std::size_t nel,
        SortAlgorithm algo = SortAlgorithm::QuickSort
    );

    void sort_uint32(
        std::uint32_t* a,
        std::size_t nel,
        SortAlgorithm algo = SortAlgorithm::QuickSort
    );

    void sort_uint64(
        std::uint64_t* a,
        std::size_t nel,
        SortAlgorithm algo = SortAlgorithm::QuickSort
    );

}


template <typename T>
bool is_sorted_increasing(const std::vector<T>& v)
{
    if (v.size() < 2) return true;
    for (std::size_t i = 1; i < v.size(); ++i)
    {
        if (v[i-1] > v[i])
            return false;
    }
    return true;
}

template <typename T>
bool is_sorted_decreasing(const std::vector<T>& v)
{
    if (v.size() < 2) return true;
    for (std::size_t i = 1; i < v.size(); ++i)
    {
        if (v[i-1] < v[i])
            return false;
    }
    return true;
}


template <typename T>
std::vector<T> make_random_vector(std::size_t n, T min_val, T max_val)
{
    std::vector<T> v;
    v.reserve(n);

    std::mt19937_64 rng(1234567);
    if constexpr (std::is_floating_point<T>::value)
    {
        std::uniform_real_distribution<T> dist(min_val, max_val);
        for (std::size_t i = 0; i < n; ++i)
            v.push_back(dist(rng));
    }
    else
    {
        std::uniform_int_distribution<long long> dist((long long)min_val, (long long)max_val);
        for (std::size_t i = 0; i < n; ++i)
            v.push_back((T)dist(rng));
    }

    return v;
}
```

```cpp
//
// Created by jhjeong on 2025-12-03.
//
#include "sort_utils.h"

#include <cstdlib>   // qsort, malloc, free
#include <cstring>   // memcpy
#include <new>       // std::nothrow

namespace util_sort
{
  namespace
  {
    constexpr std::size_t WORK_SIZE = 64;

    // ---- 타입별 비교 함수 ----
    int cmp_double(const void* a, const void* b)
    {
      double da = *static_cast<const double*>(a);
      double db = *static_cast<const double*>(b);
      if (da < db) return -1;
      if (da > db) return  1;
      return 0;
    }

    int cmp_double_desc(const void* a, const void* b)
    {
      double da = *static_cast<const double*>(a);
      double db = *static_cast<const double*>(b);
      if (da > db) return -1; // 내림차순
      if (da < db) return  1;
      return 0;
    }

    int cmp_float(const void* a, const void* b)
    {
      float fa = *static_cast<const float*>(a);
      float fb = *static_cast<const float*>(b);
      if (fa < fb) return -1;
      if (fa > fb) return  1;
      return 0;
    }

    int cmp_int(const void* a, const void* b)
    {
      int ia = *static_cast<const int*>(a);
      int ib = *static_cast<const int*>(b);
      if (ia < ib) return -1;
      if (ia > ib) return  1;
      return 0;
    }

    int cmp_uint32(const void* a, const void* b)
    {
      std::uint32_t ia = *static_cast<const std::uint32_t*>(a);
      std::uint32_t ib = *static_cast<const std::uint32_t*>(b);
      if (ia < ib) return -1;
      if (ia > ib) return  1;
      return 0;
    }

    int cmp_uint64(const void* a, const void* b)
    {
      std::uint64_t ia = *static_cast<const std::uint64_t*>(a);
      std::uint64_t ib = *static_cast<const std::uint64_t*>(b);
      if (ia < ib) return -1;
      if (ia > ib) return  1;
      return 0;
    }
  } // anonymous namespace


  void heap_sort(
      void* base,
      std::size_t nel,
      std::size_t width,
      CompareFunc compar
  )
  {
    if (!base || nel < 2 || width == 0 || !compar)
      return;

    std::size_t i_end = nel - 1;
    std::size_t k     = nel >> 1;

    unsigned char* data = static_cast<unsigned char*>(base);
    unsigned char* e_end = data + i_end * width;

    unsigned char work_memory[WORK_SIZE];
    unsigned char* e_tmp =
      (width > WORK_SIZE)
      ? static_cast<unsigned char*>(std::malloc(width))
      : work_memory;

    if (!e_tmp)
      return; // 메모리 할당 실패 시 그냥 리턴

    for (;;)
    {
      if (k)
      {
        --k;
        std::memcpy(e_tmp, data + k * width, width); // e_tmp = e[k]
      }
      else
      {
        std::memcpy(e_tmp, e_end, width); // e_tmp = e[i_end]
        std::memcpy(e_end, data, width);  // e[i_end] = e[0]
        if (!(--i_end))
        {
          std::memcpy(data, e_tmp, width); // e[0] = e_tmp
          break;
        }
        e_end -= width;
      }

      std::size_t i = k;
      std::size_t j = (k << 1) + 1;
      unsigned char* e_i = data + i * width;

      while (j <= i_end)
      {
        unsigned char* e_j = data + j * width;
        // 자식 두 개 중 더 큰 쪽 선택
        if (j < i_end && compar(e_j, e_j + width) < 0)
        {
          ++j;
          e_j += width;
        }
        if (compar(e_tmp, e_j) < 0) // e_tmp < e[j]
        {
          std::memcpy(e_i, e_j, width); // e[i] = e[j]
          i = j;
          e_i = e_j;
          j = (j << 1) + 1;
        }
        else
        {
          j = i_end + 1;
        }
      }

      std::memcpy(e_i, e_tmp, width); // e[i] = e_tmp
    }

    if (width > WORK_SIZE)
      std::free(e_tmp);
  }

  // ===========================
  //   heap_sort (with context)
  // ===========================
  void heap_sort(
      void* base,
      std::size_t nel,
      std::size_t width,
      CompareFuncContext compar,
      void* context
  )
  {
    if (!base || nel < 2 || width == 0 || !compar)
      return;

    std::size_t i_end = nel - 1;
    std::size_t k     = nel >> 1;

    unsigned char* data = static_cast<unsigned char*>(base);
    unsigned char* e_end = data + i_end * width;

    unsigned char work_memory[WORK_SIZE];
    unsigned char* e_tmp =
      (width > WORK_SIZE)
      ? static_cast<unsigned char*>(std::malloc(width))
      : work_memory;

    if (!e_tmp)
      return; // 메모리 할당 실패 시 그냥 리턴

    for (;;)
    {
      if (k)
      {
        --k;
        std::memcpy(e_tmp, data + k * width, width); // e_tmp = e[k]
      }
      else
      {
        std::memcpy(e_tmp, e_end, width); // e_tmp = e[i_end]
        std::memcpy(e_end, data, width);  // e[i_end] = e[0]
        if (!(--i_end))
        {
          std::memcpy(data, e_tmp, width); // e[0] = e_tmp
          break;
        }
        e_end -= width;
      }

      std::size_t i = k;
      std::size_t j = (k << 1) + 1;
      unsigned char* e_i = data + i * width;

      while (j <= i_end)
      {
        unsigned char* e_j = data + j * width;
        // 자식 두 개 중 더 큰 쪽 선택 (context 비교)
        if (j < i_end && compar(context, e_j, e_j + width) < 0)
        {
          ++j;
          e_j += width;
        }
        if (compar(context, e_tmp, e_j) < 0) // e_tmp < e[j]
        {
          std::memcpy(e_i, e_j, width); // e[i] = e[j]
          i = j;
          e_i = e_j;
          j = (j << 1) + 1;
        }
        else
        {
          j = i_end + 1;
        }
      }

      std::memcpy(e_i, e_tmp, width); // e[i] = e_tmp
    }

    if (width > WORK_SIZE)
      std::free(e_tmp);
  }

  // ===========================
  //   quick_sort (no context)
  // ===========================
  void quick_sort(
      void* base,
      std::size_t nel,
      std::size_t width,
      CompareFunc compar
  )
  {
    if (!base || nel < 2 || width == 0 || !compar)
      return;

    // 가능한 한 표준 qsort 사용
    ::qsort(base, nel, width, compar);
  }

  // ===========================
  //   quick_sort (with context)
  //   플랫폼별 qsort_r, qsort_s
  //   차이가 있어서, 여기서는
  //   보수적으로 heap_sort로 처리
  // ===========================
  void quick_sort(
      void* base,
      std::size_t nel,
      std::size_t width,
      CompareFuncContext compare_ctx,
      void* context
  )
  {
    // 간단하고 이식성 좋게: 그냥 heap_sort 호출
    heap_sort(base, nel, width, compare_ctx, context);
  }

  // ===========================
  //   타입별 래퍼
  // ===========================
  void sort_double(
      double* a,
      std::size_t nel,
      SortAlgorithm algo
  )
  {
    if (!a || nel < 2)
      return;

    if (algo == SortAlgorithm::HeapSort)
      heap_sort(a, nel, sizeof(double), &cmp_double);
    else
      quick_sort(a, nel, sizeof(double), &cmp_double);
  }

  void sort_double_increasing(
      double* a,
      std::size_t nel
  )
  {
    sort_double(a, nel, SortAlgorithm::QuickSort);
  }

  void sort_double_decreasing(
      double* a,
      std::size_t nel
  )
  {
    if (!a || nel < 2)
      return;
    quick_sort(a, nel, sizeof(double), &cmp_double_desc);
  }

  void sort_float(
      float* a,
      std::size_t nel,
      SortAlgorithm algo
  )
  {
    if (!a || nel < 2)
      return;

    if (algo == SortAlgorithm::HeapSort)
      heap_sort(a, nel, sizeof(float), &cmp_float);
    else
      quick_sort(a, nel, sizeof(float), &cmp_float);
  }

  void sort_int(
      int* a,
      std::size_t nel,
      SortAlgorithm algo
  )
  {
    if (!a || nel < 2)
      return;

    if (algo == SortAlgorithm::HeapSort)
      heap_sort(a, nel, sizeof(int), &cmp_int);
    else
      quick_sort(a, nel, sizeof(int), &cmp_int);
  }

  void sort_uint32(
      std::uint32_t* a,
      std::size_t nel,
      SortAlgorithm algo
  )
  {
    if (!a || nel < 2)
      return;

    if (algo == SortAlgorithm::HeapSort)
      heap_sort(a, nel, sizeof(std::uint32_t), &cmp_uint32);
    else
      quick_sort(a, nel, sizeof(std::uint32_t), &cmp_uint32);
  }

  void sort_uint64(
      std::uint64_t* a,
      std::size_t nel,
      SortAlgorithm algo
  )
  {
    if (!a || nel < 2)
      return;

    if (algo == SortAlgorithm::HeapSort)
      heap_sort(a, nel, sizeof(std::uint64_t), &cmp_uint64);
    else
      quick_sort(a, nel, sizeof(std::uint64_t), &cmp_uint64);
  }

} // namespace util_sort
```
---
## 테스트 코드
```cpp
// sort_utils_test.cpp
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include "sort_utils.h"

using namespace util_sort;


struct Pair
{
  int key;
  int value;
};

int pair_compare_noctx(const void* a, const void* b)
{
  const Pair* pa = static_cast<const Pair*>(a);
  const Pair* pb = static_cast<const Pair*>(b);
  if (pa->key < pb->key) return -1;
  if (pa->key > pb->key) return  1;
  return 0;
}


struct PairCompareContext
{
  bool ascending; // true: key 오름차순, false: 내림차순
};

int pair_compare_ctx(void* ctx, const void* a, const void* b)
{
  auto* c = static_cast<PairCompareContext*>(ctx);
  const Pair* pa = static_cast<const Pair*>(a);
  const Pair* pb = static_cast<const Pair*>(b);

  if (pa->key < pb->key) return c->ascending ? -1 : 1;
  if (pa->key > pb->key) return c->ascending ?  1 : -1;
  return 0;
}

bool test_double_sort()
{
  auto v = make_random_vector<double>(1000, -1000.0, 1000.0);
  auto v2 = v;
  auto v3 = v;

  // quick sort 오름차순
  sort_double(v.data(), v.size(), SortAlgorithm::QuickSort);
  if (!is_sorted_increasing(v))
  {
    std::cerr << "[FAIL] double quick_sort increasing\n";
    return false;
  }

  // heap sort 오름차순
  sort_double(v2.data(), v2.size(), SortAlgorithm::HeapSort);
  if (!is_sorted_increasing(v2))
  {
    std::cerr << "[FAIL] double heap_sort increasing\n";
    return false;
  }

  // 내림차순 (전용 함수)
  sort_double_decreasing(v3.data(), v3.size());
  if (!is_sorted_decreasing(v3))
  {
    std::cerr << "[FAIL] double decreasing\n";
    return false;
  }

  std::cout << "[PASS] double sort\n";
  return true;
}

bool test_float_sort()
{
  auto v = make_random_vector<float>(1000, -1000.0f, 1000.0f);

  sort_float(v.data(), v.size(), SortAlgorithm::QuickSort);
  if (!is_sorted_increasing(v))
  {
    std::cerr << "[FAIL] float quick_sort increasing\n";
    return false;
  }

  auto v2 = v;
  sort_float(v2.data(), v2.size(), SortAlgorithm::HeapSort);
  if (!is_sorted_increasing(v2))
  {
    std::cerr << "[FAIL] float heap_sort increasing\n";
    return false;
  }

  std::cout << "[PASS] float sort\n";
  return true;
}

bool test_int_sort()
{
  auto v = make_random_vector<int>(1000, -1000, 1000);

  sort_int(v.data(), v.size(), SortAlgorithm::QuickSort);
  if (!is_sorted_increasing(v))
  {
    std::cerr << "[FAIL] int quick_sort increasing\n";
    return false;
  }

  auto v2 = v;
  sort_int(v2.data(), v2.size(), SortAlgorithm::HeapSort);
  if (!is_sorted_increasing(v2))
  {
    std::cerr << "[FAIL] int heap_sort increasing\n";
    return false;
  }

  std::cout << "[PASS] int sort\n";
  return true;
}

bool test_uint32_sort()
{
  auto v = make_random_vector<std::uint32_t>(1000, 0u, 100000u);

  sort_uint32(v.data(), v.size(), SortAlgorithm::QuickSort);
  if (!is_sorted_increasing(v))
  {
    std::cerr << "[FAIL] uint32 quick_sort increasing\n";
    return false;
  }

  auto v2 = v;
  sort_uint32(v2.data(), v2.size(), SortAlgorithm::HeapSort);
  if (!is_sorted_increasing(v2))
  {
    std::cerr << "[FAIL] uint32 heap_sort increasing\n";
    return false;
  }

  std::cout << "[PASS] uint32 sort\n";
  return true;
}

bool test_uint64_sort()
{
  auto v = make_random_vector<std::uint64_t>(1000, 0ull, 100000ull);

  sort_uint64(v.data(), v.size(), SortAlgorithm::QuickSort);
  if (!is_sorted_increasing(v))
  {
    std::cerr << "[FAIL] uint64 quick_sort increasing\n";
    return false;
  }

  auto v2 = v;
  sort_uint64(v2.data(), v2.size(), SortAlgorithm::HeapSort);
  if (!is_sorted_increasing(v2))
  {
    std::cerr << "[FAIL] uint64 heap_sort increasing\n";
    return false;
  }

  std::cout << "[PASS] uint64 sort\n";
  return true;
}

bool test_heap_sort_pair_noctx()
{
  Pair arr[10] = {
    {5,  50},
    {1,  10},
    {9,  90},
    {3,  30},
    {7,  70},
    {2,  20},
    {8,  80},
    {0,   0},
    {4,  40},
    {6,  60},
  };

  heap_sort(arr, 10, sizeof(Pair), &pair_compare_noctx);

  for (int i = 0; i < 10; ++i)
  {
    if (arr[i].key != i)
    {
      std::cerr << "[FAIL] heap_sort Pair no context (index " << i
                << ", key=" << arr[i].key << ")\n";
      return false;
    }
  }

  std::cout << "[PASS] heap_sort Pair no context\n";
  return true;
}

bool test_heap_sort_pair_ctx()
{
  Pair arr[5] = {
    {10, 100},
    {30, 300},
    {20, 200},
    {40, 400},
    {0,   0}
  };

  // 오름차순
  {
    Pair tmp[5];
    std::memcpy(tmp, arr, sizeof(arr));

    PairCompareContext ctx{ true };
    heap_sort(tmp, 5, sizeof(Pair), &pair_compare_ctx, &ctx);

    for (int i = 1; i < 5; ++i)
    {
      if (tmp[i-1].key > tmp[i].key)
      {
        std::cerr << "[FAIL] heap_sort Pair context ascending\n";
        return false;
      }
    }
  }

  // 내림차순
  {
    Pair tmp[5];
    std::memcpy(tmp, arr, sizeof(arr));

    PairCompareContext ctx{ false };
    heap_sort(tmp, 5, sizeof(Pair), &pair_compare_ctx, &ctx);

    for (int i = 1; i < 5; ++i)
    {
      if (tmp[i-1].key < tmp[i].key)
      {
        std::cerr << "[FAIL] heap_sort Pair context descending\n";
        return false;
      }
    }
  }

  std::cout << "[PASS] heap_sort Pair with context\n";
  return true;
}

int main()
{
  bool ok = true;

  ok = ok && test_double_sort();
  ok = ok && test_float_sort();
  ok = ok && test_int_sort();
  ok = ok && test_uint32_sort();
  ok = ok && test_uint64_sort();
  ok = ok && test_heap_sort_pair_noctx();
  ok = ok && test_heap_sort_pair_ctx();

  if (ok)
  {
    std::cout << "\n=== ALL SORT TESTS PASSED ===\n";
    return 0;
  }
  else
  {
    std::cerr << "\n=== SOME SORT TESTS FAILED ===\n";
    return 1;
  }
}
```
### 테스트 결과
```
[PASS] double sort
[PASS] float sort
[PASS] int sort
[PASS] uint32 sort
[PASS] uint64 sort
[PASS] heap_sort Pair no context
[PASS] heap_sort Pair with context
```

---
