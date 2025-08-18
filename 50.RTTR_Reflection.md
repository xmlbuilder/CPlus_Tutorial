# 📘 RTTR (Run Time Type Reflection) 정리

## 🔹 개요

-   **RTTR**은 C++에서 **런타임 리플렉션(Runtime Reflection)** 기능을
    제공하는 라이브러리입니다.
-   Java, C#처럼 런타임에 클래스, 메서드, 속성을 다루는 기능을 C++에
    도입하기 위해 만들어졌습니다.
-   직렬화, 스크립팅 바인딩, 플러그인 시스템, UI 자동 생성 등 다양한
    응용이 가능합니다.

------------------------------------------------------------------------

## 🔹 주요 기능

### 1. 타입 정보 조회

``` cpp
rttr::type t = rttr::type::get<MyStruct>();
std::cout << t.get_name() << std::endl; // "MyStruct"
```

### 2. 속성 접근 (Property Access)

``` cpp
MyStruct obj;
rttr::property prop = rttr::type::get(obj).get_property("data");
prop.set_value(obj, 23);
std::cout << prop.get_value(obj).to_int(); // 23
```

### 3. 메서드 호출 (Method Invocation)

``` cpp
MyStruct obj;
rttr::method meth = rttr::type::get(obj).get_method("func");
meth.invoke(obj, 42.0); // "data added 42"
```

### 4. 생성자 호출 (Constructor Invocation)

``` cpp
rttr::type t = rttr::type::get_by_name("MyStruct");
rttr::variant var = t.create(); // 기본 생성자 호출
```

### 5. 동적 타입 탐색

``` cpp
for (auto& prop : t.get_properties())
    std::cout << "Property: " << prop.get_name() << std::endl;
for (auto& meth : t.get_methods())
    std::cout << "Method: " << meth.get_name() << std::endl;
```

------------------------------------------------------------------------

## 🔹 장점

-   **직관적인 API**: `property`, `method`, `constructor` 객체 기반으로
    접근 가능
-   **템플릿 기반 등록**: 타입 안전성과 C++ 문법을 활용한 등록 시스템
-   **Cross-Platform**: Windows, Linux, macOS 지원
-   **확장성**: 사용자 정의 타입도 손쉽게 등록 가능
-   **성숙한 프로젝트**: 활발한 업데이트와 다양한 예제가 제공됨

------------------------------------------------------------------------

## 🔹 한계

-   C++ 표준에 없는 기능이므로, **전처리 등록**(`RTTR_REGISTRATION`)을
    반드시 해줘야 함
-   리플렉션 자체가 런타임 비용이 있으므로 **실시간 성능이 중요한
    코드에는 주의**
-   C++20의 `constexpr` / `concepts` 기반 메타프로그래밍과는 방향이 다름

------------------------------------------------------------------------

## 🔹 추천 활용 방향

-   **직렬화/역직렬화 (JSON, XML, Binary)**\
    구조체 속성을 자동으로 탐색해서 저장/복원 가능
-   **플러그인 시스템**\
    클래스 이름만으로 동적 객체 생성 및 메서드 호출 가능
-   **스크립트 바인딩 (Python, Lua 등)**\
    리플렉션 기반으로 C++ 코드를 스크립트에 자동 노출 가능
-   **GUI 자동화**\
    속성 목록을 읽어 자동 UI 위젯 생성 가능 (예: Unity Editor의 인스펙터
    기능처럼)

------------------------------------------------------------------------

## 🔹 CAD/엔진 개발 적용 예시

-   **TrimmedSurface 속성 관리**: 속성 기반 자동 직렬화/디버깅\
-   **BRep 데이터 직렬화**: JSON/XML 변환\
-   **에디터 툴**: 객체 속성을 UI에 자동 바인딩\
-   **플러그인 확장**: 런타임에 새로운 Geometry 클래스 로딩 가능

------------------------------------------------------------------------

## 🔹 실제 사용 예제
```cpp
#include <iostream>
#include <rttr/registration>
using namespace rttr;

struct MyStruct {
 
      MyStruct() {};

    void func(double val) {
        std::cout << "data added " << val << std::endl;
    };

    int fun2() {
        return 34;
    }

    int data = 12;
};


RTTR_REGISTRATION
{
    registration::class_<MyStruct>("MyStruct")
            .constructor<>()
            .property("data", &MyStruct::data)
            .method("func", &MyStruct::func)
            .method("fun2", rttr::select_overload<int(void)>(&MyStruct::fun2));
}

int main() {
    {
        type t = type::get<MyStruct>();
        for (auto& prop : t.get_properties())
            std::cout << "name: " << prop.get_name() << std::endl;
        for (auto& meth : t.get_methods())
            std::cout << "name: " << meth.get_name() << std::endl;
    }

    {
        type t = type::get_by_name("MyStruct");
        variant var = t.create();    // will invoke the previously registered ctor

        std::cout << t.get_property_value("data", var).to_int() << std::endl; // prints "12"
        t.set_property_value("data", var, 30);
        std::cout << t.get_property_value("data", var).to_int() << std::endl; // prints "12"
        {
            method method = t.get_method("func");
            method.invoke(var, 42.0);
        }
        constructor ctor = t.get_constructor();  // 2nd way with the constructor class
        var = ctor.invoke();
        std::cout << var.get_type().get_name() << std::endl;// prints 'MyStruct'
        {
            method method2 = t.get_method("fun2");
            int ret = method2.invoke(var).to_int();
            std::cout << "Return Value : " << ret << std::endl;
        }
    }

    return 0;
}


//Set/get properties
MyStruct obj;
property prop = type::get(obj).get_property("data");
prop.set_value(obj, 23);
variant var_prop = prop.get_value(obj);
std::cout << var_prop.to_int(); // prints '23'


//Invoke Methods:
MyStruct obj;
method meth = type::get(obj).get_method("func");
meth.invoke(obj, 42.0);
variant var = type::get(obj).create();
meth.invoke(var, 42.0);

```

## 📝 결론

-   RTTR은 **C++에서 가장 완성도 높은 런타임 리플렉션 라이브러리 중
    하나**입니다.
-   CAD 엔진 같은 시스템을 개발하는 경우, **데이터 직렬화·플러그인
    시스템·스크립팅 바인딩**에서 큰 도움이 됩니다.
-   다만 성능 민감 구간에서는 직접 메타프로그래밍(C++20 Concepts, TMP)
    기반 접근을 병행하는 것이 좋습니다.
