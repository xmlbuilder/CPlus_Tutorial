# 📘 C++에서 const 객체와 mutable 멤버의 관계

## 🔍 기본 개념
- const 객체는 멤버 함수에서 멤버 변수의 값을 변경할 수 없습니다.
- const 멤버 함수는 void func() const처럼 선언되며, 내부에서 this 포인터는 const Player*로 취급됩니다.
- 하지만 때로는 논리적으로는 상수지만, 내부적으로는 변경이 필요한 멤버가 있을 수 있습니다.
예: 캐시, 로그 카운터, 디버깅용 플래그 등
이럴 때 사용하는 키워드가 바로 **mutable**입니다.

## 🧱 예제 코드 확장

- 🔧 Player 클래스에 mutable 멤버 추가
```cpp
class Player {
private:
    int x, y;
    int speed;
    static int numPlayer;
    mutable int accessCount; // const 함수에서도 변경 가능

public:
    Player(int x, int y, int speed);
    void setPosition(int x, int y);
    void printPosition() const;
    static int getNumPlayer();
};
```


- 🔧 구현 파일 수정
```cpp
Player::Player(int x, int y, int speed) : x(x), y(y), speed(speed), accessCount(0) {
    std::cout << this << std::endl;
    Player::numPlayer++;
}

void Player::printPosition() const {
    accessCount++; // const 함수지만 mutable 멤버는 변경 가능
    std::cout << "Access #" << accessCount << ": " << x << ", " << y << std::endl;
}
```


## ✅ mutable의 효과
| 멤버 | const 객체에서 변경 가능? | 설명 |
|------------------------|--|---------------------------| 
| 일반 멤버 (x, y, speed) | ❌ | const 함수에서 변경 불가 | 
| mutable 멤버 (accessCount) | ✅ | const 함수에서도 변경 가능 | 
| static 멤버 (numPlayer) | ✅ | 객체와 무관, 클래스 단위로 관리됨 | 



## 📌 사용 예시
```cpp
const Player p{1, 1, 1};
p.printPosition(); // 내부적으로 accessCount++ 가능
```

- p는 const 객체이므로 setPosition() 같은 비-const 함수는 호출 불가
- 하지만 printPosition()은 const 함수이고, 내부에서 accessCount는 mutable이므로 변경 가능

## 🧠 결론
mutable은 const 객체에서도 변경 가능한 예외적인 멤버를 정의할 때 사용됩니다.
이는 객체의 **논리적 불변성(logical constness)**을 유지하면서도, 내부 상태를 유연하게 관리할 수 있게 해줍니다.
