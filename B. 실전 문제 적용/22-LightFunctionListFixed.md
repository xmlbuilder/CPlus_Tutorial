## 소스 코드
```cpp
#pragma once

#include <cstdint>
#include <cstddef>
#include "int_lock32.h"

// 고정 용량 콜백 리스트
//  - 템플릿 인자로 MaxFunctions 를 주면, 그만큼까지만 콜백 저장 가능
//  - 내부에서 힙(new/delete) 사용 없음
//  - 쓰레드 세이프: IntLock32 사용 (락 실패 시 Result::InUse 반환)
//
// 사용 예:
//   LightFunctionListFixed<16> fl;  // 최대 16개 콜백
//
template<std::size_t MaxFunctions>
class LightFunctionListFixed
{
public:
  using Callback      = void(*)(std::uintptr_t);
  using CallbackParam = std::uintptr_t;

  enum Result : unsigned int
  {
    InUse         = 0, // 락을 못 얻어서 리스트 사용 중
    Ok            = 1, // 성공
    NotFoundOrBad = 2, // 잘못된 인자(null 함수 등) 또는 찾지 못함
    Full          = 3  // 더 이상 추가할 수 없음 (용량 초과)
  };

  LightFunctionListFixed() noexcept
    : m_head(nullptr)
    , m_tail(nullptr)
    , m_free_list(nullptr)
  {
    init_free_list();
  }

  ~LightFunctionListFixed() = default;

  LightFunctionListFixed(const LightFunctionListFixed&)            = delete;
  LightFunctionListFixed& operator=(const LightFunctionListFixed&) = delete;

  // 콜백 추가 (리스트 끝에)
  Result AddFunction(Callback fn, CallbackParam param)
  {
    if (!fn)
      return NotFoundOrBad;

    if (!m_lock.GetDefaultLock())
      return InUse;

    Node* node = allocate_node();
    if (!node)
    {
      m_lock.ReturnDefaultLock();
      return Full;
    }

    node->prev  = m_tail;
    node->next  = nullptr;
    node->fn    = fn;
    node->param = param;

    if (m_tail)
      m_tail->next = node;
    m_tail = node;

    if (!m_head)
      m_head = node;

    m_lock.ReturnDefaultLock();
    return Ok;
  }

  // 특정 (fn, param) 조합 제거 (첫 매칭 1개)
  Result RemoveFunction(Callback fn, CallbackParam param)
  {
    if (!fn)
      return NotFoundOrBad;

    if (!m_lock.GetDefaultLock())
      return InUse;

    Result rc = NotFoundOrBad;
    Node* node = find_node(fn);
    while (node)
    {
      if (node->fn == fn && node->param == param)
      {
        unlink_and_free(node);
        rc = Ok;
        break;
      }
      node = node->next;
    }

    m_lock.ReturnDefaultLock();
    return rc;
  }

  // 특정 함수 포인터만 보고 제거 (첫 매칭 1개)
  Result RemoveFunction(Callback fn)
  {
    if (!fn)
      return NotFoundOrBad;

    if (!m_lock.GetDefaultLock())
      return InUse;

    Result rc = NotFoundOrBad;
    Node* node = find_node(fn);
    if (node)
    {
      unlink_and_free(node);
      rc = Ok;
    }

    m_lock.ReturnDefaultLock();
    return rc;
  }

  // (fn, param) 조합이 리스트에 있는지 확인
  Result IsInList(Callback fn, CallbackParam param) const
  {
    if (!fn)
      return NotFoundOrBad;

    if (!m_lock.GetDefaultLock())
      return InUse;

    Result rc = NotFoundOrBad;
    Node* node = const_cast<LightFunctionListFixed*>(this)->find_node(fn);
    while (node)
    {
      if (node->fn == fn && node->param == param)
      {
        rc = Ok;
        break;
      }
      node = node->next;
    }

    m_lock.ReturnDefaultLock();
    return rc;
  }

  // 함수 포인터만 기준으로 존재 여부 확인
  Result IsInList(Callback fn) const
  {
    if (!fn)
      return NotFoundOrBad;

    if (!m_lock.GetDefaultLock())
      return InUse;

    Result rc = NotFoundOrBad;
    Node* node = const_cast<LightFunctionListFixed*>(this)->find_node(fn);
    if (node)
      rc = Ok;

    m_lock.ReturnDefaultLock();
    return rc;
  }

  // 리스트 비우기 (노드는 free_list 로만 돌려보냄)
  // 블록(배열) 자체는 그대로라서 이후 Add에서 재사용됨.
  bool EmptyList()
  {
    if (!m_lock.GetDefaultLock())
      return false;

    Node* node = m_head;
    while (node)
    {
      Node* next = node->next;
      free_node(node);
      node = next;
    }
    m_head = nullptr;
    m_tail = nullptr;

    m_lock.ReturnDefaultLock();
    return true;
  }

  // 등록된 함수들 호출
  //  bFirstToLast = true  : 추가된 순서대로
  //                 false : 역순
  // Returns:
  //   true  = 락을 얻고 호출까지 완료
  //   false = 락 실패 (아무 것도 안 함)
  bool CallFunctions(bool bFirstToLast)
  {
    if (!m_lock.GetDefaultLock())
      return false;

    if (bFirstToLast)
    {
      for (Node* node = m_head; node; node = node->next)
      {
        node->fn(node->param);
      }
    }
    else
    {
      for (Node* node = m_tail; node; node = node->prev)
      {
        node->fn(node->param);
      }
    }

    m_lock.ReturnDefaultLock();
    return true;
  }

  // 현재 락이 잡혀 있는지 (단순 조회용)
  bool IsInUse() const
  {
    return m_lock.IsLockedFlag();
  }

  // 현재 등록된 콜백 개수 (O(n))
  std::size_t Count() const
  {
    if (!m_lock.GetDefaultLock())
      return 0;

    std::size_t c = 0;
    for (Node* node = m_head; node; node = node->next)
      ++c;

    m_lock.ReturnDefaultLock();
    return c;
  }

  // 최대 콜백 개수 (컴파일 타임 상수)
  static constexpr std::size_t MaxSize()
  {
    return MaxFunctions;
  }

private:
  struct Node
  {
    Node*          prev;
    Node*          next;
    Callback       fn;
    CallbackParam  param;
  };

  Node*  m_head;
  Node*  m_tail;
  Node*  m_free_list;
  Node   m_nodes[MaxFunctions];
  mutable IntLock32 m_lock;

private:
  void init_free_list()
  {
    // 모든 노드를 free_list에 연결
    m_free_list = nullptr;
    for (std::size_t i = 0; i < MaxFunctions; ++i)
    {
      Node* n = &m_nodes[i];
      n->prev  = nullptr;
      n->next  = m_free_list;
      n->fn    = nullptr;
      n->param = 0;
      m_free_list = n;
    }
  }

  Node* allocate_node()
  {
    if (!m_free_list)
      return nullptr; // 더 이상 사용 가능한 노드 없음

    Node* n   = m_free_list;
    m_free_list = m_free_list->next;

    n->prev  = nullptr;
    n->next  = nullptr;
    n->fn    = nullptr;
    n->param = 0;
    return n;
  }

  void free_node(Node* n)
  {
    if (!n) return;
    n->fn    = nullptr;
    n->param = 0;
    n->prev  = nullptr;
    n->next  = m_free_list;
    m_free_list = n;
  }

  void unlink_and_free(Node* node)
  {
    if (!node)
      return;

    if (node->prev)
      node->prev->next = node->next;
    else
      m_head = node->next;

    if (node->next)
      node->next->prev = node->prev;
    else
      m_tail = node->prev;

    free_node(node);
  }

  Node* find_node(Callback fn)
  {
    Node* n = m_head;
    while (n)
    {
      if (n->fn == fn)
        return n;
      n = n->next;
    }
    return nullptr;
  }
};
```

