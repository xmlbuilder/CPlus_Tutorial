# 🧵 STL 이전의 수제 링크드 리스트 예제 (std::list 대체 구현)

## **배경**  
- 표준 라이브러리(STL)가 없거나 제한적이던 시절, 개발자들은 `std::list`가 제공하는 기능을 직접 **포인터와 구조체**로 구현해 사용했습니다.  
- 아래 예제는 **단일 연결 리스트(singly linked list)**를 수작업으로 만드는 전형적인 방식입니다.

---

## 1) 원본 예제 코드 (설명 주석 추가)

```cpp
#include <iostream>
using namespace std;

struct I2DNODE {
    int      id {0};
    I2DNODE* next {nullptr};
};

int main() {
    // 더미 head를 만들고 새 노드들을 앞쪽에 붙여 나감 (push_front)
    I2DNODE* head = new I2DNODE;    // id=0, tail 역할처럼 사용
    I2DNODE* data1 = new I2DNODE;   // 노드 1
    data1->id = 1;
    data1->next = head;             // 1 -> 0
    head = data1;                   // head = 1

    I2DNODE* data2 = new I2DNODE;   // 노드 2
    data2->id = 2;
    data2->next = head;             // 2 -> 1 -> 0
    head = data2;                   // head = 2

    // 순회(iteration)
    for (auto itr = head; itr != nullptr; itr = itr->next) {
        cout << itr->id << endl;
    }
    // 출력: 2, 1, 0

    // 맨 앞 노드 삭제(pop_front)
    I2DNODE* delNode = head;
    head = head->next;
    delete delNode;

    // 다시 순회
    for (auto itr = head; itr != nullptr; itr = itr->next) {
        cout << itr->id << endl;
    }
    // 출력: 1, 0
}
```

### 동작 요약
- `push_front`: 새 노드를 만들어 `head` 앞에 연결
- 순회: `for (node=head; node; node=node->next)`
- `pop_front`: `head`를 한 칸 이동시키고 원래 노드 `delete`

---

## 2) 이런 방식의 **문제점**
- **메모리 관리**: `new`/`delete`가 산재 → 누수·이중 해제 위험
- **예외 안전성 부족**: 예외 발생 시 해제 경로 누락 가능
- **복사/이동 의미론 부재**: 소유권 규칙이 불명확
- **반복자/참조 안정성** 고려 어려움
- **기능 부족**: 사이즈 추적, 양방향 순회, 범위 기반 알고리즘 호환 등 불편

> 그래서 표준 컨테이너 `std::list`(이중 연결 리스트)가 등장해 **검증된 구현**, **예외 안전성**,
> **표준 알고리즘 호환**을 제공합니다.

---

## 3) 현대 C++로 조금 더 안전하게 (RAII / `unique_ptr`)

> 교육용 예시: 여전히 학습 목적의 단일 연결 리스트지만, **스마트 포인터**를 사용해 리소스 안전성을 높입니다.

```cpp
#include <iostream>
#include <memory>

struct Node {
    int value{};
    std::unique_ptr<Node> next;
    explicit Node(int v) : value(v) {}
};

struct SinglyList {
    std::unique_ptr<Node> head;

    void push_front(int v) {
        auto n = std::make_unique<Node>(v);
        n->next = std::move(head);
        head = std::move(n);
    }

    bool pop_front(int* out = nullptr) {
        if (!head) return false;
        if (out) *out = head->value;
        head = std::move(head->next); // 앞 노드 자동 해제
        return true;
    }

    void print() const {
        for (auto* p = head.get(); p; p = p->next.get())
            std::cout << p->value << " ";
        std::cout << "\n";
    }
};

int main() {
    SinglyList lst;
    lst.push_front(0);
    lst.push_front(1);
    lst.push_front(2);
    lst.print();     // 2 1 0
    int x;
    lst.pop_front(&x);
    lst.print();     // 1 0
}
```

- `unique_ptr` 덕분에 노드 수명은 **컨테이너가 자동 관리**
- `delete` 호출이 사라져 **누수/이중 삭제 위험 감소**

---

## 4) 같은 작업을 `std::list`로 구현하면

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> lst;
    lst.push_front(0);
    lst.push_front(1);
    lst.push_front(2);

    for (int v : lst) std::cout << v << " ";
    std::cout << "\n"; // 2 1 0

    lst.pop_front();
    for (int v : lst) std::cout << v << " ";
    std::cout << "\n"; // 1 0
}
```

- **간단·안전·표준 호환**: 범용 알고리즘과 반복자 카테고리 지원, 예외 안전성이 우수

---

## 5) 언제 커스텀 리스트를 쓰나?
- **학습/실험**: 자료구조 학습, 메모리 모델 실험
- **하드 제약**: 임베디드/커널 등 표준 라이브러리 사용 불가 환경
- **특수 요구**: 커스텀 메모리 풀, lock-free, intrusive list 등 특화된 시나리오

> 그 외 일반 애플리케이션에서는 **표준 컨테이너 사용 권장**

---

## 6) 요약
- 옛날 방식: **raw pointer**로 수제 리스트 → 위험 요소 많음
- 현대 대안: **스마트 포인터(RAII)** 또는 **`std::list`** 사용
- 안전성/가독성/유지보수성을 고려하면 실무에선 표준 컨테이너가 정답
