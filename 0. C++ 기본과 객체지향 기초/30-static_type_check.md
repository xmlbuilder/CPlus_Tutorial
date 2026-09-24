# C++ Compile-Time Type Relationship Utilities

C++ 타입 사이의 관계를 **컴파일 타임(compile
time)** 에 확인하기 위한 간단한 유틸리티 함수 모음이다.

- 주요 목적은 다음 세 가지이다.

    -   두 타입이 정확히 같은 타입인지 확인
    -   한 클래스가 다른 클래스의 기반(base) 타입인지 확인
    -   한 타입이 기반 클래스 자신이거나 그 파생 클래스인지 확인

이 함수들은 `<type_traits>`의 `std::is_same_v`, `std::is_base_of_v`를
읽기 쉬운 함수 형태로 감싼 것이다.

> **중요:** 이 함수들은 객체의 실제 런타임 타입(runtime type)을 검사하지
> 않는다.\
> `Base*`만 가지고 실제 객체가 `Derived`인지 알아내야 하는 경우에는
> `dynamic_cast` 또는 프로젝트의 RTTI 시스템을 사용해야 한다.

------------------------------------------------------------------------

## 1. 전체 소스

``` cpp
#pragma once

#include <type_traits>

// T1 and T2 are exactly the same C++ type.
template <typename T1, typename T2>
constexpr bool IsSameType()
{
    return std::is_same_v<T1, T2>;
}

// Derived inherits from Base.
// Note: std::is_base_of also returns true when Base and Derived are the same type.
template <typename Base, typename Derived>
constexpr bool IsBaseType()
{
    return std::is_base_of_v<Base, Derived>;
}

// Derived must really be a different class derived from Base.
// Base == Derived returns false.
template <typename Base, typename Derived>
constexpr bool IsDerivedType()
{
    return std::is_base_of_v<Base, Derived> &&
           !std::is_same_v<Base, Derived>;
}

// Type is Base itself or a class derived from Base.
template <typename Base, typename Type>
constexpr bool IsSameOrDerivedType()
{
    return std::is_same_v<Base, Type> ||
           std::is_base_of_v<Base, Type>;
}
```

------------------------------------------------------------------------

## 2. 기본 테스트 클래스

각 함수의 차이를 보기 위해 다음과 같은 간단한 클래스 계층을 사용한다.

``` cpp
class Object {};
class Geometry : public Object {};
class Curve : public Geometry {};
class NurbsCurve : public Curve {};
class Mesh {};
```

상속 관계는 다음과 같다.

``` text
Object
  ▲
  │
Geometry
  ▲
  │
Curve
  ▲
  │
NurbsCurve

Mesh     // 위 계층과 관계 없음
```

즉 `NurbsCurve`는 `Curve`, `Geometry`, `Object`의 파생 타입이며 `Mesh`는
이 계층과 관계가 없다.

------------------------------------------------------------------------

## 3. `IsSameType<T1, T2>()`

두 타입이 **정확히 동일한 C++ 타입** 인지 확인한다.

``` cpp
template <typename T1, typename T2>
constexpr bool IsSameType()
{
    return std::is_same_v<T1, T2>;
}
```

### 사용 예

``` cpp
static_assert(IsSameType<Curve, Curve>());
static_assert(!IsSameType<Curve, NurbsCurve>());
static_assert(!IsSameType<Object, Geometry>());
```

``` text
Curve ───────── Curve
       SAME
       true

Curve ───────── NurbsCurve
       DIFFERENT
       false
```

상속 관계가 있더라도 타입 자체가 다르면 `false`이다.

``` cpp
IsSameType<Curve, NurbsCurve>()   // false
```

따라서 이 함수는 **상속 관계를 검사하는 함수가 아니다.**

------------------------------------------------------------------------

## 4. `IsBaseType<Base, Derived>()`

`Derived`가 `Base`로부터 상속된 타입인지 확인한다.

``` cpp
template <typename Base, typename Derived>
constexpr bool IsBaseType()
{
    return std::is_base_of_v<Base, Derived>;
}
```

### 사용 예

``` cpp
static_assert(IsBaseType<Object, Geometry>());
static_assert(IsBaseType<Object, NurbsCurve>());
static_assert(IsBaseType<Curve, NurbsCurve>());

static_assert(!IsBaseType<NurbsCurve, Curve>());
static_assert(!IsBaseType<Object, Mesh>());
```

