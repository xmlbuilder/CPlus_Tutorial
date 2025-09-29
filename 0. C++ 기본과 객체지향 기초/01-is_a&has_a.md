# C++ 개발자를 위한 is-a / has-a 튜토리얼

is-a : 객체 지향 프로그램에서 상속을 관계를 정의한다.  
"A는 B 유형의 물건"이라고 말하는 것이다.   
사과는 과일의 일종이고 세단은 자동차의 일종이다.  

has-a : 하나의 객체가 다른 객체를 가지고 있는 관계이다.  
자동차는 엔진을 가지고 있고 집을 목욕탕을 가지고 있다.


## is‑a (상속, 일반화–특수화)
> “A는 B의 한 종류다.”

- 의미: **하위 타입(subtype)** 이 상위 타입(supertype)의 **대체 가능성**을 만족해야 함
  (LSP: Liskov Substitution Principle).
- 특징: 다형성(polymorphism), 인터페이스 공유.
- 잘못 쓰면: 상속으로 구현 세부사항까지 엮여 **결합도↑, 취약한 기반 클래스 문제** 발생.

### C++ 예시
```cpp
struct IGCurve { virtual ~IGCurve() = default; virtual double Length() const = 0; };

struct NurbsCurve : IGCurve {
    // ... control points, knots ...
    double Length() const override { /* arc-length compute */ }
};

struct LineCurve : IGCurve {
    ON_3dPoint a, b;
    double Length() const override { return (b - a).Length(); }
};

// is-a: NurbsCurve, LineCurve 는 IGCurve 의 ‘한 종류’
```

### LSP 위반 대표 사례 (피해야 함)
```cpp
struct Rectangle { virtual void SetWidth(double); virtual void SetHeight(double); };
struct Square : Rectangle { // “정사각형은 직사각형” 논리로 상속하면
    // SetWidth/SetHeight 동작 제약 때문에 LSP 위반 가능
};
```

---

## has‑a (구성, 합성/집합)
> “A는 B를 **가지고 있다**.”

- 의미: 객체 **내부에 다른 객체를 보유**하여 기능을 구성.
- 장점: **결합도↓, 재사용성↑, 테스트 용이**, “상속보다 합성 선호(Favor composition over inheritance)”.
- 두 가지 뉘앙스:
  - **합성(Composition)**: 강한 소유/생명주기 동일 (부품이 주체에 종속)
  - **집합(Aggregation)**: 약한 소유/공유 (부품 생명주기 독립)

### C++ 예시 (엔진을 가진 자동차)
```cpp
// 합성: Car가 Engine을 '소유' (생명주기 함께)
class Engine {
public: void Start(); void Stop();
};

class Car {
    Engine engine_;                    // by value → 강한 소유
public:
    void Start() { engine_.Start(); }
};

// 집합: Garage가 Car를 '보관' (소유 X, 생명주기 독립)
class Garage {
    std::vector<Car*> cars_;           // 비소유 포인터(관찰자)
public:
    void Park(Car* c) { cars_.push_back(c); }
};
```

### 현대 C++ 소유권 모델
```cpp
class Renderer {
    std::unique_ptr<Device> device_;   // 합성(독점 소유)
    std::shared_ptr<Texture> gridTex_; // 공유 소유(집합적 사용)
    Texture* overlay_ = nullptr;       // 비소유(관찰)
};
```

---

## 언제 is‑a / has‑a?
- **is‑a**:
  - 진짜로 “대체 가능”해야 함 (상위 타입의 계약을 충실히 이행).
  - 인터페이스를 공유해 다형성이 목적일 때.
- **has‑a**:
  - 동작을 조립/확장하려는 목적.
  - 구현 세부를 숨기고 바꿀 여지가 많을 때.
  - 라이프사이클/소유권 제어가 중요할 때.

> **룰**: 헷갈리면 우선 **has‑a(합성)** 로 설계하고, “정말로” 다형성/대체 가능성이 필요할 때만 **is‑a** 로 승격.

---

## UML/그림(텍스트 다이어그램)

### is‑a (상속, 일반화)
```
   IGCurve
     ▲
     │ (is-a)
 ┌───┴─────────┐
 │             │
NurbsCurve   LineCurve
```

### has‑a (합성: 강한 소유)
```
Car ──has-a──▶ Engine
[filled diamond at Car side]
```
텍스트로:
```
Car
 └─ engine_: Engine   (by value / unique_ptr)  ← Composition
```

### has‑a (집합: 약한 소유/공유)
```
Garage ──o──▶ Car
[open diamond at Garage side]
```
텍스트로:
```
Garage
 └─ cars_: std::vector<Car*> or std::vector<std::weak_ptr<Car>>  ← Aggregation
```

---

## CAD 맥락 예시(바로 써먹기)

### 올바른 is‑a
```
IGSurface
  ▲
  ├─ NurbsSurface
  ├─ PlaneSurface
  └─ RevolvedSurface
```

### has‑a (BRep/Trim 구조)
```
BrepFace  has-a  ON_Surface (소유)
BrepFace  has-a  vector<BrepTrim*> (집합/참조)
BrepTrim  has-a  IGCurve* (참조; 곡선 공유 가능)
```

### pImpl로 구현 세부 캡슐화 (has‑a의 한 형태)
```cpp
class SurfaceTessellator {
public:
    SurfaceTessellator();
    ~SurfaceTessellator();
    void Tessellate(const IGSurface& s);
private:
    struct Impl;
    std::unique_ptr<Impl> p_; // has-a, 구현 은닉
};
```

---

## 안티패턴 체크리스트
- [ ] 상속을 “코드 재사용” 목적으로만 썼다 → 합성으로 변경 고려.
- [ ] LSP 깨질 여지가 있다(사전/사후조건 변경, 불변식 위반).
- [ ] 기반 클래스가 구체 구현을 노출한다(“Fragile Base Class”).
- [ ] 소유권이 불명확(누가 delete?) → `unique_ptr/shared_ptr` 로 명확화.
- [ ] “is‑implemented‑in‑terms‑of”를 “is‑a”로 착각(구현 상속 vs. 타입 계층 혼동).

---

## 두 개를 섞는 현명한 패턴

- **인터페이스는 is‑a, 구현은 has‑a**
  - 순수 가상 인터페이스(`IGCurve`)로 다형성을 확보(is‑a)
  - 내부 구현은 합성(pImpl/구성요소)으로 조립
- **CRTP로 비용 없는 인터페이스**(필요 시)
  ```cpp
  template <class Derived>
  struct CurveBase {
      double Length() const { return static_cast<const Derived*>(this)->LengthImpl(); }
  };
  struct MyCurve : CurveBase<MyCurve> {
      double LengthImpl() const { /*...*/ }
  };
  ```

