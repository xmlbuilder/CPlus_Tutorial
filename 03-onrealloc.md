# Windows 전용 메모리 재할당(onrealloc) 유틸

## 핵심 포인트
- `onrealloc`은 **MSVC 6.0(_MSC_VER==1200)** 의 `realloc()` 버그(Q225099) 회피를 위해, 해당 컴파일러에서만 **직접 `malloc+memcpy+free`** 로 재구현합니다.
- 그 외 컴파일러/버전에서는 표준 `realloc()`을 그대로 사용합니다.
- `onmsize()`는 Windows 전용 비표준 함수 `_msize()`를 래핑해 **할당 블록 크기**를 조회합니다(다른 OS에서는 0 반환).

---

## 주석 보강 코드

```c
#include <stdlib.h>
#include <string.h>

/**
 * @brief size>0일 때만 malloc. 0이면 NULL 반환.
 *        (표준 malloc(0)은 구현마다 다를 수 있어 명시적으로 NULL 처리)
 */
void* onmalloc(size_t sz)
{
  return (sz > 0) ? malloc(sz) : NULL;
}

/**
 * @brief num>0 && sz>0일 때만 calloc. 둘 중 하나라도 0이면 NULL.
 *        (표준 calloc의 0 처리 역시 구현차를 피하기 위해 명시화)
 */
void* oncalloc(size_t num, size_t sz)
{
  return (num > 0 && sz > 0) ? calloc(num, sz) : NULL;
}

/**
 * @brief NULL 안전 free
 */
void onfree(void* memblock)
{
  if (memblock)
    free(memblock);
}

/**
 * @brief Windows(특히 MSVC 6.0) 전용 realloc 대체 구현.
 *
 * 사용 규칙(표준 realloc과 동일한 계약을 지킵니다):
 *  - memblock == NULL  → malloc(sz) 동작
 *  - sz == 0           → free(memblock) 호출 후 NULL 반환
 *
 * MSVC 6.0(_MSC_VER==1200)에서는 realloc 버그를 피하기 위해
 * 직접 malloc+memcpy(+free)로 shrink/grow를 처리합니다.
 * 그 외 환경에서는 realloc을 그대로 호출합니다.
 *
 * @note VC6 버그 참고: MSDN KB Q225099
 */
void* onrealloc(void* memblock, size_t sz)
{
  /* 컴파일러 버전 감지 */
#if defined(_MSC_VER)
  #if _MSC_VER == 1200
    /* MSVC 6.0: realloc 버그 회피 플래그 */
    #define ON_REALLOC_BROKEN
  #endif
#endif

#if defined(ON_REALLOC_BROKEN)
  void*  p = NULL;
  size_t memblocksz = 0;
#endif

  /* realloc 규약: NULL이면 malloc과 동일 */
  if (!memblock)
    return onmalloc(sz);

  /* realloc 규약: sz==0이면 free + NULL */
  if (sz == 0)
  {
    onfree(memblock);
    return NULL;
  }

#if defined(ON_REALLOC_BROKEN)
  /* VC6 전용 경로: buggy realloc을 피한다 */
  /* _msize: Windows CRT 전용. 할당 블록의 실제 크기 조회 */
  memblocksz = _msize(memblock);

  if (sz <= memblocksz)
  {
    /* shrink (작게 줄이기) */
    /* 아주 조금만 줄거나, 작은 블록은 굳이 재할당하지 않고 그대로 둠 */
    if (memblocksz <= 28 || (8*sz >= 7*memblocksz))
    {
      p = memblock; /* 그대로 사용 */
    }
    else
    {
      /* 더 작게 새로 할당 후 복사 */
      p = malloc(sz);
      if (p)
      {
        memcpy(p, memblock, sz);
        free(memblock);
      }
    }
  }
  else
  {
    /* grow (더 크게) */
    p = malloc(sz);
    if (p)
    {
      memcpy(p, memblock, memblocksz);
      free(memblock);
    }
  }
  return p;

#else
  /* 일반 경로: 표준 realloc 사용 */
  return realloc(memblock, sz);
#endif
}

/**
 * @brief Windows 전용: _msize로 실제 할당 블록 크기 조회.
 *        다른 OS에서는 0 반환(지원되지 않음).
 *
 * @warning _msize는 비표준이며 구현/CRT에 종속됩니다.
 */
size_t onmsize(const void* memblock)
{
#if defined(ON_OS_WINDOWS)
  return _msize((void*)memblock);
#else
  (void)memblock;
  return 0;
#endif
}
```

---

## 왜 이렇게 하나요?

- **`realloc`의 계약**
  - `realloc(NULL, sz)` → `malloc(sz)`와 동일.
  - `realloc(p, 0)`     → `free(p)` + `NULL` 반환(이 구현은 명시적으로 free 후 NULL로 통일).
- **VC6 버그 회피**
  - 오래된 MSVC 6.0 CRT의 `realloc` 버그로 크래시가 보고됨(MSDN Q225099).
  - 해당 버전에서만 `malloc+memcpy(+free)`로 안전 경로를 탑니다.
- **`_msize` 사용**
  - Windows CRT가 제공하는 비표준 API로, **할당된 블록의 실제 크기**를 조회.
  - 포터블 코드가 아니므로 Windows 한정 사용.

---

## 주의할 점

1. **이식성**: `_msize`는 Windows CRT 전용. 다른 OS에선 `onmsize()`가 0을 반환.
2. **예외/스레드 안전성**: C 스타일 메모리 API를 감싸며 예외를 던지지 않음. DLL별 CRT 혼용은 피할 것.
3. **성능**: shrink 시 특정 조건에서 재할당 생략 → 복사 비용 절감.
4. **정렬(Alignment)**: CRT가 보장하는 정렬을 그대로 따름.
5. **실패 시 포인터**: 확장 실패 시 NULL 반환, 기존 포인터는 유효(표준 realloc과 동일).

---

## 간단 사용 예

```c
#include <stdio.h>

int main(void)
{
  size_t n = 10;
  int* a = (int*)onmalloc(n * sizeof(int));
  if (!a) return 1;

  for (size_t i = 0; i < n; ++i) a[i] = (int)i;

  /* 더 크게 */
  n = 20;
  int* b = (int*)onrealloc(a, n * sizeof(int));
  if (!b) {
    onfree(a);
    return 1;
  }
  a = b;

  /* 0으로 줄이면 free + NULL */
  a = (int*)onrealloc(a, 0);
  if (a != NULL) {
    onfree(a);
  }

  return 0;
}
```

---

## 대안/확장 아이디어
- **사이즈 안전 연산**: `num*sz` 오버플로 검사 추가.
- **디버그 헬퍼**: 디버그 빌드에서 메모리 패턴 채우기.
- **플랫폼 추상화**: Linux/Posix는 `malloc_usable_size()`로 대체.
- **C++ 사용 시**: new/delete와 혼용 금지.
