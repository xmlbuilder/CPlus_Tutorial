## 📚 C++ STL `std::map` 정리

### 📌 특징
- `(키, 값)`의 쌍을 원소로 저장하는 **제네릭 컨테이너**
- 동일한 키를 가진 원소가 중복 저장되면 오류 발생
- 키를 이용해 값 검색 가능

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, std::string> dic;
    dic.insert(std::pair<std::string, std::string>("love", "사랑"));

    dic["love"] = "사랑";

    std::cout << dic["love"] << std::endl;
    std::cout << dic.at("love") << std::endl;
}
```



### 📌 주요 멤버 함수 및 연산자

| 멤버/연산자 | 설명 |
|-------------|------|
| `insert(pair<> &element)` | `키`와 `값`으로 구성된 pair 객체 element 삽입 |
| `at(key_type& key)` | `키` 값에 해당하는 `값` 리턴 |
| `begin()` | 맵의 첫 번째 원소에 대한 참조 리턴 |
| `end()` | 맵의 끝(마지막 원소 다음)을 가리키는 참조 리턴 |
| `empty()` | 맵이 비어 있으면 true 리턴 |
| `find(key_type& key)` | `키` 값에 해당하는 원소를 가리키는 iterator 리턴 |
| `erase(iterator it)` | iterator가 가리키는 원소 삭제 |
| `size()` | 맵에 들어 있는 원소의 개수 리턴 |
| `operator[key_type& key]` | `키` 값에 해당하는 원소를 찾아 `값` 리턴 |
| `operator=` | 맵 치환(복사) |



### 📌 Comparator 사용 예시

```cpp
#include <iostream>
#include <map>
using namespace std;

struct DoubleComp {
    bool operator()(double lhs, double rhs) const {
        return lhs > rhs; // 내림차순 정렬
    }
};

int main() {
    std::map<double, int, DoubleComp> myMap;

    myMap.insert(std::pair<double, int>(1.2, 1));
    myMap.insert(std::pair<double, int>(2.2, 1));
    myMap.insert(std::pair<double, int>(1.233333, 1));
    myMap.insert(std::pair<double, int>(3.2, 1));
    myMap.insert(std::pair<double, int>(1.2333339, 1));

    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        cout << it->first << " " << it->second << endl;
    }

    // 출력:
    // 3.2 1
    // 2.2 1
    // 1.2333339 1
    // 1.233333 1
    // 1.2 1
}
```


### 📌 정리
- `std::map`은 정렬된 키-값 쌍 컨테이너
- 키 중복 불가 (중복 키 허용하려면 `std::multimap` 사용)
- 기본 정렬은 오름차순 (`<`), 사용자 정의 Comparator로 변경 가능
- 검색, 삽입, 삭제가 `O(log n)` 복잡도로 수행됨

---