상속 계층 전체를 따라 검사할 수 있다.

``` text
Object
  ▲
  │
Geometry
  ▲
  │
Curve
  ▲
  │
NurbsCurve
```

따라서:

``` cpp
IsBaseType<Object, NurbsCurve>()    // true
IsBaseType<Geometry, NurbsCurve>()  // true
IsBaseType<Curve, NurbsCurve>()     // true
```

### 주의점

`std::is_base_of`는 같은 타입도 `true`로 판단한다.

``` cpp
IsBaseType<Curve, Curve>()   // true
```

즉 이 함수에서 `Base`라는 이름은 의미상 기반 타입을 나타내지만, **동일
타입도 허용된다.**

------------------------------------------------------------------------

## 5. `IsDerivedType<Base, Derived>()`

`IsBaseType()`과 비슷하지만 **Base와 Derived가 동일한 타입인 경우를
제외** 한다.

``` cpp
template <typename Base, typename Derived>
constexpr bool IsDerivedType()
{
    return std::is_base_of_v<Base, Derived> &&
           !std::is_same_v<Base, Derived>;
}
```

### 사용 예

``` cpp
static_assert(IsDerivedType<Object, Geometry>());
static_assert(IsDerivedType<Geometry, Curve>());
static_assert(IsDerivedType<Curve, NurbsCurve>());

static_assert(!IsDerivedType<Curve, Curve>());
static_assert(!IsDerivedType<Object, Mesh>());
```

| 검사 | `IsBaseType` | `IsDerivedType` |
|---|---:|---:|
| `<Curve, Curve>` | true | false |
| `<Curve, NurbsCurve>` | true | true |
| `<Object, NurbsCurve>` | true | true |
| `<Object, Mesh>` | false | false |

따라서 이름 그대로 **"정말 파생된 다른 타입인가?"** 를 검사할 때
사용한다.

------------------------------------------------------------------------

## 6. `IsSameOrDerivedType<Base, Type>()`

`Type`이 다음 둘 중 하나이면 `true`이다.

1.  `Base`와 정확히 같은 타입
2.  `Base`로부터 파생된 타입

``` cpp
template <typename Base, typename Type>
constexpr bool IsSameOrDerivedType()
{
    return std::is_same_v<Base, Type> ||
           std::is_base_of_v<Base, Type>;
}
```

### 사용 예

``` cpp
static_assert(IsSameOrDerivedType<Curve, Curve>());
static_assert(IsSameOrDerivedType<Curve, NurbsCurve>());

static_assert(!IsSameOrDerivedType<Curve, Geometry>());
static_assert(!IsSameOrDerivedType<Curve, Mesh>());
```

``` text
                 IsSameOrDerivedType<Curve, T>

Geometry                    false
   ▲
   │
Curve                       true   ← same
   ▲
   │
NurbsCurve                  true   ← derived

Mesh                        false
```

템플릿에서 특정 기반 타입 계열만 허용하고 싶을 때 유용하다.

``` cpp
template <typename T>
void ProcessCurve()
{
    static_assert(
        IsSameOrDerivedType<Curve, T>(),
        "T must be Curve or a class derived from Curve");

    // ...
}
```

------------------------------------------------------------------------

## 7. 네 함수의 차이

``` text
                        Object
                          ▲
                          │
                       Geometry
                          ▲
                          │
                        Curve
                          ▲
                          │
                     NurbsCurve
```

`Curve`를 기준으로 보면:

| 함수 | `<Curve, Curve>` | `<Curve, NurbsCurve>` | `<Curve, Geometry>` | `<Curve, Mesh>` |
|---|---:|---:|---:|---:|
| `IsSameType` | true | false | false | false |
| `IsBaseType` | true | true | false | false |
| `IsDerivedType` | false | true | false | false |
| `IsSameOrDerivedType` | true | true | false | false |

정리하면:

``` text
IsSameType
    └─ 정확히 같은 타입인가?

IsBaseType
    └─ Base가 Type의 기반 타입인가?
       동일 타입도 true

IsDerivedType
    └─ 실제로 다른 파생 클래스인가?
       동일 타입은 false

IsSameOrDerivedType
    └─ Base 자신 또는 Base 계열인가?
```

