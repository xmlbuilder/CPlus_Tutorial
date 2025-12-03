# LightFunctionList
## 1. LightFunctionList 개요
```cpp
class LightFunctionList
{
public:
  using Callback      = void(*)(std::uintptr_t);
  using CallbackParam = std::uintptr_t;

  enum Result : unsigned int
  {
    InUse         = 0,
    Ok            = 1,
    NotFoundOrBad = 2
  };
  ...
};
```
### 핵심 특징

- 저장하는 항목:
  - void(*)(std::uintptr_t) 형태의 콜백 함수 포인터
  - std::uintptr_t (포인터 크기 정수) 파라미터

- 내부 구조:
  - 이중 연결 리스트(Node) 로 순서 유지
  - 노드를 담는 Block 단위 메모리 풀 + free list → 동적 할당 최소화
  - IntLock32 로 전체 리스트 조작을 간단하게 보호 (lock 실패 시 Result::InUse)

- 사용 용도:
  - 이벤트 핸들러 리스트, 콜백 목록, **버튼 클릭 시 등록된 함수들 호출** 같은 패턴에 적합
  - std::vector / std::list / std::function 없이, 가볍게 콜백 리스트만 구현

## 2. 내부 구조 간단 설명
### 2.1 Node
```cpp
struct Node
{
  Node*          prev;
  Node*          next;
  Callback       fn;
  CallbackParam  param;
};
```

- 이중 연결 리스트 요소:
  - prev, next: 양방향 연결
  - fn: 콜백 함수 포인터
  - param: 콜백에 넘길 인자

### 2.2 Block + free list
```cpp
struct Block
{
  Block* next;
  // 바로 뒤에 Node 배열이 연속으로 붙어 있음
};
```
- Block은 Node 여러 개를 한 번에 할당한 덩어리
- 할당 방식:
  - new char[ sizeof(Block) + capacity * sizeof(Node) ]
  - Block 헤더 뒤에 Node 배열이 붙음

- free list:
  - 할당되지 않은 Node 들은 m_free_list 단일 연결 리스트로 관리
  - 노드를 더 요청하면 free list에서 하나 꺼내서 사용
  - free list가 비면 새로운 Block을 할당하고 거기서 다시 free list 구성

## 3. public 함수별 설명
### 3.1 생성자 / 소멸자
```cpp
explicit LightFunctionList(std::size_t function_count_estimate = 0) noexcept;
~LightFunctionList();
```

- 생성자
  - function_count_estimate:
    - 예상되는 함수 개수
    - 0이면 내부에서 기본값(8개 정도)로 블록 하나를 잡아서 Node 풀을 만들어 둠

  - 첫 블록을 allocate_block()으로 할당, free list 구성

- 소멸자
  - 가능한 한 m_lock.GetLock(dtor_token)으로 잠금을 시도한 뒤,
  - 모든 Block에 대해 delete[] 호출
  - m_head, m_tail, m_free_list, m_blocks 초기화
  - 마지막에 ReturnLock 또는 BreakLock으로 락 정리 시도
    (파괴 시점에는 사실 외부에서 쓰고 있으면 안 되는 상황이라, 디버깅용 성격)

### 3.2 Result AddFunction(Callback fn, CallbackParam param)
```cpp
Result AddFunction(Callback fn, CallbackParam param);
```

- 역할:
  - 콜백 하나를 리스트 끝에 추가합니다.

- 인자:
  - fn: 호출할 함수 포인터 (nullptr이면 에러)
  - param: 함수에 넘겨줄 인자 (포인터든 인덱스든 자유롭게 사용 가능)

- 동작:
  - fn == nullptr 인 경우 Result::NotFoundOrBad 반환
  - m_lock.GetDefaultLock() 실패 시 Result::InUse (다른 스레드가 사용 중)
  - allocate_node()로 Node 하나를 가져옴
  - m_tail 뒤에 붙여서 이중 연결 리스트 업데이트
  - m_head가 없으면 첫 노드로 설정
  - m_lock.ReturnDefaultLock() 으로 락 해제

- 반환값:
  - Ok : 추가 성공
  - InUse : 락 획득 실패 → 리스트 사용 중이라 추가 못 함
  - NotFoundOrBad : fn == nullptr 또는 메모리 부족 등

