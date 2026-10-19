## 📘 C# ↔ C++ 데이터 연동 가이드

- C# GUI와 C++ CAD/FEM 계산 라이브러리를 연결할 때 참고하는 실무 요약.

### 📌 1. 권장 구조

```text
C# GUI (WPF / WinForms)
    │ P/Invoke: 값, 배열 포인터, 객체 핸들
    ▼
C ABI 인터페이스 DLL (extern "C")
    │ 입력 검증 / 예외 차단 / 메모리 수명 관리
    ▼
C++ CAD Kernel (Mesh, NURBS, B-Rep, FEM)
```

- **단순 데이터**: 양쪽의 메모리 레이아웃을 맞춰 전달한다.
- **복잡한 C++ 객체**: C++이 소유하고 C#에는 불투명 핸들(`IntPtr`/`SafeHandle`)만 전달한다.
- **대용량 배열**: 복사를 줄이기 위해 포인터와 원소 수를 함께 전달한다.

### 📌 2. `extern "C"`와 P/Invoke

- `extern "C"`는 C++ 함수에 C 언어 링키지를 적용한다.
- C++ 이름 장식(name mangling)을 방지하지만, **구조체 레이아웃·호출 규약·메모리 수명·예외 처리까지 자동으로 맞춰 주지는 않는다.**

#### 🔹 C++ (MSVC DLL)

```cpp
struct Point3D { double x, y, z; };

extern "C" __declspec(dllexport)
void __cdecl TranslatePoints(Point3D* points, int count,
                             double dx, double dy, double dz)
{
    if (!points || count < 0) return;
    for (int i = 0; i < count; ++i) {
        points[i].x += dx;
        points[i].y += dy;
        points[i].z += dz;
    }
}
```

#### 🔹 C#

```csharp
using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct Point3D
{
    public double X, Y, Z;
}

public static class NativeMethods
{
    [DllImport("Geometry.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern unsafe void TranslatePoints(
        Point3D* points, int count, double dx, double dy, double dz);
}

Point3D[] points = new Point3D[1000];
unsafe
{
    fixed (Point3D* p = points)
    {
        NativeMethods.TranslatePoints(p, points.Length, 10, 20, 30);
    }
}
```

- `fixed`는 호출 중 GC가 배열을 이동시키지 못하도록 고정한다.
- **C++이 포인터를 호출 이후까지 저장하면 안 된다.** 위 함수는 전달된 배열을 제자리에서 수정하며 추가 배열 복사가 필요 없다.

### 📌 3. 구조체 레이아웃과 Marshal

#### 🔹 3.1 기본 자료형

| C++ | C# | 주의 |
|---|---|---|
| `int32_t` | `int` | 4바이트 |
| `uint32_t` | `uint` | 4바이트 |
| `double` | `double` | 8바이트 |
| `float` | `float` | 4바이트 |
| `uint8_t` | `byte` | 1바이트 |
| `void*` | `IntPtr` | 포인터 크기(x86/x64) |
| `bool` | `[MarshalAs(UnmanagedType.I1)] bool` | C++ `bool`은 보통 1바이트, 기본 P/Invoke `bool`은 Win32 `BOOL`(4바이트) |

- `bool`이 포함된 C# 구조체는 일반적으로 blittable이 아니다.
- **포인터로 직접 공유할 데이터는 C++ `uint8_t` ↔ C# `byte`로 명시** 하는 편이 단순하다.

#### 🔹 3.2 패딩과 정렬

##### C++

```cpp
struct Node { int32_t id; double x, y, z; };
// 일반적인 MSVC 기본 정렬: id=0, x=8, y=16, z=24, sizeof=32
```

##### C#

```csharp
[StructLayout(LayoutKind.Sequential)]
public struct Node
{
    public int Id;
    public double X, Y, Z;
}
```
- C++에서 `#pragma pack(push, 1)`을 사용한 구조체라면 C#에서도 `[StructLayout(LayoutKind.Sequential, Pack = 1)]`로 맞춘다.
- **무조건 `Pack = 1`로 설정하지 않는다.**

