## C++17 std::optional 완전 정리
### 📌 개요
- std::optional은 값이 있을 수도 있고 없을 수도 있는 상황을 표현하기 위한 C++17 표준 라이브러리 타입입니다.  
- 예외 처리 없이 안전하게 **값 없음** 을 표현할 수 있어 함수의 반환값이나 설정값 처리에 유용합니다.  

### 📌 기본 사용 예제
```cpp
#include <iostream>
#include <string>
#include <optional>
#include <map>

std::optional<std::string> getValueFromMap(const std::map<int, std::string>& m, int key) {
    auto itr = m.find(key);
    if (itr != m.end()) {
        return itr->second;
    }
    return std::nullopt;
}

int main() {
    std::map<int, std::string> data = {
        {1, "model"},
        {2, "sample"},
        {3, "sample2"}
    };

    std::optional<std::string> retOpt = getValueFromMap(data, 3);
    if (retOpt.has_value()) {
        std::cout << retOpt.value() << std::endl; // sample2
    }
}
```


### 📌 값 접근 방법
| 방법 | 설명 | 
|-------------|------------------------------------------| 
| opt.value() | 값이 있을 때 접근, 없으면 예외 발생 | 
| *opt 또는 opt-> | 역참조로 값 접근 | 
| opt.value_or(def) | 값이 없을 경우 기본값 반환 | 

```cpp
int width = get_option("WIDTH").value_or(800); // 기본값 800
```


### 📌 복사 vs 참조
- ❗ 기본적으로 optional은 `값을 복사함`
```cpp
OptionA a;
a.data = 4;
std::optional<OptionA> b = a;
a.data = 23;

std::cout << b->data << std::endl; // 4 (복사됨)
```

-  참조를 원할 경우 std::reference_wrapper 사용
```cpp
std::optional<std::reference_wrapper<OptionA>> b = std::ref(a);
b->get().data = 23;

std::cout << a.data << std::endl; // 23 (참조됨)
```


### 📌 std::optional과 std::tuple 조합
```cpp
template <typename T, typename U>
std::optional<std::tuple<T, U>> monad(const T& rhs1, const U& rhs2) {
    if (rhs1 != NULL && rhs2 != NULL) {
        return std::tuple<T, U>(rhs1, rhs2);
    }
    return std::nullopt;
}

if (auto op1 = monad(1, 3.5); op1.has_value()) {
    std::cout << std::get<0>(*op1) << std::endl;
    std::cout << std::get<1>(*op1) << std::endl;
}
```

- 복합적인 조건을 만족할 때만 값 반환
-  std::get<N>(*optional)로 튜플 내부 값 접근 가능

### ⚠️ 주의 사항
- value()는 값이 없을 경우 예외(bad_optional_access)를 던짐
- optional<T>는 T의 복사 생성자를 요구함
- 참조를 저장하려면 std::reference_wrapper<T>를 사용해야 함
- optional은 nullptr과는 다름.
- `std::nullopt` 로 명시적으로 **없음** 표현

### 📌 요약
| 항목 | 설명 | 
|-------------|------------------------------------------| 
| std::optional<T> | T 타입의 값이 있을 수도 없을 수도 있는 컨테이너 | 
| has_value() | 값 존재 여부 확인 | 
| value_or(default) | 값이 없을 경우 기본값 반환 | 
| std::nullopt | 값 없음을 나타내는 상수 | 
| std::reference_wrapper<T> | 참조를 저장하고 싶을 때 사용 | 
| *opt, opt->, value() | 값 접근 방식 | 

---


### 현업에서 사용 예제

- 예제는 상해 치 구할 때 값이 존재하지 않아서 0인지 존재하는데 0인지 구별이 필요 할 때 사용

```cpp
std::optional<double> CalcInjuryBase::getDoubleValue(const std::string& strKey, double dScale)
{
    if(m_pKeyValuePool->hasValue(strKey)){

         return strtof(m_pKeyValuePool->getValue(strKey).c_str(), nullptr) * dScale;
    }

    return std::nullopt;
}

std::optional<double> optHead3ms = calInjNCAP->getDoubleValue(REAR_LH_Head3MS);

if(optHead3ms.has_value()){
    bHasLeft = true;
    QTableWidgetItem* item = new QTableWidgetItem(QString::number(optHead3ms.value(), 'f', 3));
    item->setTextAlignment(Qt::AlignRight);
    ui->tableWidget->setItem(startIndex, 2, item);
    //Point
    std::optional<double> optPointHead3MS = calInjNCAP->getDoubleValue("P4Head3MS");

    if(optPointHead3MS.has_value())
    {
        double dPointHead3MS = optPointHead3MS.value();
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(dPointHead3MS, 'f', 3));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(startIndex+1, 2, item);
    }
}
```
---




