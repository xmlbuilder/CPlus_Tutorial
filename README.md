# 📘 CPlus_Tutorial

## 🧭 프로젝트 목적

이 프로젝트는 **기존 C++ 개발자들이 Modern C++(C++11 이후)의 개념과 문법으로 자연스럽게 전환**할 수 있도록 돕기 위한 튜토리얼 모음입니다.  
단순한 문법 설명을 넘어서, **개발 중에 발생할 수 있는 예기치 못한 상황의 원인과 해결 방법**을 함께 다루어 실전 감각을 높이는 데 초점을 맞추고 있습니다.

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

---

# 목차
## 📚 CPlus_Tutorial 목차
- [0. C++ 기본과 객체지향 기초](#0-C-기본과-객체지향-기초)
- [1. Modern C++ 핵심 문법](#1-Modern-C-핵심-문법)
- [2. 템플릿 & 제네릭 프로그래밍](#2-템플릿--제네릭-프로그래밍)
- [3. STL & 컨테이너 & 알고리즘](#3-STL--컨테이너--알고리즘)
- [4. 함수형 & 람다 & 클로저](#4-함수형--람다--클로저)
- [5. 동시성 (Concurrency & Multithreading)](#5-동시성-Concurrency--Multithreading)
- [6. 예외와 오류 처리](#6-예외와-오류-처리)
- [7. 메모리 관리 & 시스템](#7-메모리-관리--시스템)
- [8. 입출력 & 유틸리티](#8-입출력--유틸리티)
- [9. 고급 주제 & 패턴](#9-고급-주제--패턴)
- [10. vcpkg로 테스트 통합](#10-vcpkg로-테스트-통합)
- [A. Design Pattern](#A-Design-Pattern)
- [B. 실전 문제 적용](#B-실전-문제-적용)

## 0. C++ 기본과 객체지향 기초

- [is-a & has-a](./0.%20C%2B%2B%20기본과%20객체지향%20기초/01-is_a&has_a.md)
- [생성자_기본_제거](./0.%20C%2B%2B%20기본과%20객체지향%20기초/02-생성자_기본_제거.md)
- [Polymorphism](./0.%20C%2B%2B%20기본과%20객체지향%20기초/03-Polymorphism_basic.md)
- [복사_생성자_가이드](./0.%20C%2B%2B%20기본과%20객체지향%20기초/04-copy_constructor_guide.md)
- [가상함수](./0.%20C%2B%2B%20기본과%20객체지향%20기초/05-virtual.md)
- [final](./0.%20C%2B%2B%20기본과%20객체지향%20기초/06-final.md)
- [위임_생성자](./0.%20C%2B%2B%20기본과%20객체지향%20기초/07-delegate_constructor.md)
- [순수_가상_클래스](./0.%20C%2B%2B%20기본과%20객체지향%20기초/08-pure_virtual_class.md)
- [연산자_오버로딩](./0.%20C%2B%2B%20기본과%20객체지향%20기초/09-operator_overloading.md)
- [연산자_오버로딩_가이드](./0.%20C%2B%2B%20기본과%20객체지향%20기초/10-operator_overloading_guide.md)
- [가상소멸자_가이드](./0.%20C%2B%2B%20기본과%20객체지향%20기초/11-virtual_destructor_guide.md)
- [가상_상속](./0.%20C%2B%2B%20기본과%20객체지향%20기초/12-virtual_Inheritance.md)
- [const와_mutable](./0.%20C%2B%2B%20기본과%20객체지향%20기초/13-const_mutable.md)
- [friend](./0.%20C%2B%2B%20기본과%20객체지향%20기초/14-friend.md)
- [friend_상세](./0.%20C%2B%2B%20기본과%20객체지향%20기초/15-friend_상세.md)
- [단항연산자중복](./0.%20C%2B%2B%20기본과%20객체지향%20기초/16-단항연산자중복.md)
- [단항연산자중복_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/17-단항연산자중복_심화.md)
- [복사생성자](./0.%20C%2B%2B%20기본과%20객체지향%20기초/18-복사생성자.md)
- [static_cast](./0.%20C%2B%2B%20기본과%20객체지향%20기초/19-static_cast.md)
- [dynamic_cast](./0.%20C%2B%2B%20기본과%20객체지향%20기초/20-dynamic_cast.md)
- [가상함수_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/21-가상함수_심화.md)
- [대입연산자_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/22-13-10.대입연산자_심화.md)
- [가상함수_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/22-가상함수_심화.md)
- [static](./0.%20C%2B%2B%20기본과%20객체지향%20기초/23-static.md)
- [static_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/24-static_심화.md)
- [namespace](./0.%20C%2B%2B%20기본과%20객체지향%20기초/25-namespace.md)
- [enum_class](./0.%20C%2B%2B%20기본과%20객체지향%20기초/26-enum_class.md)
- [inline](./0.%20C%2B%2B%20기본과%20객체지향%20기초/27-inline.md)
- [대입연산자_심화](./0.%20C%2B%2B%20기본과%20객체지향%20기초/28-대입연산자_심화.md)


## 1. Modern C++ 핵심 문법
- [auto](./1.%20Modern%20C%2B%2B%20핵심%20문법/01-auto.md)
- [auto_type](./1.%20Modern%20C%2B%2B%20핵심%20문법/02-auto_type.md)
- [optional](./1.%20Modern%20C%2B%2B%20핵심%20문법/03-optional.md)
- [decltype](./1.%20Modern%20C%2B%2B%20핵심%20문법/04-decltype.md)
- [modern_function](./1.%20Modern%20C%2B%2B%20핵심%20문법/05-modern_function.md)
- [function_compare](./1.%20Modern%20C%2B%2B%20핵심%20문법/06-function_compare.md)
- [tuple](./1.%20Modern%20C%2B%2B%20핵심%20문법/07-tuple.md)
- [type_id](./1.%20Modern%20C%2B%2B%20핵심%20문법/08-type_id.md)
- [variant](./1.%20Modern%20C%2B%2B%20핵심%20문법/09-variant.md)
- [constexpr](./1.%20Modern%20C%2B%2B%20핵심%20문법/10-constexpr.md)
- [nullptr](./1.%20Modern%20C%2B%2B%20핵심%20문법/11-nullptr.md)
- [initializer_list](./1.%20Modern%20C%2B%2B%20핵심%20문법/12-initializer_list.md)
- [user_literal](./1.%20Modern%20C%2B%2B%20핵심%20문법/13-user_literal.md)
- [RTTI](./1.%20Modern%20C%2B%2B%20핵심%20문법/14-RTTI.md)


## 2. 템플릿 & 제네릭 프로그래밍
- [templates](./2.%20템플릿%20%26%20제네릭%20프로그래밍/01-templates.md)
- [Template_operator](./2.%20템플릿%20%26%20제네릭%20프로그래밍/02-Template_operator.md)
- [Variadic_arguments_guide](./2.%20템플릿%20%26%20제네릭%20프로그래밍/03-Variadic_arguments_guide.md)
- [template_map](./2.%20템플릿%20%26%20제네릭%20프로그래밍/04-template_map.md)
- [template_folding](./2.%20템플릿%20%26%20제네릭%20프로그래밍/05-template_folding.md)
- [template_sandbox](./2.%20템플릿%20%26%20제네릭%20프로그래밍/06-template_sandbox.md)
- [가변인자](./2.%20템플릿%20%26%20제네릭%20프로그래밍/07-가변인자.md)
- [template_본질](./2.%20템플릿%20%26%20제네릭%20프로그래밍/08-template_본질.md)

## 3. STL & 컨테이너 & 알고리즘

- [STL_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/01-STL_overview.md)
- [STL_container](.3.%20STL%20%26%20컨테이너%20%26%20알고리즘/02-STL_container.md)
- [Iterator_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/03-Iterator_overview.md)
- [STL_copy_if](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/04-STL_copy_if.md)
- [Array_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/05-Array_overview.md)
- [stl_format](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/06.stl_format.md)
- [Algorithms_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/07-Algorithms_overview.md)
- [For_each_example](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/08-For_each_example.md)
- [For_each_example](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/09-For_each_example.md)
- [For_each_example](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/10-For_each_example.md)
- [Sort](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/11-Sort.md)
- [Deque_queue_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/12-Deque_queue_overview.md)
- [Manual_list_before_stl](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/13-Manual_list_before_stl.md)
- [Map_summary](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/14-Map_summary.md)
- [Vector_overview](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/15-Vector_overview.md)
- [vector_emplace_back_vs_push_back](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/16-vector_emplace_back_vs_push_back.md)
- [swap](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/17.swap.md)
- [tarray_range_for](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/18.tarray_range_for.md)
- [fast_list](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/19.fast_list.md)
- [for_each](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/20.for_each.md)
- [back_inserter](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/21.back_inserter.md)
- [transform](./3.%20STL%20%26%20컨테이너%20%26%20알고리즘/22.transform.md)

## 4. 함수형 & 람다 & 클로저
- [Lambda_summary](./4.%20함수형%20%26%20람다%20%26%20클로저/01-Lambda_summary.md)
- [Lambda_capture_guide](./4.%20함수형%20%26%20람다%20%26%20클로저/02-Lambda_capture_guide.md)
- [Mutable_guide](./4.%20함수형%20%26%20람다%20%26%20클로저/03-Mutable_guide.md)
- [function_compose](./4.%20함수형%20%26%20람다%20%26%20클로저/06.function_compose.md)
- [function_pipeline](./4.%20함수형%20%26%20람다%20%26%20클로저/07.function_pipeline.md)
- [function_cury](./4.%20함수형%20%26%20람다%20%26%20클로저/08.function_cury.md)
- [function_programming](./4.%20함수형%20%26%20람다%20%26%20클로저/09.function_programming.md)
- [Generator](./4.%20함수형%20%26%20람다%20%26%20클로저/10-Generator.md)
- [Iterator_Generator](./4.%20함수형%20%26%20람다%20%26%20클로저/11-Iterator_Generator.md)
- [즉시실행_단계별_실행](./4.%20함수형%20%26%20람다%20%26%20클로저/12-즉시실행_단계별_실행.md)
- [on_closure_type_erasure](./4.%20함수형%20%26%20람다%20%26%20클로저/13-on_closure_type_erasure.md)
- [on_closure_class](./4.%20함수형%20%26%20람다%20%26%20클로저/14-on_closure_class.md)
  
## 5. 동시성 (Concurrency & Multithreading)
- [thread_basics](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/01-thread_basics.md)
- [multithreading_basic](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/02-multithreading_basic.md)
- [thread_member_function_and_args](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/03-thread_member_function_and_args.md)
- [thread_member_and_vector](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/04-thread_member_and_vector.md)
- [thread_interrupt_boost_vs_java](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/05-thread_interrupt_boost_vs_java.md)
- [CAS_explanation](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/06_01-CAS_explanation.md)
- [Atomic](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/07-Atomic.md)
- [thread_local_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/08-thread_local_guide.md)
- [mutex_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/09-mutex_guide.md)
- [Recursive_mutex_example](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/10-Recursive_mutex_example.md)
- [mutex_lock_examples](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/11-mutex_lock_examples.md)
- [thread_wait_notify_examples](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/12-thread_wait_notify_examples.md)
- [condition_variable_producer_consumer](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/13-condition_variable_producer_consumer.md)
- [promise_future_examples](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/14-promise_future_examples.md)
- [future_promise_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/15-future_promise_guide.md)
- [async_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/16-async_guide.md)
- [thread_pool_readme](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/17-thread_pool_readme.md)
- [single_scheduler_readme](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/18-single_scheduler_readme.md)
- [mutex_osyncstream_readme](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/19-mutex_osyncstream_readme.md)
- [qt_async_progress_readme](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/20-qt_async_progress_readme.md)
- [qt_eventloop_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/21-qt_eventloop_guide.md)
- [Chrono_timer_guide](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/22-Chrono_timer_guide.md)
- [coroutine](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/23.coroutine.md)
- [threadpool](./5.%20동시성%20(Concurrency%20%26%20Multithreading)/24-threadpool.md)
  
## 6. 예외와 오류 처리
- [exception_handling](./6.%20예외와%20오류%20처리/01-exception_handling.md)
- [noexcept_guide](./6.%20예외와%20오류%20처리/02-noexcept_guide.md)
- [예외클래스_정의](./6.%20예외와%20오류%20처리/03-예외클래스_정의.md)

## 7. 메모리 관리 & 시스템
- [onrealloc](./7.%20메모리%20관리%20%26%20시스템/01-onrealloc.md)
- [windows_long_path_utf8](./7.%20메모리%20관리%20%26%20시스템/02-windows_long_path_utf8.md)
- [MemoryManager](./7.%20메모리%20관리%20%26%20시스템/03-MemoryManager.md)
- [memory manager_guide](./7.%20메모리%20관리%20%26%20시스템/04-memory_manager_guide.md)
- [memory alignment](./7.%20메모리%20관리%20%26%20시스템/05.memory_alignment.md)
- [windows shared memory](./7.%20메모리%20관리%20%26%20시스템/06.windows_shared_memory.md)
- [linux shared_memory](./7.%20메모리%20관리%20%26%20시스템/07.linux_shared_memory.md)
- [unique_ptr](./7.%20메모리%20관리%20%26%20시스템/08.unique_ptr.md)
- [new placement deconstructor](./7.%20메모리%20관리%20%26%20시스템/09.new_placement_deconstructor.md)
- [bigobj의 역할](./7.%20메모리%20관리%20%26%20시스템/10-bigobj의_역할.md)

## 8. 입출력 & 유틸리티
- [Raw_string_literal](./8.%20입출력%20%26%20유틸리티/01-Raw_string_literal.md)
- [cout_formatting](./8.%20입출력%20%26%20유틸리티/02.cout_formatting.md)
- [stream기본](./8.%20입출력%20%26%20유틸리티/03.stream기본.md)
- [stream_random_access](./8.%20입출력%20%26%20유틸리티/04.stream_random_access.md)
- [string_parsing_cpp](./8.%20입출력%20%26%20유틸리티/05.string_parsing_cpp.md)
- [using](./8.%20입출력%20%26%20유틸리티/06.using.md)
- [unicode_cout](./8.%20입출력%20%26%20유틸리티/07.unicode_cout.md)
- [index_map](./8.%20입출력%20%26%20유틸리티/08.index_map.md)
- [crossplatform_io](./8.%20입출력%20%26%20유틸리티/09.crossplatform_io.md)
- [ofstream](./8.%20입출력%20%26%20유틸리티/10.ofstream.md)
- [programpath](./8.%20입출력%20%26%20유틸리티/11-programpath.md)
- [String_Utility](./8.%20입출력%20%26%20유틸리티/12-String_Utility.md)

## 9. 고급 주제 & 패턴
- [random](./9.%20고급%20주제%20%26%20패턴/01-random.md)
- [random_샘플](./9.%20고급%20주제%20%26%20패턴/02-random_샘플.md)
- [inline_function_overhead](./9.%20고급%20주제%20%26%20패턴/03-inline_function_overhead.md)
- [lvalue_rvalue-1](./9.%20고급%20주제%20%26%20패턴/04-lvalue_rvalue-1.md)
- [lvalue_rvalue-2](./9.%20고급%20주제%20%26%20패턴/05-lvalue_rvalue-2.md)
- [lvalue_rvalue-3](./9.%20고급%20주제%20%26%20패턴/06-lvalue_rvalue-3.md)
- [regular_expression](./9.%20고급%20주제%20%26%20패턴/07-regular_expression.md)
- [UML_Class_Guide](./9.%20고급%20주제%20%26%20패턴/08-UML_Class_Guide.md)
- [SOLID_OCP](./9.%20고급%20주제%20%26%20패턴/09-SOLID_OCP.md)
- [fmod](./9.%20고급%20주제%20%26%20패턴/10-fmod.md)
- [StringSwitch-1](./9.%20고급%20주제%20%26%20패턴/11-StringSwitch-1.md)
- [StringSwitch-2](./9.%20고급%20주제%20%26%20패턴/12-StringSwitch-2.md)
- [operator_likeas_array](./9.%20고급%20주제%20%26%20패턴/13-operator_likeas_array.md)
- [trun_function](./9.%20고급%20주제%20%26%20패턴/14-trun_function.md)
- [LinkedList_Study](./9.%20고급%20주제%20%26%20패턴/15-LinkedList_Study.md)
- [sizeof](./9.%20고급%20주제%20%26%20패턴/16-sizeof.md)
- [Process_Handling_with_Boost](./9.%20고급%20주제%20%26%20패턴/17-Process_Handling_with_Boost.md)
- [range_views_concept](./9.%20고급%20주제%20%26%20패턴/18-range_views_concept.md)
- [코드영역정리](./9.%20고급%20주제%20%26%20패턴/19-코드영역정리.md)
- [static_variable](./9.%20고급%20주제%20%26%20패턴/20-static_variable.md)
- [Decorator](./9.%20고급%20주제%20%26%20패턴/21-Decorator.md)
- [Regexpression_수량자](./9.%20고급%20주제%20%26%20패턴/22-Regexpression_수량자.md)

## 10. vcpkg로 테스트 통합
- [vcpkg 테스트 환경 조성](./01-vcpkg_테스트환경_조성.md)


## A. Design Pattern
- [Decoration Pattern](./01-DecoratonPattern.md)

## B. 실전 문제 적용

- [RTTR_Reflection](./B.%20실전%20문제%20적용/01-RTTR_Reflection.md)
- [ODE_Integration_Methods](./B.%20실전%20문제%20적용/02-ODE_Integration_Methods.md)
- [매크로기반_클래스자동등록](./B.%20실전%20문제%20적용/03-매크로기반_클래스자동등록.md)
- [BackgroundWorker](./B.%20실전%20문제%20적용/04-BackgroundWorker.md)
- [AutoDelete](./B.%20실전%20문제%20적용/05-AutoDelete.md)
- [06-k-d tree정리.md](./B.%20실전%20문제%20적용/06-k-d_tree정리.md)



---

