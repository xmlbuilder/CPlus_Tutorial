# AutoDelete
AutoDelete<T> 구조는 C++에서 소유권을 명확히 표현하고, 
  메모리 해제를 자동으로 처리하는 간단한 스마트 포인터예요. 
  마치 std::unique_ptr<T>의 축소판처럼 동작하죠.

## 코드
```cpp
template <typename T>
struct AutoDelete {
  T* p{nullptr};
  AutoDelete() = default;
  explicit AutoDelete(T* q):p(q){}
  ~AutoDelete(){ delete p; }
  T* release(){ T* q = p; p=nullptr; return q; }
  T* get() const { return p; }
  T& operator*() { return *p; }
  T* operator->() { return p; }
  AutoDelete(const AutoDelete&) = delete;
  AutoDelete& operator=(const AutoDelete&) = delete;
};
```

## 🧠 AutoDelete<T>을 쓰는 장점
### ✅ 1. RAII 기반 자동 메모리 관리
- 객체가 스코프를 벗어나면 AutoDelete의 소멸자가 delete를 호출해 메모리를 자동 해제
- new로 할당한 포인터를 잊지 않고 안전하게 정리 가능
```cpp
AutoDelete<MyClass> obj(new MyClass());
// 자동 delete 호출됨
```


### ✅ 2. 소유권 명시
- release()를 통해 소유권을 명시적으로 포기할 수 있음
- 이후 AutoDelete 더 이상 메모리를 관리하지 않음
```cpp
T* raw = obj.release(); // AutoDelete 더 이상 delete하지 않음
```

### ✅ 3. 직관적인 포인터 인터페이스
- operator*와 operator->를 통해 일반 포인터처럼 사용 가능
- get()으로 내부 포인터를 안전하게 조회 가능
```cpp
obj->doSomething();
std::cout << *obj;
```

### ✅ 4. 예외 안전성
- 예외가 발생해도 소멸자가 호출되므로 메모리 누수 방지

## 🔍 왜 std::unique_ptr 대신 직접 만들었을까?

| 상황 또는 목적         | 설명                                      |
|------------------------|-------------------------------------------|
| `<memory>` 없이 사용    | 헤더 의존성을 줄이기 위해 직접 구현함         |
| `delete[]` 처리 필요   | 배열 메모리 해제를 위해 커스텀 소멸자 구현 필요 |
| 커스터마이징 용도       | 로깅, 디버깅, 리소스 추적 등 부가 기능 추가 목적 |


## ⚠️ 주의할 점
- AutoDelete<T>는 std::unique_ptr<T>처럼 복사 불가능하게 막아야 안전합니다
→ 복사 생성자/대입 연산자를 delete 처리하는 게 좋습니다
```cpp
AutoDelete(const AutoDelete&) = delete;
AutoDelete& operator=(const AutoDelete&) = delete;
```

- delete[]가 필요한 경우에는 별도 처리 필요

## 💬 요약
AutoDelete<T>는 C++에서 소유권을 명확히 표현하고, 메모리 해제를 자동화하는 경량 스마트 포인터예요.

---