### 3.3 Result RemoveFunction(Callback fn, CallbackParam param)
```cpp
Result RemoveFunction(Callback fn, CallbackParam param);
```
- 역할:
  - (fn, param) 조합이 일치하는 노드를 찾아서 제거 (첫 매칭 1개).

- 동작:
  - fn이 nullptr이면 NotFoundOrBad
  - 락 못 얻으면 InUse
  - find_node(fn)으로 첫 번째 해당 함수 포인터를 갖는 노드를 찾고,  
    뒤로 순회하면서 node->fn == fn && node->param == param인 노드를 찾음
  - 찾으면 unlink_and_free(node)로 리스트에서 제거하고 Ok
  - 못 찾으면 NotFoundOrBad

- 반환값:
  - Ok : 찾고 삭제 성공
  - InUse : 락 실패
  - NotFoundOrBad : 못 찾았거나 입력이 잘못됨

### 3.4 Result RemoveFunction(Callback fn)
```cpp
Result RemoveFunction(Callback fn);
```

- 역할:
  - 함수 포인터만 매칭해서 첫 번째 노드 하나만 삭제.
- 동작:
  - fn == nullptr → NotFoundOrBad
  - 락 실패 → InUse
  - find_node(fn) 수행:
    - 첫 matching 노드를 찾으면 unlink_and_free
    - 없으면 NotFoundOrBad

- 반환값:
  - Ok / InUse / NotFoundOrBad (위와 동일한 의미)

### 3.5 Result IsInList(Callback fn, CallbackParam param) const
```cpp
Result IsInList(Callback fn, CallbackParam param) const;
```
- 역할:
  - (fn, param) 조합이 이미 등록되어 있는지를 확인.

- 동작:
  - fn nullptr → NotFoundOrBad
  - 락 실패 → InUse
  - 내부에서 find_node(fn)로 시작해서, 같은 함수 포인터들만 순회하며
    - node->fn == fn && node->param == param 인지 확인
  - 찾으면 Ok, 못 찾으면 NotFoundOrBad

- 사용 예:
  - 중복 등록 방지용:
```cpp
if (fl.IsInList(fn, param) != LightFunctionList::Ok)
  fl.AddFunction(fn, param);
```

## 3.6 Result IsInList(Callback fn) const
```cpp
Result IsInList(Callback fn) const;
```
- 역할:
  - 해당 함수 포인터를 가진 노드가 하나라도 존재하는지 확인.

- 동작:
  - fn nullptr → NotFoundOrBad
  - 락 실패 → InUse
  - find_node(fn) 호출:
    - 매칭 노드가 있으면 Ok
    - 없으면 NotFoundOrBad

### 3.7 bool EmptyList()
```cpp
bool EmptyList();
```
- 역할:
  - 리스트를 비우되, 노드 메모리는 풀에 돌려보냅니다.
- 동작:
  - 락 실패 → false
  - m_head부터 끝까지 순회하면서:
    - free_node(node)로 각 노드를 free list에 pushing
  - m_head = m_tail = nullptr
  - 락 반환 후 true
- 특징:
  - Block 자체(큰 덩어리)는 유지되고, Node만 free list로 돌아가기 때문에  
    한 번 할당한 블록은 재사용됩니다.

### 3.8 bool CallFunctions(bool bFirstToLast)
```cpp
bool CallFunctions(bool bFirstToLast);
```
- 역할:
  - 리스트에 등록된 콜백들을 순서대로 (또는 역순으로) 호출.
- 인자:
  - bFirstToLast == true : m_head → m_tail 방향 (등록 순서대로)
  - bFirstToLast == false: m_tail → m_head 방향 (역순)
- 동작:
  - 락 실패 → false
  - 리스트 순회하면서 node->fn(node->param) 호출
  - 락 반환 후 true
- 주의:
  - 콜백이 다시 이 리스트를 변경하는 행동(재귀 Add/Remove 등)은 주의가 필요합니다  
    (기본적으로는 re-entrancy 를 고려하지 않은 단순 구조입니다).

