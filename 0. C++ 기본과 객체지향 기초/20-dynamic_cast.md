# 🧠 C++ dynamic_cast 완전 정리

## 📌 1. dynamic_cast란?
- **런타임 타입 검사(RTTI)** 를 통해 안전한 다운캐스팅을 수행하는 연산자
- 부모 클래스 포인터를 자식 클래스 포인터로 변환할 때 사용
- 실제 객체의 타입이 자식 클래스인지 확인한 후 변환

### 🔧 기본 문법
```cpp
dynamic_cast<새로운_타입>(표현식);
```

### 예시:
```cpp
Child* c = dynamic_cast<Child*>(parentPtr);
```


## 🔍 2. 사용 조건
| 조건 | 설명 |
|------|-----| 
| 상속 관계 | 부모 → 자식 클래스 간 변환 | 
| RTTI 활성화 | 최소 하나의 virtual 함수가 있어야 함 | 
| 포인터 또는 참조 타입 | dynamic_cast는 포인터 또는 참조에만 사용 가능 | 



## ✅ 3. 성공/실패 시 동작
| 대상 타입 | 성공 시 | 실패 시 | 
|------|-----|-------|
| 포인터 | 변환된 포인터 반환 | nullptr 반환 | 
| 참조 | 변환된 참조 반환 | std::bad_cast 예외 발생 | 


### 예시: 포인터
```cpp
Shape* shape = new Triangle();
Triangle* tri = dynamic_cast<Triangle*>(shape); // OK
```



### 예시: 실패 시 nullptr
```cpp
Shape* shape = new Shape();
Triangle* tri = dynamic_cast<Triangle*>(shape); // nullptr
```

### 예시: 참조 → 예외 발생
```cpp
Shape& shapeRef = Shape();
try {
    Triangle& triRef = dynamic_cast<Triangle&>(shapeRef); // throws std::bad_cast
} catch (const std::bad_cast& e) {
    std::cout << "bad_cast caught: " << e.what() << std::endl;
}
```


### 🧪 실전 예제
```cpp
class Shape {
public:
    virtual void draw() { std::cout << "Shape\n"; }
};

class Triangle : public Shape {
public:
    void draw() override { std::cout << "Triangle\n"; }
    void onlyTriangle() { std::cout << "Triangle only\n"; }
};

int main() {
    Shape* s = new Shape();
    Triangle* t = dynamic_cast<Triangle*>(s);

    if (t == nullptr) {
        std::cout << "Downcast failed\n";
    } else {
        t->onlyTriangle();
    }
}
```


## ⚠️ 4. 주의사항
- RTTI가 비활성화된 컴파일러 옵션에서는 동작하지 않음
- 성능 비용이 있으므로 빈번한 캐스팅은 피하는 것이 좋음
- 참조 타입 변환 실패 시 예외 처리 필수

## 🔄 5. static_cast와의 비교
| 항목 | static_cast | dynamic_cast | 
|------|------------|---------------|
| 타입 검사 | 컴파일 타임 | 런타임 | 
| 안전성 | 낮음 (실패 시 예측 불가) | 높음 (실패 시 nullptr 또는 예외) | 
| RTTI 필요 | 불필요 | 필요 (virtual 함수 필수) | 
| 성능 | 빠름 | 느림 | 
| 사용 목적 | 논리적 변환 | 안전한 다운캐스트 | 



## ✅ 요약
- dynamic_cast는 런타임 타입 검사 기반의 안전한 다운캐스트 도구
- RTTI가 활성화되어야 하며, virtual 함수가 반드시 있어야 함
- 포인터 변환 실패 시 nullptr, 참조 변환 실패 시 std::bad_cast 예외
- 성능 비용이 있으므로 꼭 필요한 경우에만 사용

---
