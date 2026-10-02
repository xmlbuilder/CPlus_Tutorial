## 📘 static_cast

### 📌 1. static_cast란?
- 컴파일 타임에 타입을 명시적으로 변환하는 연산자
- 논리적으로 변환 가능한 타입끼리만 허용
- 런타임 비용 없음 → 빠르고 안전하지만 제한적
#### 🔹 기본 문법
- static_cast<새로운_타입>(표현식);



### 📌 2. 변환 가능한 타입 예시
#### 🔹 기본 타입 간 변환
```cpp
double d = 13.24;
int i = static_cast<int>(d); // 13
```
- 실수 ↔ 정수
- 열거형 ↔ 정수
- float ↔ double
#### 🔹 열거형 변환
```cpp
enum E_VAL { A = 0, B = 1, N = 13 };
E_VAL e = A;
int a = static_cast<int>(e); // 0
E_VAL e2 = static_cast<E_VAL>(13); // N
```


### 📌 3. 배열 → 포인터 변환
```cpp
int arr[10] = {1,2,3,4,5,6,7,8,9,10};
int* ptr = static_cast<int*>(arr); // OK
```

- 배열 이름은 포인터로 암시적 변환 가능
- static_cast로 명시적 변환도 가능

### 📌 4. 허용되지 않는 포인터 변환
#### 🔹 서로 관련 없는 타입 간 변환
```cpp
char str[] = "static_cast";
int* ptr2 = static_cast<int*>(str); // ❌ 컴파일 오류
```

- `char* → int*` 변환은 허용되지 않음
- `int* → char` *도 마찬가지

### 📌 5. 상속 관계에서의 포인터 변환
#### 🔹 업캐스트 (자식 → 부모)
```cpp
Triangle triangle;
Shape* shape = static_cast<Shape*>(&triangle); // OK
shape->draw(); // Triangle draw called
```
- 안전한 변환
- 자식 객체를 부모 타입으로 변환
#### 🔹 다운캐스트 (부모 → 자식)
```cpp
Shape s;
Triangle* triangle1 = static_cast<Triangle*>(&s); // 위험!
triangle1->draw(); // 예측 불가
triangle1->onlyTriangle(); // 런타임 오류 가능
```

- 컴파일은 되지만 안전하지 않음
- 실제 객체가 자식이 아닐 경우 예측 불가능한 동작

### 📌 6. dynamic_cast와의 차이점
| 항목 | static_cast | dynamic_cast |
|-----|--------------|-------------| 
| 타입 검사 | 컴파일 타임 | 런타임 | 
| 성능 | 빠름 | 느림 | 
| 다운캐스트 안전성 | 위험 | 안전 (nullptr 반환) | 
| RTTI 필요 여부 | 불필요 | 필요 (virtual 함수 필수) | 


#### 🔹 안전한 다운캐스트 예시
```cpp
Shape* shape = new Shape();
Triangle* triangle = dynamic_cast<Triangle*>(shape);
if (triangle == nullptr) {
    cout << "error detected" << endl;
}
```

- 실패 시 nullptr 반환
- Null pointer exception 방지 가능

### 📌 특징 요약
- 컴파일 타임에 타입 오류를 잡아줌
- 논리적으로 변환 가능한 타입만 허용
- 런타임 비용 없음
- 상속 관계에서 업캐스트는 안전, 다운캐스트는 위험
- 포인터 타입 간 변환은 제한적
- 배열 → 포인터, 함수 → 함수 포인터는 허용

### 📌 실전 팁
- 기본 타입 변환에는 static_cast가 가장 적합
- 상속 관계에서 다운캐스트는 반드시 dynamic_cast 사용
- 컴파일 오류를 미리 잡고 싶을 때 static_cast를 적극 활용

---

