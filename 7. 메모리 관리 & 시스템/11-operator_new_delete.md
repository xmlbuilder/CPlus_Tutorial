# `operator new` / `operator delete`

## 1. 일반 `new`

``` cpp
class Test
{
public:
    Test()
    {
        std::cout << "constructor\n";
    }
};

int main()
{
    Test* p = new Test();
    delete p;
}
```

### 실제 동작 개념:

``` cpp
void* memory = operator new(sizeof(Test));
Test* p = new(memory) Test();
```

#### 동작 순서:

1.  메모리 할당
2.  생성자 호출
3.  객체 사용
4.  소멸자 호출
5.  메모리 해제

------------------------------------------------------------------------

## 2. 사용자 정의 `operator new`

``` cpp
class Test
{
public:

    static void* operator new(std::size_t size)
    {
        std::cout<<"custom new\n";
        return std::malloc(size);
    }

    static void operator delete(void* p)
    {
        std::free(p);
    }

    Test()
    {
        std::cout<<"constructor\n";
    }
};
```

### 호출:

``` cpp
Test* p = new Test();
```

### 컴파일러 내부 개념:

``` cpp
void* memory =
    Test::operator new(sizeof(Test));

Test* p =
    new(memory) Test();
```

------------------------------------------------------------------------

## 3. 추가 인자를 받는 `operator new`

``` cpp
struct MyContext
{
    const char* name;
};

class Test
{
public:

    static void* operator new(
        std::size_t size,
        MyContext* ctx)
    {
        std::cout<<"context="
                 <<ctx->name
                 <<std::endl;

        return std::malloc(size);
    }

    static void operator delete(void* p)
    {
        std::free(p);
    }

    Test(int value)
    {
        std::cout
            <<"value="
            <<value
            <<std::endl;
    }
};
```

### 사용:

``` cpp
MyContext ctx{"Main"};

Test* p =
    new(&ctx) Test(123);
```

### 실제 의미:

``` cpp
void* memory =
    Test::operator new(
        sizeof(Test),
        &ctx);

Test* p =
    new(memory)
    Test(123);
```

------------------------------------------------------------------------

## 가장 중요한 공식

### 형식:

``` cpp
new(A) Type(B)
```

### 의미:

``` text
A → operator new 로 전달
B → 생성자로 전달
```

### 예:

``` cpp
new(&context)
IwBSplineSurface(oldSurface)
```

### 실제:

``` cpp
IwObject::operator new(
    sizeof(IwBSplineSurface),
    &context);

IwBSplineSurface(
    oldSurface);
```

------------------------------------------------------------------------

## operator delete도 같이 만드는 이유

### 나쁜 예:

``` cpp
class Test
{
public:

    static void* operator new(
        std::size_t size)
    {
        return std::malloc(size);
    }
};
```

### 문제:

``` text
new → malloc 사용
delete → 기본 delete 사용
```

- 메모리 관리 방식이 섞일 수 있음.

### 좋은 예:

``` cpp
class Test
{
public:

    static void* operator new(
        std::size_t size)
    {
        return std::malloc(size);
    }

    static void operator delete(
        void* p)
    {
        std::free(p);
    }
};
```

------------------------------------------------------------------------

## 추가 인자 operator delete

``` cpp
class Test
{
public:

    static void* operator new(
        size_t size,
        MyContext* ctx)
    {
        return std::malloc(size);
    }

    static void operator delete(
        void* p,
        MyContext*)
    {
        std::free(p);
    }

    static void operator delete(
        void* p)
    {
        std::free(p);
    }
};
```

### 이유:

- 생성자 중 예외가 발생하면:

``` cpp
Test* p =
    new(&ctx) Test();
```

- 컴파일러는:

``` cpp
operator delete(
    ptr,
    &ctx);
```

- 를 호출할 수 있음.

------------------------------------------------------------------------

## CAD 코드 해석

``` cpp
IwBSplineSurface* p =
new(GetContext())
IwBSplineSurface(
    *(IwBSplineSurface*)
    GetSurface());
```

### 해석:

``` text
GetContext()
    → operator new(size, GetContext())

GetSurface()
    → IwBSplineSurface 생성자 인자
```

### 실제:

``` cpp
void* memory=
IwObject::operator new(
sizeof(IwBSplineSurface),
GetContext());

IwBSplineSurface(
*(IwBSplineSurface*)
GetSurface());
```

------------------------------------------------------------------------

## 실무 기억법

``` text
operator new 만들면
operator delete도 같이 만든다.

operator new(size,extra...)
만들면

operator delete(void*,extra...)
도 같이 만든다.
```
---
