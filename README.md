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
- [0.-C++-기본과-객체지향-기초](#0.-C++-기본과-객체지향-기초)
- [1. Modern C++ 핵심 문법](
- [2. 템플릿 & 제네릭 프로그래밍](
- [3. STL & 컨테이너 & 알고리즘](
- [4. 함수형 & 람다 & 클로저](
- [5. 동시성 (Concurrency & Multithreading)](
- [6. 예외와 오류 처리](
- [7. 메모리 관리 & 시스템](
- [8. 입출력 & 유틸리티](
- [9. 고급 주제 & 패턴](
- [A. Design Pattern](
- [B. 실전 문제 적용](

## 0. C++ 기본과 객체지향 기초
- 00-is_a&has_a.md
- 01-생성자_기본_제거.md
- 22-01-Polymorphism_basic.md
- 22-02-copy_constructor_guide.md
- 22-03-virtual.md
- 22-04-final.md
- 22-05-delegate_constructor.md
- 22-06-pure_virtual_class.md
- 22-07-operator_overloading.md
- 22-08-operator_overloading_guide.md
- 22-09-virtual_destructor_guide.md
- 22-10.virtual Inheritance.md
- 22-12.const_mutable.md
- 22-13-01.friend.md
- 22-13-02.friend.md
- 22-13-03.단항연산자중복.md
- 22-13-04.단항연산자중복_심화.md
- 22-13-05.복사생성자.md
- 22-13-06.static_cast.md
- 22-13-07.dynamic_cast.md
- 22-13-08.가상함수_심화.md
- 22-13-09.가상함수_심화.md
- 22-13-10.대입연산자_심화.md
- 25-01.static.md
- 25-02.static_심화.md
- 75.namespace.md
- 76.enum_class.md
- 77-inline.md
## 1. Modern C++ 핵심 문법
- 02-decltype.md
- 26.auto_type.md
- 72.auto.md
- 44.constexpr.md
- 17-user_literal.md
- 18-01-modern_function.md
- 18-02-function_compare.md
- 19-tuple.md
- 20-initializer_list.md
- 21-optional.md
- 39.variant.md
- 62.nullptr.md

## 2. 템플릿 & 제네릭 프로그래밍
- [templates](./2.%20템플릿%20%26%20제네릭%20프로그래밍/01-templates.md)
- [Template_operator](./2.%20템플릿%20%26%20제네릭%20프로그래밍/02-Template_operator.md)
- [Variadic_arguments_guide](./2.%20템플릿%20%26%20제네릭%20프로그래밍/03-Variadic_arguments_guide.md)
- [template_map](./2.%20템플릿%20%26%20제네릭%20프로그래밍/04-template_map.md)
- [template_folding](./2.%20템플릿%20%26%20제네릭%20프로그래밍/05-template_folding.md)
- [template_sandbox](./2.%20템플릿%20%26%20제네릭%20프로그래밍/06-template_sandbox.md)
- [가변인자](./2.%20템플릿%20%26%20제네릭%20프로그래밍/07-가변인자.md)
- [template_본질](./2.%20템플릿%20%26%20제네릭%20프로그래밍/08-template_본질.md)

## 3. STL & 컨테이너 / 알고리즘
- 07-01-STL_overview.md
- 07-02-STL_container.md
- 07-03-Iterator_overview.md
- 07-04-STL_copy_if.md
- 07-05-Array_overview.md
- 07-06-Vector_overview.md
- 07-07-Algorithms_overview.md
- 07-08-For_each_example.md
- 07-09-Binary_search.md
- 07-10-Find_count_findif.md\
- 07-11-Sort.md
- 07-12-Deque_queue_overview.md
- 07-13-Manual_list_before_stl.md
- 07-14-Map_summary.md
- 10-vector_emplace_back_vs_push_back.md
- 30.swap.md
- 34.tarray_range_for.md
- 56.for_each.md
- 57.back_inserter.md
- 58.transform.md
- 73.stl_format.md
- 54.fast_list.md
## 4. 함수형 & 람다 / 클로저
- 08-01-Lambda_summary.md
- 08-02-Lambda_capture_guide.md
- 09-Mutable_guide.md
- 23-01-on_closure_readme.md
- 23-02-on_closure_readme.md
- 59-01.function_compose.md
- 59-02.function_pipeline.md
- 59-03.function_cury.md
- 60.function_programming.md
- 92-Generator.md
- 93-Iterator_Generator.md
- 94-즉시실행_단계별_실행.md
## 5. 동시성 (Concurrency / Multithreading)
- 12-Chrono_timer_guide.md
- 13-01-thread_basics.md
- 13-02-multithreading_basic.md
- 13-03-thread_member_function_and_args.md
- 13-04-thread_memeber_and_vector.md
- 13-05-thread_interrupt_boost_vs_java.md
- 13-06-Atomic.md
- 13-06_01-CAS_explanation.md
- 13-07-thread_local_guide.md
- 13-08-mutex_guide.md
- 13-09-Recursive_mutex_example.md
- 13-10-mutex_lock_examples.md
- 13-11-thread_wait_notify_examples.md
- 13-12-condition_variable_producer_consumer.md
- 13-13-promise_future_examples.md
- 13-14-future_promise_guide.md
- 13-15-async_guide.md
- 13-16-thread_pool_readme.md
- 13-17-single_scheduler_readme.md
- 13-18-mutex_osyncstream_readme.md
- 13-19-qt_async_progress_readme.md
- 13-20-qt_eventloop_guide.md
- 13-21-threadpool.md
- 70.coroutine.md
## 6. 예외와 오류 처리
- 14-01-exception_handling.md
- 14-02-noexcept_guide.md
- 24.예외클래스_정의.md

## 7. 메모리 관리 & 시스템
- 15-windows_long_path_utf8.md
- 16-01-MemoryManager.md
- 16-02-memory_manager_guide.md
- 28.unique_ptr.md
- 43.memory_alignment.md
- 52-01.linux_shared_memory.md
- 52-02.linux_shared_memory.md
- 65.new_placement_deconstructor.md
- 80-bigobj의 역할.md

## 8. 입출력 & 유틸리티
- 06-Raw_string_literal.md
- 29.cout_formatting.md
- 31-01.stream기본.md
- 32-02.stream_random_access.md
- 35.string_parsing_cpp.md
- 37.using.md
- 46.unicode_cout.md
- 61.crossplatform_io.md
- 69.ofstream.md
- 82-String_Utility.md
- 82-programpath.md
- 53.index_map.md

## 9. 고급 주제 & 패턴
- 33-01.random.md
- 33-02.random_샘플.md
- 36.inline_function_overhead.md
- 38-01.lvalue_rvalue.md
- 38-02.lvalue_rvalue.md
- 38-03.lvalue_rvalue.md
- 47.UML_Class_Guide.md
- 49.SOLID_OCP.md
- 50.RTTR_Reflection.md
- 51-01.StringSwitch.md
- 51-02.StringSwitch.md
- 63.operator_likeas_array.md
- 64.trun_function.md
- 66.LinkedList_Study.md
- 68.sizeof.md
- 71.Process_Handling_with_Boost.md
- 74.range_views_concept.md
- 79- 코드영역정리.md
- 81-static_variable.md
- 83-fmod.md
- 84-ODE_Integration_Methods.md
- 85-01-DecoratonPattern.md
- 86-매크로기반_클래스자동등록.md
- 87-BackgroundWorker.md
- 88-AutoDelete.md
- 90.k-d tree정리.md
- 91-Decorator.md
## 10. 디자인 패턴
## 11. 실전 문제 적용
