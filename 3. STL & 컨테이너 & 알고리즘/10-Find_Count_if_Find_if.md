## 📚 C++ STL `find`, `count_if`, `find_if` 정리

### 📌 개요
- STL 알고리즘 `find`, `count_if`, `find_if`는 반복자 구간에서 조건에 맞는 요소를 찾거나 개수를 세는 데 사용됩니다.


### 📌 `std::find`
- **특정 값과 일치하는 첫 번째 요소를 찾는 알고리즘**

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

int main() {
    std::vector<std::string> vec{"apple", "tomato", "peach", "melon", "kiwi"};

    auto it = std::find(vec.begin(), vec.end(), "melon");
    if(it != vec.end()) {
        std::cout << *it << " : " << std::distance(vec.begin(), it) << std::endl;
    }
    // 출력: melon : 3
}
```

- **비교 연산** 은 `operator==`를 사용하므로, 사용자 정의 타입은 이를 오버로딩해야 함
- 찾지 못하면 `end()` 반복자를 반환



### 📌 `std::count_if`
- **조건에 맞는 요소의 개수를 세는 알고리즘**

#### 함수 포인터 사용 예
```cpp
#include <iostream>
#include <vector>
#include <algorithm>

template<typename T>
bool is_even(const T& num) {
    return (num % 2) == 0;
}

int main() {
    std::vector<int> vec{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int n = std::count_if(vec.begin(), vec.end(), is_even<int>);
    std::cout << "even count : " << n << std::endl; // 5
}
```

#### 람다식 사용 예
```cpp
int n = std::count_if(vec.begin(), vec.end(), [](int num){
    return (num % 2) == 0;
});
std::cout << "even count : " << n << std::endl;
```


### 📌 `std::find_if`
- **조건을 만족하는 첫 번째 요소를 찾는 알고리즘**

```cpp
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](int c) { return !std::isspace(c); }));
}

void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](int c) { return !std::isspace(c); }).base(), s.end());
}
```

#### 컨테이너에서 객체 찾기
```cpp
m_vecChannelMngr.erase(
    std::find_if(m_vecChannelMngr.begin(), m_vecChannelMngr.end(),
        [&](ChannelManagerBase* c) {
            return arChnMngr[i] == c;
        })
);
```


### 📌 요약
| 알고리즘 | 설명 | 반환값 |
|----------|------|--------|
| `find` | 값과 일치하는 첫 번째 요소 | 해당 요소 반복자 or `end()` |
| `count_if` | 조건을 만족하는 요소 개수 | 정수 개수 |
| `find_if` | 조건을 만족하는 첫 번째 요소 | 해당 요소 반복자 or `end()` |

#### ✅ Tip:
- 모든 알고리즘은 반복자를 사용하므로, 범위를 지정해 동작
- 람다식을 사용하면 함수 정의 없이 간단히 조건 작성 가능
- 요소가 사용자 정의 타입이면 `operator==` 또는 조건자 필요


### 📌 실무 응용
- 특정한 정보를 찾아 지우는 용도로 사용 가능
- 여기서는 ChannelManager에서 특정한 Channel을 지우는 용도로 사용

```cpp
void ChannelTreeWidget::delSelChnMngrs(std::vector<ChannelManagerBase*>& arChnMngr)
{
    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("InjuryReport"),
                                                                 tr("Do you want to delete seleted channels?"),
                                                                 QMessageBox::Ok | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Ok:
        break;
    default:
        return;
    }
    int nFirst = m_vecChannelMngr.size();
    for(int i=0; i<arChnMngr.size(); i++)
    {

        // find_if -> erase
        m_vecChannelMngr.erase(std::find_if(m_vecChannelMngr.begin(), m_vecChannelMngr.end(), [&](ChannelManagerBase* c) {
            return arChnMngr[i] == c; }));


    }
    if(nFirst != m_vecChannelMngr.size())
    {
        updateTrees();
    }
}
```

---