### 3.9 bool IsInUse() const
```cpp
bool IsInUse() const
{
  return m_lock.IsLockedFlag();
}
```
- 역할:
  - 단순히 IntLock32의 상태를 그대로 보여주는 helper
- 의미:
  - true → 현재 누군가 락을 잡고 있음 (리스트 조작 중)
  - false → 사용 중이 아님

## 4. 테스트 코드 정리

앞에서 드린 샘플들은 LightFunctionList의 주요 기능을 각각 다른 각도에서 검증하는 예제였습니다.  
간단히 무엇을 확인하는 코드인지를 정리해 보겠습니다.

### 샘플 A (처음 실행하신 예제)
```
my_callback(&a) is in list
callback: 10
callback: 20
callback: 20
```

- 내용:
  - my_callback을 ( &a ), ( &b ) 파라미터로 두 번 등록
  - IsInList(my_callback, &a)를 통해 **등록 여부** 확인
  - CallFunctions(true)로 순서대로 호출
  - 그 후 Remove/추가/호출을 섞어가며 출력 확인

- 검증 포인트:
  - Add / Call / IsInList 의 기본 동작
  - 콜백이 올바른 파라미터를 받는지
  - 여러 개 등록 시 호출 순서

### 샘플 1: 서로 다른 콜백 + 순/역 호출

- 핵심 코드:
```cpp
fl.AddFunction(&print_int,    &a);
fl.AddFunction(&print_int,    &b);
fl.AddFunction(&print_double, &c);

fl.CallFunctions(true);   //  a, b, c 순
fl.CallFunctions(false);  //  c, b, a 순
```

- 검증:
  - 서로 다른 함수 포인터가 섞여 있어도 정상적으로 저장/호출되는지
  - bFirstToLast가 true/false일 때 리스트 순회 방향이 정확한지

### 샘플 2: RemoveFunction 조합
- 핵심 코드:
```cpp
fl.AddFunction(&show_id, 1);
fl.AddFunction(&show_id, 2);
fl.AddFunction(&show_id, 3);

fl.RemoveFunction(&show_id, 2); // (fn, param) 기준 삭제
fl.RemoveFunction(&show_id);    // fn 기준 첫 노드 삭제
fl.EmptyList();
```

- 검증:
  - (fn, param)으로 정확하게 특정 콜백만 제거되는지
  - 특정 함수 포인터만 보고 첫 노드 하나를 제거하는지
  - EmptyList()가 모든 노드를 제거하고, 그 후 호출 시 아무 것도 실행되지 않는지

### 샘플 3: IsInList로 중복 등록 방지

- 핵심 코드:
```cpp
auto rc = fl.IsInList(&print_msg, param);
if (rc != LightFunctionList::Ok)
  fl.AddFunction(&print_msg, param);
```
- 검증:
  - 동일 (fn, param) 조합이 이미 있을 때 중복 등록을 막을 수 있는지
  - IsInList(fn, param)과 IsInList(fn)의 용도 차이 확인

### 샘플 4: 버튼 이벤트 시스템처럼 사용

- 구조:
```cpp
struct Button
{
  LightFunctionList on_click;

  void click()
  {
    on_click.CallFunctions(true);
  }
};

btn.on_click.AddFunction(handler1, ...);
btn.on_click.AddFunction(handler2, ...);
btn.click();
```

- 검증:
- LightFunctionList를 다른 구조체 안에 포함시켜 “이벤트 시스템” 역할을 할 수 있는지
- Remove 후 다시 클릭했을 때 해당 핸들러만 빠지는지

### 샘플 5: 멀티 스레드 상황에서의 동작
- 핵심 구조:
```cpp
LightFunctionList fl;

// 등록 스레드
std::thread reg_thread([&](){
  fl.AddFunction(...);
});

// 호출 스레드
std::thread fire_thread([&](){
  fl.CallFunctions(true);
});
```

- 검증:
- 여러 스레드가 동시에 Add/Call을 수행해도 IntLock32 덕분에
  - 리스트 구조가 깨지지 않고
  - 콜백 호출이 안전하게 이뤄지는지

- 이때 std::cout은 스레드 경쟁 때문에 로그가 섞일 수 있지만,
  - 최소한 크래시 없이 콜백이 정상 호출되는지를 보는 용도

