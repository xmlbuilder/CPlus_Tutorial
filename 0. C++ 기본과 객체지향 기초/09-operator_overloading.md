## ➕ C++ 연산자 오버로딩 정리

### 📌 개요
- C++에서는 `+`, `-`, `=`, `*`, `==`, `<` 등과 같은 연산자를 사용자 정의 타입에 맞게 재정의할 수 있습니다.   
- 이를 연산자 오버로딩이라고 하며, 클래스 객체가 기본 타입처럼 동작하도록 만들어줍니다.

### 📌  연산자 오버로딩의 목적
- 사용자 정의 타입을 기본 타입처럼 직관적으로 사용 가능
- 코드의 가독성 향상 및 사용 편의성 증가
- 예: player1 + player2, point1 == point2

### 📌 자동 생성 여부
| 연산자 | 자동 생성 여부 |
|--------|----------------|
| = (대입) | 자동 생성됨 (사용자 정의 가능) | 
| 기타 (+, -, ==, < 등) | 자동 생성되지 않음, 직접 구현 필요 | 



### 📌 예제: Point 클래스

```cpp
#include <iostream>

class Point {
public:
    int x, y;

    Point(int x, int y) : x(x), y(y) {}

    // + 연산자 오버로딩
    Point operator+(const Point& rhs) const {
        return Point(x + rhs.x, y + rhs.y);
    }

    // - 연산자 오버로딩
    Point operator-(const Point& rhs) const {
        return Point(x - rhs.x, y - rhs.y);
    }

    // == 연산자 오버로딩
    bool operator==(const Point& rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    // < 연산자 오버로딩
    bool operator<(const Point& rhs) const {
        return (x < rhs.x) || (x == rhs.x && y < rhs.y);
    }
};
```


### 📌 사용 예시
```cpp
int main() {
    Point p1{10, 20};
    Point p2{30, 40};

    Point p3 = p1 + p2; // p1.operator+(p2)
    p3 = p1 - p2;       // p1.operator-(p2)

    if (p1 == p2) {     // p1.operator==(p2)
        std::cout << "Points are equal\n";
    }

    if (p1 < p2) {      // p1.operator<(p2)
        std::cout << "p1 is less than p2\n";
    }

    return 0;
}
```


### 📌 오버로딩 방식
| 방식 | 설명 |
|----------------------|-----------------------------| 
| 멤버 함수 | Point operator+(const Point& rhs) const; | 
| 비멤버 함수 | friend Point operator+(const Point& lhs, const Point& rhs); | 
| friend 함수 사용 | 클래스 외부에서 정의할 때 접근 권한 필요 시 사용 | 


### ⚠️ 주의사항
- 연산자 오버로딩은 의미가 명확한 경우에만 사용해야 함
- = 연산자는 자동 생성되지만, 깊은 복사가 필요한 경우 직접 구현 필요
- 논리 연산자, 입출력 연산자(<<, >>) 등도 오버로딩 가능

### 📌 요약
| 항목 | 설명 |
|----------------------|-----------------------------|  
| 목적 | 사용자 정의 타입을 기본 타입처럼 사용 | 
| 자동 생성 여부 | =만 자동, 나머지는 직접 구현 필요 | 
| 구현 방식 | 멤버 함수 또는 friend 함수 | 
| 사용 예시 | obj1 + obj2, obj1 == obj2 등 | 
| 주의사항 | 의미가 명확한 경우에만 오버로딩 권장 | 

---



