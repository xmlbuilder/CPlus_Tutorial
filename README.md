# 📘 CPlus_Tutorial

## 🧭 프로젝트 목적

이 프로젝트는 **기존 C++ 개발자들이 Modern C++(C++11 이후)의 개념과 문법으로 자연스럽게 전환**할 수 있도록 돕기 위한 튜토리얼 모음입니다.  단순한 문법 설명을 넘어서, **개발 중에 발생할 수 있는 예기치 못한 상황의 원인과 해결 방법**을 함께 다루어 실전 감각을 높이는 데 초점을 맞추고 있습니다.

## 🎯 대상 독자

- C++98/03 스타일에 익숙하지만 Modern C++로의 전환을 고민 중인 개발자  
- 중급 이상의 C++ 경험을 가진 개발자  
- 실무에서 발생하는 C++ 관련 문제의 원인을 이해하고 싶은 개발자  
- "왜 이런 문제가 생겼지?"라는 질문을 자주 던지는 호기심 많은 개발자  

## ⚙️ 사용 방법

1. **튜토리얼 탐색**  
   각 예제는 독립적으로 구성되어 있으며, 설명 중에 있는 샘플 코드를 사용하시기 바랍니다.

2. **코드 실행**  
   예제는 표준 C++ 컴파일러(g++, clang++, MSVC 등)에서 실행 가능하며, C++17 이상을 기준으로 작성됩니다.  
   예:  
   ```bash
   g++ -std=c++17 example.cpp -o example && ./example


# CPlus_Tutorial — Table of Contents
> 기반 저장소: https://github.com/xmlbuilder/CPlus_Tutorial  
> 마지막 갱신(작성 기준): **2025-08-20 (Asia/Seoul)**

---

# 목차

## 0. 기본 개념
- [00 — is-a / has-a](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/00-is_a%26has_a.md)

## 1. 생성자·타입 도구
- [01 — 생성자: 기본/삭제](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/01-%EC%83%9D%EC%84%B1%EC%9E%90_%EA%B8%B0%EB%B3%B8_%EC%A0%9C%EA%B1%B0.md)
- [02 — `decltype`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/02-decltype.md)
- [03 — on `realloc`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/03-onrealloc.md)
- [04 — 템플릿 개요](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/04-templates.md)
- [05 — 템플릿과 `operator`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/05-Template_operator.md)
- [06 — Raw string literal](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/06-Raw_string_literal.md)

## 2. STL 시리즈 (07-xx)
- [07-01 — STL 개요](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-01-STL_overview.md)
- [07-02 — 컨테이너](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-02-STL_container.md)
- [07-03 — 이터레이터 개요](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-03-Iterator_overview.md)
- [07-04 — `std::copy_if`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-04-STL_copy_if.md)
- [07-05 — `std::array`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-05-Array_overview.md)
- [07-06 — `std::vector`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-06-Vector_overview.md)
- [07-07 — 알고리즘 개요](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-07-Algorithms_overview.md)
- [07-08 — `for_each` 예제](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-08-For_each_example.md)
- [07-09 — 이분 탐색](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-09-Binary_search.md)
- [07-10 — `find / count / find_if`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-10-Find_count_findif.md)
- [07-11 — 정렬](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-11-Sort.md)
- [07-12 — `deque / queue` 개요](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-12-Deque_queue_overview.md)
- [07-13 — STL 전 수동 리스트 구현](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-13-Manual_list_before_stl.md)
- [07-14 — `map` 요약](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/07-14-Map_summary.md)

## 3. 람다·함수·타입 유틸
- [08-01 — 람다 요약](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/08-01-Lambda_summary.md)
- [08-02 — 람다 캡처 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/08-02-Lambda_capture_guide.md)
- [09 — `mutable` 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/09-Mutable_guide.md)
- [10 — `vector::emplace_back` vs `push_back`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/10-vector_emplace_back_vs_push_back.md)
- [11 — 가변인자(Variadic) 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/11-Variadic_arguments_guide.md)
- [12 — `std::chrono` 타이머 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/12-Chrono_timer_guide.md)

## 4. 동시성(13-xx)
- [13-01 — 스레드 기초](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-01-thread_basics.md)
- [13-02 — 멀티스레딩 기초](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-02-multithreading_basic.md)
- [13-03 — 멤버함수/인자 스레딩](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-03-thread_member_function_and_args.md)
- [13-04 — 멤버 + `std::vector`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-04-thread_memeber_and_vector.md)
- [13-05 — 인터럽트: Boost vs Java](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-05-thread_interrupt_boost_vs_java.md)
- [13-06 — 원자(Atomic) 기초](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-06-Atomic.md)
- [13-06_01 — CAS 설명](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-06_01-CAS_explanation.md)
- [13-07 — `thread_local` 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-07-thread_local_guide.md)
- [13-08 — 뮤텍스 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-08-mutex_guide.md)
- [13-09 — 재귀 뮤텍스 예제](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-09-Recursive_mutex_example.md)
- [13-10 — 락 예제 모음](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-10-mutex_lock_examples.md)
- [13-11 — wait/notify 예제](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-11-thread_wait_notify_examples.md)
- [13-12 — 조건변수: 생산자/소비자](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-12-condition_variable_producer_consumer.md)
- [13-13 — `promise`/`future` 예제](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-13-promise_future_examples.md)
- [13-14 — future/promise 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-14-future_promise_guide.md)
- [13-15 — `std::async` 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-15-async_guide.md)
- [13-16 — 스레드풀](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-16-thread_pool_readme.md)
- [13-17 — 싱글 스케줄러](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-17-single_scheduler_readme.md)
- [13-18 — `mutex` + `osyncstream`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-18-mutex_osyncstream_readme.md)
- [13-19 — Qt 비동기 Progress](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-19-qt_async_progress_readme.md)
- [13-20 — Qt 이벤트 루프 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/13-20-qt_eventloop_guide.md)

## 5. 예외/에러 흐름
- [14-01 — 예외 처리](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/14-01-exception_handling.md)
- [14-02 — `noexcept` 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/14-02-noexcept_guide.md)
- [24 — 예외 클래스 정의](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/24.%EC%98%88%EC%99%B8%ED%81%B4%EB%9E%98%EC%8A%A4_%EC%A0%95%EC%9D%98.md)

## 6. 플랫폼/시스템
- [15 — Windows Long Path & UTF-8](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/15-windows_long_path_utf8.md)
- 스트림
  - [31-01 — 스트림 기본](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/31-01.stream%EA%B8%B0%EB%B3%B8.md)
  - [32-02 — 스트림 랜덤 액세스](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/32-02.stream_random_access.md)

## 7. 메모리·리소스
- [16-01 — Memory Manager](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/16-01-MemoryManager.md)
- [16-02 — Memory Manager 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/16-02-memory_manager_guide.md)
- [28 — `std::unique_ptr`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/28-unique_ptr.md)

## 8. 함수/타입 유틸 모음
- [17 — 사용자 정의 리터럴](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/17-user_literal.md)
- [18-01 — Modern `std::function`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/18-01-modern_function.md)
- [18-02 — `function` 비교](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/18-02-function_compare.md)
- [19 — `std::tuple`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/19-tuple.md)
- [20 — `initializer_list`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/20-initializer_list.md)
- [21 — `std::optional`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/21-optional.md)
- [26 — `auto` 타입](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/26.auto_type.md)
- [27 — `typeid`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/27.type_id.md)
- [29 — `cout` 포맷팅](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/29.cout_formatting.md)
- [30 — `swap`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/30.swap.md)
- [33-01 — 난수 기초](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/33-01.random.md)
- [33-02 — 난수 샘플](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/33-02.random_%EC%83%98%ED%94%8C.md)
- [34 — `std::array` + range-for](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/34.tarray_range_for.md)
- [35 — 문자열 파싱](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/35.string_parsing_cpp.md)
- [36 — 인라인 함수 오버헤드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/36.inline_function_overhead.md)
- [37 — `using`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/37.using.md)
- [38-01 — lvalue/rvalue](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/38-01.lvalue_rvalue.md)

## 9. 객체지향·다형성 (22-xx)
- [22-01 — 다형성 기초](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-01-Polymorphism_basic.md)
- [22-02 — 복사 생성자 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-02-copy_constructor_guide.md)
- [22-03 — `virtual`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-03-virtual.md)
- [22-04 — `final`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-04-final.md)
- [22-05 — 위임 생성자](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-05-delegate_constructor.md)
- [22-06 — 순수가상 클래스](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-06-pure_virtual_class.md)
- [22-07 — 연산자 오버로딩](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-07-operator_overloading.md)
- [22-08 — 연산자 오버로딩 가이드](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-08-operator_overloading_guide.md)
- [22-09 — 가상 소멸자](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-09-virtual_destructor_guide.md)
- [22-10 — 가상 상속](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-10.virtual%20Inheritance.md)
- [22-12 — const/mutable](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-12.const_mutable.md)
- 친구(friend) · 단항연산자 · 캐스트 · 심화
  - [22-13-01 — friend #1](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-01.friend.md)
  - [22-13-02 — friend #2](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-02.friend.md)
  - [22-13-03 — 단항 연산자 중복](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-03.%EB%8B%A8%ED%95%AD%EC%97%B0%EC%82%B0%EC%9E%90%EC%A4%91%EB%B3%B5.md)
  - [22-13-04 — 단항 연산자 중복(심화)](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-04.%EB%8B%A8%ED%95%AD%EC%97%B0%EC%82%B0%EC%9E%90%EC%A4%91%EB%B3%B5_%EC%8B%AC%ED%99%94.md)
  - [22-13-05 — 복사 생성자](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-05.%EB%B3%B5%EC%82%AC%EC%83%9D%EC%84%B1%EC%9E%90.md)
  - [22-13-06 — `static_cast`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-06.static_cast.md)
  - [22-13-07 — `dynamic_cast`](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-07.dynamic_cast.md)
  - [22-13-08 — 가상함수 심화 #1](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-08.%EA%B0%80%EC%83%81%ED%95%A8%EC%88%98_%EC%8B%AC%ED%99%94.md)
  - [22-13-09 — 가상함수 심화 #2](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-09.%EA%B0%80%EC%83%81%ED%95%A8%EC%88%98_%EC%8B%AC%ED%99%94.md)
  - [22-13-10 — 대입연산자 심화](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/22-13-10.%EB%8C%80%EC%9E%85%EC%97%B0%EC%82%B0%EC%9E%90_%EC%8B%AC%ED%99%94.md)

## 10. 기타
- [23-01 — on closure #1](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/23-01-on_closure_readme.md)
- [23-02 — on closure #2](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/23-02-on_closure_readme.md)
- [75 — namespace](https://github.com/xmlbuilder/CPlus_Tutorial/blob/main/23-02-on_closure_readme.md)

---

### 사용 팁
- 각 문서는 독립적으로 읽을 수 있도록 작성되어 있습니다.  
- 동시성(13-xx)과 객체지향(22-xx)은 **연속 시리즈**이므로 순서대로 읽으면 좋습니다.
- 공학 예제(메쉬/로보틱스 등)와 결합할 때는 동시성·메모리·수치 파트를 우선 참고하세요.
