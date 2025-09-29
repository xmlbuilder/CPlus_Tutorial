# C++에서 friend 키워드의 의미와 사용법

## 🔍 기본 개념
- friend는 클래스의 캡슐화를 유지하면서도 특정 외부 함수나 클래스가 private 또는 protected 멤버에 접근할 수 있도록 허용하는 키워드입니다.
- friend는 접근 권한만 부여하며, 상속이나 멤버 관계는 형성하지 않습니다.

## 🧱 friend 선언의 종류
| 선언 방식 | 설명 |
|----------|-----| 
| friend void func(); | 특정 외부 함수에게 접근 권한 부여 | 
| friend class ClassName; | 특정 클래스 전체에게 접근 권한 부여 | 
| friend Type::member; | 특정 멤버 함수에게만 권한 부여 (C++11 이후) | 



## 📌 핵심 규칙
- A가 B의 friend일 때, B는 A의 멤버에 접근할 수 있지만 역은 성립하지 않습니다.
- friend 관계는 단방향입니다.
- friend 관계는 전이되지 않습니다.
예: A가 B의 friend이고, B가 C의 friend여도 A는 C의 friend가 아님

## 🧪 예제 분석
```cpp
class Player {
private:
    int x, y;
    int speed;

    friend void displayPlayer(const Player& player); // 외부 함수에게 권한 부여
    friend class Game; // Game 클래스에게 전체 접근 권한 부여

    void play() {
        cout << "play" << endl;
    }
};
```

## ✅ 외부 함수 접근
```cpp
void displayPlayer(const Player& p){
    cout << p.x << ", " << p.y << endl; // private 멤버 접근 가능
}
```


## ✅ friend 클래스 접근
```cpp
class Game {
public:
    void setPlay(Player& player){
        player.play(); // private 함수 호출 가능
    }
};
```



## 🧠 friend 키워드의 장단점
### 👍 장점
- 테스트 코드나 디버깅 도구에서 유용
- 연산자 오버로딩 시 외부 함수가 내부 상태에 접근 가능
- 특정 클래스 간의 밀접한 협력 관계를 표현 가능
### 👎 단점
- 캡슐화 원칙을 약화시킬 수 있음
- 남용 시 코드의 의존성이 증가하고 유지보수가 어려워짐

## 📘 요약
| 항목 | 설명 |
|------|------| 
| 목적 | 특정 외부 함수/클래스에 private 접근 권한 부여 | 
| 방향성 | 단방향 (역방향은 자동으로 friend 아님) | 
| 전이성 | 없음 (friend 관계는 전이되지 않음) | 
| 사용 예 | 연산자 오버로딩, 테스트, 밀접한 협력 클래스 | 

---


