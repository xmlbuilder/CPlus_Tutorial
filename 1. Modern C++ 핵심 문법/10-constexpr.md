# 📘 constexpr vs const — C++ 정리 문서
## 🧠 기본 개념
| 키워드 | 의미 |
|-------|------| 
| const | 값이 변경 불가능함을 의미. 런타임에 결정되어도 상관없음 | 
| constexpr | 값이 컴파일 타임에 결정되어야 함. 상수식이어야 함 | 



## 🔍 차이점 요약
| 항목 | const | constexpr |
|------|------|-----------|
| 평가 시점 | 런타임 또는 컴파일 타임 | 반드시 컴파일 타임 | 
| 사용 가능 위치 | 변수, 포인터, 참조 등 | 변수, 함수, 클래스, 템플릿 인자 등 | 
| 배열 크기 지정 | 불가능 (const int size = x;) | 가능 (constexpr int size = 10;) | 
| 템플릿 인자 | 불가능 (const int x = a;) | 가능 (constexpr int x = 3;) | 
| 함수 반환 | 런타임에 계산됨 | 컴파일 타임에 계산됨 (조건 만족 시) | 
| 컴파일러 최적화 | 제한적 | 매우 강력함 | 



## 🧪 예제 비교
```cpp
int a = 5;
const int x = a;       // OK: 런타임에 결정 가능
constexpr int y = a;   // ❌ Error: a는 컴파일 타임 상수가 아님


constexpr int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; ++i)
        result *= i;
    return result;
}

A<factorial(5)> a;  // OK: 컴파일 타임에 계산됨
```


## 🧩 DLL과 헤더에서의 영향
### 📦 DLL 헤더에 constexpr이 있을 경우
- constexpr 변수나 함수는 컴파일 타임에 인라인으로 처리됨
- DLL에 실제로 기계어 코드가 포함되지 않을 수 있음
- 메인 프로젝트에서 constexpr을 사용하면 링크 타임 영향 없음
- 반면 const는 실제 메모리에 존재할 수 있으며 DLL에서 참조될 수 있음
## ✅ 요약
| 항목 | const in DLL | constexpr in DLL | 
|------|--------------|------------------|
| 메모리 존재 여부 | 있음 (전역 변수 등) | 없음 (인라인 처리됨) | 
| 링크 타임 영향 | 있음 | 없음 | 
| 헤더 포함 시 영향 | 참조 필요 | 값 복사됨 (컴파일 타임) | 



## 🧮 C# const vs readonly 비교
| 키워드 | 의미 | 평가 시점 | 수정 가능 여부 | 
|-------|------|---------|--------------|
| const | 컴파일 타임 상수 | 컴파일 타임 | ❌ 불가능 | 
| readonly | 런타임 상수 (생성자에서만 초기화 가능) | 런타임 | ✅ 생성자에서만 | 


## 📌 메인에서 호출 시 차이
- const는 값이 컴파일 타임에 결정되므로 DLL에서 변경해도 메인에 반영되지 않음
- readonly는 런타임에 결정되므로 DLL에서 변경하면 메인에도 반영됨
### 🧪 예시
```csharp
public class Config {
    public const int MaxSize = 10;
    public static readonly int Timeout = 30;
}
```

- MaxSize는 컴파일 타임에 값이 박혀서 DLL 교체해도 메인에 영향 없음
- Timeout은 런타임에 결정되므로 DLL 교체 시 메인에서 값이 바뀔 수 있음

✅ 전체 요약
| 언어 | 키워드 | 컴파일 타임 | 런타임 | 수정 가능 | DLL 영향 |
|------|-------|-----------|-------|----------|---------| 
| C++ | const | 가능 | 가능 | ❌ | 있음 | 
| C++ | constexpr | ✅ 반드시 | ❌ | ❌ | 없음 | 
| C# | const | ✅ 반드시 | ❌ | ❌ | 없음 | 
| C# | readonly | ❌ | ✅ | ✅ 생성자 | 있음 | 


-----