## 5. 정리

- LightFunctionList는:
  - 가벼운 콜백 리스트를 위해 설계된 구조
  - std::function / STL 컨테이너 없이, void(*)(uintptr_t) + uintptr_t 조합에만 집중
  - 고정 크기 Node 블록 + free list로 메모리 관리
  - IntLock32로 단순한 thread-safe 보장

- 공개 메서드들은:
  - 등록: AddFunction
  - 제거: RemoveFunction(2종), EmptyList
  - 조회: IsInList(2종), IsInUse
  - 호출: CallFunctions(true/false)

- 테스트 예제들은:
  - 순/역순 호출, 제거, 중복 체크, 이벤트 시스템 응용, 멀티스레드 환경 등  
    실제 사용할 만한 패턴들을 모두 한 번씩 밟아보도록 구성되어 있습니다.

---
### 테스트 코드
```cpp
#include "light_function_list.h"
#include <iostream>
#include <thread>

void my_callback(std::uintptr_t ptr)
{
    int* p = reinterpret_cast<int*>(ptr);
    std::cout << "callback: " << (p ? *p : -1) << "\n";
}


void print_int(std::uintptr_t p)
{
    int* v = reinterpret_cast<int*>(p);
    std::cout << "print_int: " << (v ? *v : -1) << "\n";
}

void print_double(std::uintptr_t p)
{
    double* v = reinterpret_cast<double*>(p);
    std::cout << "print_double: " << (v ? *v : -1.0) << "\n";
}

void show_id(std::uintptr_t id)
{
    std::cout << "show_id: " << static_cast<int>(id) << "\n";
}

void print_msg(std::uintptr_t p)
{
    const char* msg = reinterpret_cast<const char*>(p);
    std::cout << "print_msg: " << (msg ? msg : "(null)") << "\n";
}

struct Button
{
    LightFunctionList on_click;

    void click()
    {
        std::cout << "[Button clicked]\n";
        on_click.CallFunctions(true);
    }
};

void on_click_print(std::uintptr_t p)
{
    const char* name = reinterpret_cast<const char*>(p);
    std::cout << "  handler: " << (name ? name : "(null)") << "\n";
}


void print_tick(std::uintptr_t p)
{
    int id = static_cast<int>(p);
    std::cout << "[tick handler " << id << "]\n";
}

int main()
{
    {
        std::cout << "case 1" << std::endl;
        LightFunctionList flist;

        int a = 10;
        int b = 20;

        // 추가
        flist.AddFunction(&my_callback, reinterpret_cast<std::uintptr_t>(&a));
        flist.AddFunction(&my_callback, reinterpret_cast<std::uintptr_t>(&b));

        // 리스트에 있는지 확인
        auto r = flist.IsInList(&my_callback, reinterpret_cast<std::uintptr_t>(&a));
        if (r == LightFunctionList::Ok)
            std::cout << "my_callback(&a) is in list\n";

        // 순서대로 호출
        flist.CallFunctions(true);

        // 하나 제거
        flist.RemoveFunction(&my_callback, reinterpret_cast<std::uintptr_t>(&a));

        // 역순으로 호출 (이제 b만)
        flist.CallFunctions(false);

        // 모두 제거
        flist.EmptyList();
    }

    {
        std::cout << "case 2" << std::endl;
        LightFunctionList fl;
        int    a = 10;
        int    b = 20;
        double c = 3.14;

        fl.AddFunction(&print_int,    reinterpret_cast<std::uintptr_t>(&a));
        fl.AddFunction(&print_int,    reinterpret_cast<std::uintptr_t>(&b));
        fl.AddFunction(&print_double, reinterpret_cast<std::uintptr_t>(&c));

        std::cout << "--- Call first to last ---\n";
        fl.CallFunctions(true);

        std::cout << "--- Call last to first ---\n";
        fl.CallFunctions(false);

    }

    {
        std::cout << "case 3" << std::endl;
        LightFunctionList fl;

        // 같은 함수 포인터 + 서로 다른 param
        fl.AddFunction(&show_id, 1);
        fl.AddFunction(&show_id, 2);
        fl.AddFunction(&show_id, 3);

        std::cout << "=== initial list ===\n";
        fl.CallFunctions(true);  // 1, 2, 3

        // param == 2 인 것만 제거
        fl.RemoveFunction(&show_id, 2);
        std::cout << "=== after RemoveFunction(fn, 2) ===\n";
        fl.CallFunctions(true);  // 1, 3

        // 함수 포인터만 보고 첫 번째 show_id 제거
        fl.RemoveFunction(&show_id);
        std::cout << "=== after RemoveFunction(fn) ===\n";
        fl.CallFunctions(true);  // 3 만 남음

        // 전부 제거
        fl.EmptyList();
        std::cout << "=== after EmptyList ===\n";
        bool called = fl.CallFunctions(true); // 아무 것도 안 찍힘
        std::cout << "called = " << called << "\n";
    }

    {

        std::cout << "case 4" << std::endl;
        LightFunctionList fl;

        const char* msg1 = "hello";
        const char* msg2 = "world";

        auto add_unique = [&](LightFunctionList::Callback fn, std::uintptr_t param)
        {
            auto rc = fl.IsInList(fn, param);
            if (rc == LightFunctionList::Ok)
            {
                std::cout << "already in list, skip\n";
                return;
            }

            auto add_rc = fl.AddFunction(fn, param);
            if (add_rc == LightFunctionList::Ok)
                std::cout << "added\n";
            else
                std::cout << "add failed (result = " << add_rc << ")\n";
        };

        add_unique(&print_msg, reinterpret_cast<std::uintptr_t>(msg1));
        add_unique(&print_msg, reinterpret_cast<std::uintptr_t>(msg1)); // 중복
        add_unique(&print_msg, reinterpret_cast<std::uintptr_t>(msg2));

        std::cout << "=== CallFunctions ===\n";
        fl.CallFunctions(true);
    }

    {
        std::cout << "case 5" << std::endl;
        Button btn;

        const char* h1 = "first handler";
        const char* h2 = "second handler";

        btn.on_click.AddFunction(&on_click_print, reinterpret_cast<std::uintptr_t>(h1));
        btn.on_click.AddFunction(&on_click_print, reinterpret_cast<std::uintptr_t>(h2));

        btn.click();
        std::cout << "--- remove first handler ---\n";

        btn.on_click.RemoveFunction(&on_click_print, reinterpret_cast<std::uintptr_t>(h1));

        btn.click();
    }

    {

        std::cout << "case 6" << std::endl;

        LightFunctionList fl;

        std::thread reg_thread([&]()
        {
          for (int i = 0; i < 3; ++i)
          {
            fl.AddFunction(&print_tick, static_cast<std::uintptr_t>(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
          }
        });


        std::thread fire_thread([&]()
        {
          for (int t = 0; t < 5; ++t)
          {
            std::cout << "--- tick " << t << " ---\n";
            fl.CallFunctions(true); // 등록된 모든 핸들러 호출
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
          }
        });

        reg_thread.join();
        fire_thread.join();
    }
    return 0;
}
```
### 출력 결과
```
case 1
my_callback(&a) is in list
callback: 10
callback: 20
callback: 20
case 2
--- Call first to last ---
print_int: 10
print_int: 20
print_double: 3.14
--- Call last to first ---
print_double: 3.14
print_int: 20
print_int: 10
case 3
=== initial list ===
show_id: 1
show_id: 2
show_id: 3
=== after RemoveFunction(fn, 2) ===
show_id: 1
show_id: 3
=== after RemoveFunction(fn) ===
show_id: 3
=== after EmptyList ===
called = 1
case 4
added
already in list, skip
added
=== CallFunctions ===
print_msg: hello
print_msg: world
case 5
[Button clicked]
  handler: first handler
  handler: second handler
--- remove first handler ---
[Button clicked]
  handler: second handler
case 6
--- tick 0 ---
[tick handler 0]
--- tick 1 ---
[tick handler 0]
[tick handler 1]
--- tick 2 ---
[tick handler 0]
[tick handler 1]
[tick handler 2]
--- tick 3 ---
[tick handler 0]
[tick handler 1]
[tick handler 2]
--- tick 4 ---
[tick handler 0]
[tick handler 1]
[tick handler 2]
```
---

