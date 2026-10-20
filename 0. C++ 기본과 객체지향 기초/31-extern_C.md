## 📘 C++ `extern "C"` 정리

### 📌 1. 목적

- `extern "C"`는 **C++ 컴파일러에 C 언어 링키지(language linkage)를 지정** 하는 문법이다.
- 주된 용도는 C++ 함수의 이름 장식(name mangling)을 피하고 C 또는 C# P/Invoke 등에서 접근하기 쉬운 인터페이스를 제공하는 것이다.

> `extern "C"`는 코드를 C 언어로 컴파일하거나, 모든 ABI·메모리 호환성을 보장하는 기능이 아니다.

### 📌 2. C와 C++의 차이

#### 🔹 C 컴파일러 (`.c`)

```c
__declspec(dllexport)
int Add(int a, int b) { return a + b; }
```

- C 컴파일러는 기본적으로 C 링키지를 사용하므로 `extern "C"`가 필요 없다.
- 일반적인 MSVC x64 DLL에서는 `Add`라는 이름으로 내보낸다.

#### 🔹 C++ 컴파일러 (`.cpp`)

```cpp
__declspec(dllexport)
int Add(int a, int b) { return a + b; }
```
- C++은 함수 오버로딩 등을 지원하기 위해 이름을 장식한다.
- 예를 들어 MSVC x64에서는 `?Add@@YAHHH@Z`와 같은 심볼이 될 수 있다.

```cpp
extern "C" __declspec(dllexport)
int Add(int a, int b) { return a + b; }
```

- `extern "C"`를 지정하면 C++식 이름 장식을 적용하지 않는다.
- 일반적인 MSVC x64 DLL에서는 `Add`로 내보낸다.

| 항목 | C 컴파일러 | C++ 컴파일러 |
|---|---|---|
| 기본 링키지 | C | C++ |
| `extern "C"` | 불필요하며 C 문법 아님 | 필요할 때 지정 |
| C++식 이름 장식 | 없음 | 기본적으로 있음 |
| 함수 오버로딩 | 불가능 | 가능 (`extern "C"` 함수끼리는 불가능) |

### 📌 3. 헤더에서 C/C++ 동시 지원

```cpp
// math_api.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int Add(int a, int b);

#ifdef __cplusplus
}
#endif
```

- `__cplusplus`는 C++ 컴파일 시 정의된다. 따라서 같은 헤더를 C와 C++ 소스에서 모두 포함할 수 있다.
- 함수 구현은 선언과 동일한 링키지를 따라야 한다.

### 📌 4. C#에서 DLL 호출

#### 🔹 C++ DLL:

```cpp
extern "C" __declspec(dllexport)
int __cdecl Add(int a, int b)
{
    return a + b;
}
```

#### 🔹 C#:

```csharp
using System.Runtime.InteropServices;

static class NativeMethods
{
    [DllImport("MathLib.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int Add(int a, int b);
}
```

- C++의 호출 규약과 C#의 `CallingConvention`을 일치시켜야 한다.

### 📌 5. C++ 클래스는 불투명 핸들로 노출

- C++ 클래스의 멤버 함수를 그대로 C 인터페이스로 공개하는 대신 일반 C 링키지 함수를 제공한다.

```cpp
class Mesh {
public:
    void AddNode(double x, double y, double z);
};

extern "C" {
    __declspec(dllexport) void* __cdecl Mesh_Create()
    {
        try { return new Mesh(); }
        catch (...) { return nullptr; }
    }

    __declspec(dllexport) void __cdecl Mesh_Destroy(void* handle)
    {
        delete static_cast<Mesh*>(handle);
    }

    __declspec(dllexport) int __cdecl Mesh_AddNode(
        void* handle, double x, double y, double z)
    {
        if (!handle) return 0;
        try {
            static_cast<Mesh*>(handle)->AddNode(x, y, z);
            return 1;
        } catch (...) {
            return 0;
        }
    }
}
```

- C#은 `void*`에 대응하는 `IntPtr`로 핸들을 보관하고 `Mesh_Destroy`로 해제한다.
- 실서비스에서는 생성 실패·잘못된 핸들·예외 상황에 대한 계약을 별도로 정의해야 한다.

#### 🔹 1. C++과 C#의 대응 관계

| C++	| C#	| 설명 |
|-----|-----|------|
| void*	| IntPtr	| C++ 객체 주소 |
| double	| double	| 64비트 실수 |
| int	| int	| 32비트 정수 |
| __cdecl	| CallingConvention.Cdecl	| 호출 규약 |
| Mesh_Create()	| Mesh_Create()	| 객체 생성 |
| Mesh_Destroy()	| Mesh_Destroy()	| 객체 삭제 |
| Mesh_AddNode()	| Mesh_AddNode()	| 객체의 멤버 함수 호출 |

#### 🔹 2. C#에서 DLL 함수 선언

- C++ 코드를 MeshLib.dll로 빌드했다고 가정.
```csharp
using System;
using System.Runtime.InteropServices;

public static class NativeMesh
{
    private const string DllName = "MeshLib.dll";

    [DllImport(DllName,
        CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr Mesh_Create();

    [DllImport(DllName,
        CallingConvention = CallingConvention.Cdecl)]
    public static extern void Mesh_Destroy(IntPtr handle);

    [DllImport(DllName,
        CallingConvention = CallingConvention.Cdecl)]
    public static extern int Mesh_AddNode(
        IntPtr handle,
        double x,
        double y,
        double z);
}
```