### 샘플 코드
```cpp
// light_function_list_fixed.h
// IntLock32.h
// 가 이미 프로젝트에 있다고 가정
#include "int_lock32.h"
#include "LightFunctionListFixed.h"
#include <iostream>
#include <cassert>


void cb_print(std::uintptr_t p)
{
    int* v = reinterpret_cast<int*>(p);
    std::cout << "cb_print: " << (v ? *v : -1) << "\n";
}

void test_basic()
{
    std::cout << "[TEST] basic add / call / count\n";
    LightFunctionListFixed<4> fl;

    int a = 10;
    int b = 20;

    auto rc1 = fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&a));
    auto rc2 = fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    assert(rc1 == LightFunctionListFixed<4>::Ok);
    assert(rc2 == LightFunctionListFixed<4>::Ok);

    assert(fl.Count() == 2);
    fl.CallFunctions(true);   // cb_print: 10, cb_print: 20
    fl.CallFunctions(false);  // cb_print: 20, cb_print: 10

    std::cout << "  OK\n";
}

void test_full()
{
    std::cout << "[TEST] capacity full\n";
    LightFunctionListFixed<2> fl;

    int a = 1, b = 2, c = 3;
    auto rc1 = fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&a));
    auto rc2 = fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    auto rc3 = fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&c));

    assert(rc1 == LightFunctionListFixed<2>::Ok);
    assert(rc2 == LightFunctionListFixed<2>::Ok);
    assert(rc3 == LightFunctionListFixed<2>::Full); // 더 이상 못 넣음

    assert(fl.Count() == 2);
    fl.CallFunctions(true);   // 1, 2 출력

    std::cout << "  OK\n";
}


void test_remove_and_empty()
{
    std::cout << "[TEST] remove / empty / is_in_list\n";
    LightFunctionListFixed<4> fl;

    int a = 10, b = 20, c = 30;

    fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&a));
    fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    fl.AddFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&c));


    auto rc_check = fl.IsInList(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    assert(rc_check == LightFunctionListFixed<4>::Ok);


    auto rc_rem = fl.RemoveFunction(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    assert(rc_rem == LightFunctionListFixed<4>::Ok);


    rc_check = fl.IsInList(&cb_print, reinterpret_cast<std::uintptr_t>(&b));
    assert(rc_check == LightFunctionListFixed<4>::NotFoundOrBad);

    // 함수 포인터만 보고 제거 (첫 번째 매칭)
    rc_rem = fl.RemoveFunction(&cb_print);
    assert(rc_rem == LightFunctionListFixed<4>::Ok); // a 제거

    assert(fl.Count() == 1);
    fl.CallFunctions(true); // cb_print: 30

    bool ok = fl.EmptyList();
    assert(ok);
    assert(fl.Count() == 0);

    std::cout << "  OK\n";
}

void cb_msg(std::uintptr_t p)
{
    const char* msg = reinterpret_cast<const char*>(p);
    std::cout << "cb_msg: " << (msg ? msg : "(null)") << "\n";
}

void test_unique_add()
{
    std::cout << "[TEST] unique add (IsInList)\n";
    LightFunctionListFixed<4> fl;

    const char* msg1 = "hello";
    const char* msg2 = "world";

    auto add_unique = [&](LightFunctionListFixed<4>::Callback fn,
                          std::uintptr_t param)
    {
        auto rc = fl.IsInList(fn, param);
        if (rc == LightFunctionListFixed<4>::Ok)
        {
            std::cout << "  already in list, skip\n";
            return;
        }
        auto add_rc = fl.AddFunction(fn, param);
        std::cout << "  AddFunction -> " << add_rc << "\n";
    };

    add_unique(&cb_msg, reinterpret_cast<std::uintptr_t>(msg1));
    add_unique(&cb_msg, reinterpret_cast<std::uintptr_t>(msg1)); // 중복
    add_unique(&cb_msg, reinterpret_cast<std::uintptr_t>(msg2));

    fl.CallFunctions(true); // hello, world

    std::cout << "  OK\n";
}

int main()
{
    test_basic();
    test_full();
    test_remove_and_empty();
    test_unique_add();

    std::cout << "\n=== ALL LightFunctionListFixed TESTS PASSED ===\n";
    return 0;
}

```