##### 검증:

```cpp
#include <cstddef>
static_assert(sizeof(Node) == 32);
static_assert(offsetof(Node, x) == 8);
```

```csharp
Console.WriteLine(Marshal.SizeOf<Node>());
Console.WriteLine(Marshal.OffsetOf<Node>(nameof(Node.X)));
```

- C++에서는 #pragma pack을 사용하는 경우가 있습니다.

##### C++
```cpp
#pragma pack(push, 1)

struct Node
{
    int id;
    double x;
    double y;
    double z;
};

#pragma pack(pop)
```
- 이 구조체는 28바이트입니다.
```
Offset    Field
---------------
0         id
4         x
12        y
20        z
---------------
Size = 28
```
- C#에서는 Pack을 동일하게 지정해야 합니다.
```csharp
[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Node
{
    public int Id;
    public double X;
    public double Y;
    public double Z;
}
```
- 반대로 C++에서 기본 정렬을 사용한다면 일반적인 MSVC x64 환경에서는 다음과 같습니다.
```
Offset    Field
---------------
0         id
4         padding
8         x
16        y
24        z
---------------
Size = 32
```
- 이때 C#에 Pack = 1을 지정하면 오히려 잘못된 구조가 됩니다.
- 중요한 것은 C++과 C#의 구조체 크기뿐 아니라 각 필드의 Offset까지 일치해야 한다는 점입니다.


#### 🔹 3.3 고정 크기 배열

**C++**

```cpp
struct Element { int32_t id; int32_t nodes[4]; };
```

**C# (포인터 직접 공유용)**

```csharp
[StructLayout(LayoutKind.Sequential)]
public unsafe struct Element
{
    public int Id;
    public fixed int Nodes[4];
}


unsafe
{
    Element element = new Element();

    element.Id = 10;
    element.Nodes[0] = 100;
    element.Nodes[1] = 101;
    element.Nodes[2] = 102;
    element.Nodes[3] = 103;
}
```
`[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] int[]`도 네이티브 고정 배열과의 마샬링에 사용할 수 있지만, 관리 배열 참조가 들어 있으므로 `Element*`로 직접 공유하는 형태에는 맞지 않는다.



### 📌 4. 문자열과 포인터 필드

- 문자열 인코딩은 반드시 양쪽에서 명시한다.
- UTF-8을 사용한다면 C++ `const char*`와 C#의 `[MarshalAs(UnmanagedType.LPUTF8Str)] string`을 **입력 인수** 로 매핑할 수 있다.
- C++이 문자열 포인터를 저장하면 안 되며, 반환 문자열의 소유권/해제 방식은 별도로 정의해야 한다.

```cpp
struct MeshData {
    int32_t nodeCount;
    const double* coordinates;
    int32_t elementCount;
    const int32_t* connectivity;
};
```

```csharp
[StructLayout(LayoutKind.Sequential)]
public struct MeshData
{
    public int NodeCount;
    public IntPtr Coordinates;
    public int ElementCount;
    public IntPtr Connectivity;
}
```

- `IntPtr`은 주소를 표현할 뿐 메모리를 소유하지 않는다. 포인터가 유효한 동안에만 읽어야 하며, C++ 객체가 재할당되면 이전 포인터는 무효화될 수 있다.

### 📌 5. C++ 객체는 핸들로 전달

#### 🔹 C++

```cpp
extern "C" __declspec(dllexport)
void* __cdecl Mesh_Create() noexcept
{
    try { return new Mesh(); }
    catch (...) { return nullptr; }
}

extern "C" __declspec(dllexport)
void __cdecl Mesh_Destroy(void* handle) noexcept
{
    delete static_cast<Mesh*>(handle);
}

extern "C" __declspec(dllexport)
int __cdecl Mesh_GetNodeCount(void* handle) noexcept
{
    try {
        if (!handle) return -1;
        return static_cast<Mesh*>(handle)->GetNodeCount();
    } catch (...) { return -1; }
}
```