#### 🔹 3. C#에서 Mesh 생성 및 사용
```csharp
using System;

class Program
{
    static void Main()
    {
        // 1. C++ Mesh 객체 생성
        IntPtr mesh = NativeMesh.Mesh_Create();

        if (mesh == IntPtr.Zero)
        {
            Console.WriteLine("Mesh 생성 실패");
            return;
        }

        try
        {
            // 2. C++ Mesh에 Node 추가
            int result1 = NativeMesh.Mesh_AddNode(
                mesh, 1.0, 2.0, 3.0);

            int result2 = NativeMesh.Mesh_AddNode(
                mesh, 4.0, 5.0, 6.0);

            if (result1 == 1 && result2 == 1)
            {
                Console.WriteLine("Node 추가 성공");
            }
            else
            {
                Console.WriteLine("Node 추가 실패");
            }
        }
        finally
        {
            // 3. C++ Mesh 객체 삭제
            NativeMesh.Mesh_Destroy(mesh);
        }
    }
}
```
#### 🔹실행 흐름

```
C#                              C++
───────────────────────────────────────────

Mesh_Create() ────────────────→ new Mesh()
                                     │
IntPtr mesh  ←───────────────── Mesh*
     │
     │
Mesh_AddNode(mesh, 1,2,3)
     │
     └────────────────────────→ mesh->AddNode()
                                     │
int result   ←───────────────────────┘
     │
     │
Mesh_Destroy(mesh)
     │
     └────────────────────────→ delete mesh
```
- C#에서는 실제 Mesh 클래스의 구조를 몰라도 됩니다.
  
#### 🔹 4. C# 클래스로 감싸서 사용하는 방법

- 실제로 GUI 프로그램을 개발한다면 IntPtr을 직접 관리하는 것보다 C# 클래스로 감싸는 편이 편리합니다.
- 예를 들어 다음과 같이 구현할 수 있습니다.

##### MeshWrapper.cs
```csharp
using System;

public sealed class MeshWrapper : IDisposable
{
    private IntPtr handle;
    private bool disposed;

    public MeshWrapper()
    {
        handle = NativeMesh.Mesh_Create();

        if (handle == IntPtr.Zero)
        {
            throw new InvalidOperationException(
                "C++ Mesh 생성 실패");
        }
    }

    public void AddNode(double x, double y, double z)
    {
        if (disposed)
        {
            throw new ObjectDisposedException(
                nameof(MeshWrapper));
        }

        int result = NativeMesh.Mesh_AddNode(
            handle, x, y, z);

        if (result == 0)
        {
            throw new InvalidOperationException(
                "C++ Mesh_AddNode 실패");
        }
    }

    public void Dispose()
    {
        if (!disposed)
        {
            if (handle != IntPtr.Zero)
            {
                NativeMesh.Mesh_Destroy(handle);
                handle = IntPtr.Zero;
            }

            disposed = true;
        }

        GC.SuppressFinalize(this);
    }
}
```
#### 🔹 사용 방법
```csharp
using (var mesh = new MeshWrapper())
{
    mesh.AddNode(1.0, 2.0, 3.0);
    mesh.AddNode(4.0, 5.0, 6.0);
    mesh.AddNode(7.0, 8.0, 9.0);
}
```
- 이렇게 작성하면 C#에서 일반적인 객체를 사용하는 것과 거의 같습니다.
- using 블록을 벗어나면 Dispose()가 호출되고 C++의 Mesh_Destroy()가 실행됩니다.
- 단, 위 클래스는 명시적인 Dispose()를 통한 해제를 전제로 한 간단한 구현입니다.


### 📌 6. 주의할 점

- **이름 장식:** `extern "C"`는 C++식 이름 장식을 억제하지만, x86의 `_Add`, `_Add@8`처럼 호출 규약에 따른 장식은 남을 수 있다.
- **호출 규약:** `extern "C"`와 `__cdecl`/`__stdcall`은 별개의 개념이다.
- **자료형:** `std::vector`, `std::string`, C++ 클래스 레이아웃을 다른 언어에 그대로 노출하지 않는 것이 좋다. 정수, 실수, C 호환 구조체, 포인터+길이 등을 사용한다.
- **메모리 소유권:** C++에서 생성한 객체는 해당 라이브러리가 제공하는 해제 함수로 정리한다.
- **예외:** C++ 예외가 C ABI 경계를 넘어가지 않도록 함수 내부에서 처리하고 오류 코드로 전달한다.
- **성능:** `extern "C"` 자체가 실행 속도를 높이거나 낮추지는 않는다.

### 📌 7. 실제 DLL 심볼 확인
- Visual Studio 개발자 명령 프롬프트에서:

```cmd
dumpbin /exports MathLib.dll
```
- DLL에 실제로 공개된 이름을 확인할 수 있다. 심볼 이름이 고정되어야 한다면 `.def` 파일을 통한 export 이름 제어도 고려할 수 있다.

### 📌 요약

- **C 컴파일러는 기본적으로 C 링키지를 사용한다. C++ 컴파일러에서 C 호환 함수 인터페이스를 공개할 때 `extern "C"`를 사용한다.**
- 다만 호출 규약, 데이터 레이아웃, 예외 처리, 메모리 소유권은 별도로 설계해야 한다.

---

