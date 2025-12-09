# unique_ptr / shared_ptr / weak_ptr

## 핵심 요약:
- unique_ptr → 단일 소유권, 복사 불가, 이동만 가능.
- shared_ptr → 다중 소유권, 참조 카운트 기반, 마지막 소유자가 사라질 때 해제.
- weak_ptr → 소유권 없음, shared_ptr를 관찰만 함, 순환 참조 방지용.

## 🔹 스마트 포인터 비교

| 스마트 포인터 | 소유권 | 복사 | 이동 | 공유 | 해제 시점 | 기타 |
|---------------|--------|------|------|------|-----------|------|
| unique_ptr    | 단일   | 불가 | std::move | 불가 | 소유자 소멸 시 | 가장 가볍고 빠름 |
| shared_ptr    | 다중   | 가능 | 가능 | 가능 | 참조 카운트 0 시 | 순환 참조 주의 |
| weak_ptr      | 없음   | 불가 | 불가 | shared_ptr 관찰 | lock()으로 shared_ptr 변환 | 순환 참조 방지 |

## 🔹 사용 예시
### 1. unique_ptr
```cpp
std::unique_ptr<Resource> res = std::make_unique<Resource>();
res->doSomething();
// 다른 곳으로 소유권 이동
std::unique_ptr<Resource> res2 = std::move(res);
```
- 👉 단일 소유권을 보장. 함수 반환값으로 자원 전달 시 자주 사용.

### 2. shared_ptr
```cpp
std::shared_ptr<Resource> res1 = std::make_shared<Resource>();
std::shared_ptr<Resource> res2 = res1; // 참조 카운트 증가
std::cout << res1.use_count(); // 2
```
- 👉 여러 객체가 같은 리소스를 공유해야 할 때 적합.

### 3. weak_ptr
```cpp
std::shared_ptr<Resource> res = std::make_shared<Resource>();
std::weak_ptr<Resource> weakRes = res;

if (auto spt = weakRes.lock()) { // 안전하게 접근
    spt->doSomething();
}
```

- 👉 순환 참조 방지. 예를 들어 부모-자식 관계에서 서로 shared_ptr을 가지면 해제가 안 되는데,  
  한쪽을 weak_ptr로 바꿔 해결.

## 🔹 선택 가이드
- 리소스를 한 곳에서만 관리 → unique_ptr
- 여러 곳에서 공유해야 함 → shared_ptr
- 공유는 필요 없고 단순히 관찰만 → weak_ptr

- 👉 정리하면, 기본적으로 unique_ptr을 우선 사용하고, 정말 공유가 필요할 때만 shared_ptr,  
  그리고 순환 참조가 우려될 때 weak_ptr을 쓰는 것이 가장 안전한 패턴입니다.



## 🔹 현실적인 사용 시나리오
### 1. unique_ptr — 소유권이 명확한 경우
- 예: 게임 엔진에서 총알 객체는 발사된 총알이 소멸될 때만 없어져야 합니다.
```cpp
class Bullet {
public:
    void update() { /* 위치 갱신 */ }
};

std::vector<std::unique_ptr<Bullet>> bullets;

void shoot() {
    bullets.push_back(std::make_unique<Bullet>());
}

void updateAll() {
    for (auto& b : bullets) b->update();
}
```
- 👉 Bullet은 게임 엔진이 단독으로 관리하므로 unique_ptr이 적합합니다.

### 2. shared_ptr — 여러 곳에서 공유해야 하는 경우
- 예: GUI 시스템에서 위젯을 여러 컨테이너가 공유.
```cpp
class Widget {
public:
    void draw() { /* 그리기 */ }
};

auto w = std::make_shared<Widget>();

std::vector<std::shared_ptr<Widget>> panels;
panels.push_back(w);

std::vector<std::shared_ptr<Widget>> dialogs;
dialogs.push_back(w);

w->draw();
```
- 👉 Widget은 여러 UI 컨테이너가 동시에 참조하므로 shared_ptr이 필요합니다.

### 3. weak_ptr — 순환 참조 방지
- 예: 부모-자식 관계 트리 구조
```cpp
struct Node {
    std::string name;
    std::vector<std::shared_ptr<Node>> children;
    std::weak_ptr<Node> parent; // 순환 참조 방지
};

auto root = std::make_shared<Node>();
auto child = std::make_shared<Node>();

root->children.push_back(child);
child->parent = root; // weak_ptr로 연결
```

- 👉 만약 parent도 shared_ptr이었다면, root와 child가 서로 참조 카운트를 유지해서  
  메모리 해제 불가 문제가 생깁니다. weak_ptr을 쓰면 관찰만 하고 소유하지 않으므로 안전합니다.

## 🔹 정리된 현실적 가이드
- 리소스가 한 곳에서만 관리 → unique_ptr
- 여러 객체가 같은 리소스를 공유 → shared_ptr
- 공유는 필요하지만 순환 참조 위험 → weak_ptr

- 👉 즉, 기본은 unique_ptr, 공유가 필요할 때만 shared_ptr,  
  그리고 트리/그래프 구조에서는 반드시 weak_ptr을 섞어야 합니다.