------------------------------------------------------------------------

## 8. 컴파일 타임 검사

이 유틸리티의 중요한 특징은 모두 `constexpr`이라는 점이다.

따라서 다음처럼 `static_assert`에 사용할 수 있다.

``` cpp
static_assert(IsSameType<int, int>());

static_assert(IsBaseType<Object, Geometry>());
static_assert(IsDerivedType<Object, Geometry>());

static_assert(IsSameOrDerivedType<Geometry, Geometry>());
static_assert(IsSameOrDerivedType<Geometry, NurbsCurve>());
```

조건이 맞지 않으면 프로그램 실행 중에 실패하는 것이 아니라 **컴파일
단계에서 오류를 검출** 할 수 있다.

예:

``` cpp
template <typename T>
void AddGeometry(T* geometry)
{
    static_assert(
        IsSameOrDerivedType<Geometry, T>(),
        "T must be Geometry or derived from Geometry");

    // ...
}
```

------------------------------------------------------------------------

## 9. 런타임 타입 검사와의 차이

이 함수들이 가장 혼동되기 쉬운 부분이다.

``` cpp
Object* object = new NurbsCurve;
```

실제 객체는 `NurbsCurve`이지만 변수의 정적 타입은 `Object*`이다.

`IsSameType`, `IsBaseType`, `IsDerivedType`, `IsSameOrDerivedType`은
실제 객체를 검사하는 것이 아니라 **컴파일러가 알고 있는 타입 정의 사이의
관계** 를 검사한다.

실제 객체가 `NurbsCurve`인지 확인하려면 런타임 타입 검사가 필요하다.

``` cpp
Object* object = new NurbsCurve;

NurbsCurve* curve =
    dynamic_cast<NurbsCurve*>(object);

if (curve)
{
    // 실제 객체가 NurbsCurve 계열
}
```

프로젝트 자체 RTTI가 있다면 같은 역할을 프로젝트의 `Cast()` 계열 함수로
수행할 수도 있다.

``` text
Compile Time
──────────────────────────────────

IsSameType<A, B>()
IsBaseType<Base, Derived>()
IsDerivedType<Base, Derived>()
IsSameOrDerivedType<Base, T>()

        ↓

타입 정의 사이의 관계 검사


Runtime
──────────────────────────────────

Object* p
   │
   ▼
실제 객체가 무엇인가?

        ↓

dynamic_cast
또는 프로젝트 RTTI / Cast()
```

따라서 두 용도는 분리해서 사용해야 한다.

------------------------------------------------------------------------

## 10. CAD 코드에서의 사용 예

CAD 커널에서는 특정 `Geometry` 계열만 허용하는 템플릿 API에서 사용할 수
있다.

``` cpp
template <typename T>
void RegisterGeometryType()
{
    static_assert(
        IsSameOrDerivedType<Geometry, T>(),
        "T must be a Geometry type");

    // register...
}
```

또한 기반 타입과 실제 파생 타입을 구별해야 하는 템플릿 로직에서는:

``` cpp
if constexpr (IsDerivedType<Geometry, T>())
{
    // Geometry 자신이 아닌 실제 파생 타입
}
```

처럼 사용할 수 있다.

------------------------------------------------------------------------

## 11. 요약

이 유틸리티는 **런타임 RTTI를 대체하기 위한 코드가 아니라 템플릿 및
컴파일 타임 타입 검증을 읽기 쉽게 만들기 위한 코드** 이다.

``` text
IsSameType<T1, T2>
        │
        └─ 두 타입이 정확히 같은가?


IsBaseType<Base, Derived>
        │
        └─ Base가 기반 클래스인가?
           동일 타입 포함


IsDerivedType<Base, Derived>
        │
        └─ Derived가 Base의 실제 파생 클래스인가?
           동일 타입 제외


IsSameOrDerivedType<Base, Type>
        │
        └─ Base 자신이거나 Base에서 파생되었는가?
```

특히 CAD 커널처럼 `Object → Geometry → Curve/Surface/...` 형태의 타입
계층과 템플릿 코드가 많은 프로젝트에서는 잘못된 타입 사용을 컴파일
단계에서 제한하는 용도로 사용할 수 있다.

---

