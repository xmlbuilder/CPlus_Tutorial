# ⚠️ 값 범주가 바뀌었을 때의 문제점과 예시

## 1️⃣ RValue가 LValue가 되었을 때의 문제
### 💥 문제: 이동이 아닌 복사가 발생함

```cpp
void take(std::string&& s) {
    std::string copy = s; // ❌ s는 LValue로 취급됨 → 복사 발생
}
```


- s는 RValue Reference지만 이름이 있으므로 LValue
- 복사 생성자가 호출됨 → 성능 저하
### ✅ 해결 방법
```cpp
std::string moved = std::move(s); // ✅ RValue로 캐스팅 → 이동 발생
```


## 2️⃣ LValue가 RValue로 되었을 때의 문제
### 💥 문제: 원본 객체가 무효화됨
```cpp
std::string a = "hello";
std::string b = std::move(a); // a는 RValue로 취급됨 → 내부 자원 이동됨
std::cout << a; // ❌ a는 비어있을 수 있음
```

- a는 std::move로 RValue가 되어 자원을 b로 이동
- 이후 a는 사용하면 안 됨
### ✅ 해결 방법
- std::move는 정말 이동이 필요한 경우에만 사용
- 이후 a를 사용하지 않거나, 다시 값을 할당해야 함

## 3️⃣ Forwarding Reference에서 std::forward를 안 쓴 경우
### 💥 문제: 값 범주가 바뀌어 복사 발생
```cpp
template<typename T>
void wrapper(T&& arg) {
    process(arg); // ❌ arg는 LValue로 취급됨 → 복사 발생
}
```

- arg는 Forwarding Reference지만 이름이 있으므로 LValue
- process가 RValue Reference를 받는다면 오버로드 미스매치
### ✅ 해결 방법
```cpp
process(std::forward<T>(arg)); // ✅ 원래 값 범주 유지
```


## 4️⃣ RValue를 LValue로 바꿔서 참조할 때의 문제
### 💥 문제: Dangling Reference
```cpp
const std::string& ref = std::string("temp"); // ❌ temp는 RValue → 수명 짧음
```


- ref는 LValue Reference지만 RValue를 참조
- temp는 임시 객체 → 수명 종료 후 ref는 Dangling
### ✅ 해결 방법
```cpp
std::string temp = "temp";
const std::string& ref = temp; // ✅ 안전한 참조
```



## 5️⃣ LValue를 RValue Reference로 받으려 할 때
### 💥 문제: 컴파일 오류
```cpp
std::string s = "hello";
take(std::move(s)); // ✅ OK
take(s);            // ❌ 오류: LValue를 RValue Reference로 전달 불가
```

- take(std::string&&)는 RValue만 받음
- s는 LValue → std::move(s) 필요

## 🧾 요약: 값 범주 변화의 문제점
| 상황 | 문제점 | 해결 방법 |
|------|-------|---------| 
| RValue → LValue | 복사 발생 | std::move 사용 | 
| LValue → RValue | 원본 객체 무효화 | 이후 사용 금지 또는 재할당 | 
| Forwarding Reference에서 forward 미사용 | 값 범주 손실 → 오버로드 실패 | std::forward<T>(arg) 사용 | 
| RValue 참조를 LValue로 받음 | Dangling Reference | 수명 연장 또는 복사 | 
| LValue를 RValue Reference로 전달 | 컴파일 오류 | std::move로 캐스팅 | 

----



