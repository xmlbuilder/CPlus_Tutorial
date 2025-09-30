# 📚 C++ STL `std::sort` 사용법 정리

## 🧩 개요

`std::sort`는 C++ STL에서 제공하는 **정렬 알고리즘**으로, 주어진 범위의 요소를 **오름차순** 또는 **사용자 정의 기준**에 따라 정렬할 수 있습니다.  
시간 복잡도는 평균적으로 **O(N log N)** 이며, 매우 빠른 정렬 방법 중 하나입니다.

---

## ⚙️ 기본 문법

```cpp
#include <algorithm>

std::sort(반복자_시작, 반복자_끝);                 // 기본 오름차순 정렬
std::sort(반복자_시작, 반복자_끝, 비교_함수);     // 사용자 정의 비교 함수
```

- **기본 정렬** : `<` 연산자를 사용하여 오름차순
- **사용자 정의 정렬** : 비교 함수를 제공하여 정렬 순서를 변경

---

## 📌 예제 코드

```cpp
#include <iostream>
#include <algorithm>
#include <array>

bool compare(int a, int b) { return a > b; }

int main(void) {
    std::array<int, 10> arr1 = {5, 4, 3, 22, 34, 25, 45, 78, 90, 100};

    // 1️⃣ 기본 오름차순 정렬
    std::sort(arr1.begin(), arr1.end());
    for (int val : arr1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 2️⃣ 사용자 정의 함수(내림차순 정렬)
    std::sort(arr1.begin(), arr1.end(), compare);
    for (int val : arr1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 3️⃣ 람다식을 이용한 오름차순 정렬
    std::sort(arr1.begin(), arr1.end(), [](int a, int b) {
        return a < b;
    });
    for (int val : arr1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}
```

---

## 📝 실행 결과

```
3 4 5 22 25 34 45 78 90 100   // 기본 오름차순
100 90 78 45 34 25 22 5 4 3   // compare() 함수 이용 내림차순
3 4 5 22 25 34 45 78 90 100   // 람다식 이용 오름차순
```

---

## 💡 정리

- `std::sort`는 **빠르고 효율적인 정렬 함수**
- 기본은 **오름차순** (`<` 연산자 사용)
- 비교 함수를 전달하여 내림차순, 특정 조건 기반 정렬 가능
- 람다식을 활용하면 코드 간결화 가능


📌 **Tip** : `std::array`뿐만 아니라 `std::vector`, C 배열 등 반복자가 지원되는 컨테이너에서 모두 사용 가능

---

## 실무 사용
- vector에 float 값을 담아 작은 순서에서 큰 순서로 정렬에 사용
  
```cpp
std::vector<float> vecFloat;
GrpPt2f *parPoint = pData1->getGrpPtArray()->getArray();
for(int i=0; i< pData1->getCount() ; i++)
{
    vecFloat.push_back(parPoint[i].fy);
}

//std::less -> sort
std::sort(vecFloat.begin(), vecFloat.end(), std::less<float>());
```

