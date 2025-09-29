# ☕ ON_SafeMemMgrEx 사용 가이드 for Java 개발자

## 📌 도입 취지

Java 개발자는 보통 Garbage Collector(GC)에 의존해 객체의 생명주기를 자동으로 관리합니다. 하지만 C++에서는 `new`와 `delete`를 직접 호출해야 하며, 이로 인해 다음과 같은 문제가 발생할 수 있습니다:

- 메모리 누수
- 객체 소멸자 누락
- 성능 저하 (반복적인 `new/delete`)
- 멀티스레드 환경에서의 동기화 문제

`ON_SafeMemMgrEx`는 이러한 문제를 해결하기 위해 설계된 **C++용 객체 중심 메모리 관리 시스템**입니다. Java 개발자가 GC 없이도 안전하고 효율적으로 객체를 관리할 수 있도록 도와줍니다.

---

## 🚀 핵심 개념

| Java 개념              | ON_SafeMemMgrEx 대응               |
|------------------------|------------------------------------|
| `new T()`              | `AllocObject<T>(tag, args...)`     |
| `obj.close()` 또는 GC  | `FreeObject(tag, obj)`             |
| `try-with-resources`  | 태그 기반 객체 그룹 관리           |
| `ConcurrentHashMap`   | 내부 `unordered_map` + `mutex` 사용 |

---

## 🧪 사용법

### 1. 객체 생성

```cpp
ON_SafeMemMgrEx memMgr;

// Java: MyClass obj = new MyClass(10);
MyVirtualClass* obj = memMgr.AllocObject<MyVirtualClass>("network", 10);
```

### 2. 객체 해제
```cpp
// Java: obj.close() 또는 GC
memMgr.FreeObject("network", obj);
```

- 객체의 소멸자를 호출한 뒤 메모리를 반환합니다.
- 태그를 통해 해당 객체가 어떤 그룹에 속했는지 명시합니다.

---

### 3. 태그 그룹 전체 해제
```cpp
// Java: 리소스 그룹 전체 정리
memMgr.FreeObjectsByTag("network");
```

### 4. 전체 객체 해제
```cpp
// Java: System.gc() 또는 shutdown hook
memMgr.FreeAllObjects();
```
- 모든 객체를 소멸시키고, 내부 메모리 풀도 정리합니다.

---

## 🧠 예제 코드
```cpp
class MyVirtualClass {
public:
    MyVirtualClass(int a) {
        std::cout << "Created with " << a << std::endl;
    }
    virtual ~MyVirtualClass() {
        std::cout << "Destroyed" << std::endl;
    }
};

int main() {
    ON_SafeMemMgrEx memMgr;

    auto* obj1 = memMgr.AllocObject<MyVirtualClass>("network", 1);
    auto* obj2 = memMgr.AllocObject<MyVirtualClass>("graphics", 2);
    auto* obj3 = memMgr.AllocObject<MyVirtualClass>("network", 3);

    memMgr.FreeObject("network", obj1);         // 개별 해제
    memMgr.FreeObjectsByTag("network");         // 그룹 해제
    memMgr.FreeAllObjects();                    // 전체 해제
}

```
---
## ✅ 장점 요약

- 객체 생성/소멸을 안전하게 관리
- 메모리 풀 기반으로 성능 향상
- 태그 기반 그룹 관리로 구조적 해제 가능
- 스레드 안전성 확보
- Java 개발자에게 친숙한 API 스타일

---
## 📎 참고 

- 객체를 생성할 때는 반드시 AllocObject를 사용하세요. new를 직접 쓰면 메모리 풀을 우회하게 됩니다.
- 객체를 해제할 때는 FreeObject 또는 FreeObjectsByTag를 사용하세요. delete를 직접 쓰면 소멸자 호출은 되지만 메모리 풀에는 반환되지 않습니다.
- 태그는 "network", "graphics", "session" 등 자유롭게 지정할 수 있으며, 그룹 해제에 유용합니다.