#### 🔹 C#

```csharp
[DllImport("Geometry.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern IntPtr Mesh_Create();

[DllImport("Geometry.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern void Mesh_Destroy(IntPtr handle);

[DllImport("Geometry.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern int Mesh_GetNodeCount(IntPtr handle);

IntPtr mesh = Mesh_Create();
if (mesh == IntPtr.Zero) throw new InvalidOperationException("Mesh creation failed");
try
{
    int count = Mesh_GetNodeCount(mesh);
    if (count < 0) throw new InvalidOperationException("Native mesh error");
}
finally
{
    Mesh_Destroy(mesh);
}
```

- 실제 서비스에서는 `SafeHandle`로 감싸 이중 해제·누락을 줄이는 편이 좋다.
- C++ 예외는 P/Invoke 경계를 넘어 전파하지 말고 오류 코드 또는 별도 오류 조회 API로 전달한다.
- 위 예시는 개념 설명을 위해 오류를 `-1`로 단순화했다.

### 📌 6. 메모리 소유권 선택

| 설계 | 데이터 소유자 | 전달 방식 | 적합한 용도 |
|---|---|---|---|
| C# 배열 → C++ 계산 | C# / GC | `fixed` + 포인터 + 길이 | 단발성 대량 계산 |
| C++ 객체 → C# GUI | C++ | `IntPtr`/`SafeHandle` | NURBS, B-Rep, Mesh 모델 |
| C++ 결과 버퍼 → C# | C++ 또는 호출자 | 복사 API 또는 포인터 뷰 | 결과 시각화 |

- 예를 들어 절점 1,000만 개 × `double` 3개는 240,000,000바이트(약 229MiB)다.
- 호출할 때마다 복사하기보다 **소유권이 명확한 버퍼를 일시적으로 공유**하는 것이 유리할 수 있다.
- 다만 GUI 렌더러가 별도 포맷을 요구하면 변환 비용은 남는다.
- C++의 `std::vector`, `std::string`, 가상 함수가 있는 클래스는 C# `StructLayout`만으로 직접 호환되지 않는다. C++ 내부에 유지하고 C ABI 함수로 접근한다.

### 📌 7. GUI 스레드와 계산 스레드

- 장시간 C++ 계산을 GUI 스레드에서 직접 실행하면 화면이 멈춘다.
- C#에서는 `Task.Run` 등을 사용해 계산을 백그라운드로 실행하고, UI 변경은 GUI 스레드로 다시 전달한다.
- 동일 C++ 객체에 여러 스레드가 접근한다면 C++ 측 동기화 규칙도 정의해야 한다.

### 📌 8. 구현 전 체크리스트

- [ ] DLL과 C# 프로세스 아키텍처(x64 등) 일치
- [ ] `extern "C"`, export 이름, `CallingConvention` 일치
- [ ] 구조체 `sizeof`, 필드 `offsetof`, `Pack` 검증
- [ ] `bool`, 문자열, 고정 배열의 네이티브 표현 확인
- [ ] 배열 포인터의 원소 수와 유효 기간 명시
- [ ] 할당자와 해제자 짝 맞추기 (C++ `new` → C++ `delete` 등)
- [ ] C++ 예외가 ABI 경계를 넘지 않도록 처리
- [ ] GUI 스레드 차단 및 동시 접근 점검
- [ ] null, 빈 배열, 대량 데이터, 잘못된 핸들 등 경계 테스트

- **핵심:** C# GUI와 C++ 계산 코어의 연동은 가능하며, 난점은 호출 문법보다 **메모리 레이아웃·소유권·수명·스레드 안전성** 에 있다.

---

