## 🎲 C++ Random Number Generation Guide

- 난수(Random number) 생성은 일반적으로 확률 값이 필요한 경우에
사용됩니다.
- 프로그래밍을 처음 배울 때는 **로또 번호 생성기** 제작 예제로 많이
활용되며, 암호화 등 다양한 분야에서도 쓰입니다.


### 📌 기존 C 표준 난수 생성기

``` cpp
#include <stdlib.h>
#include <time.h>
#include <iostream>

int main()
{
    srand((int)time(0));

    int nRand1 = rand();        // 임의의 정수
    int nRand2 = rand() % 11;   // 0~10 범위

    std::cout << nRand1 << std::endl;
    std::cout << nRand2 << std::endl;

    return 0;
}
```

-   `rand()`는 0부터 `RAND_MAX`까지의 값을 반환합니다.
-   특정 범위 값은 **나머지 연산** 으로 제한합니다.



### 📌 C++11 표준 난수 라이브러리

- C++11부터는 `<random>` 헤더를 통해 더 정교한 난수 생성기가 제공됩니다.
- 구조는 크게 **엔진(Generators)** 과 **분포(Distributions)** 로 나뉩니다.
    - **Generators**: 균일하게 분포된 난수 생성기
    - **Distributions**: 원하는 확률 분포로 변환


### 📌 로또 번호 생성기 예제

``` cpp
#include <random>
#include <chrono>
#include <set>
#include <iostream>

int main()
{
    auto current = std::chrono::system_clock::now();
    auto duration = current.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::mt19937_64 genMT(millis);                       // Mersenne Twister 엔진
    std::uniform_int_distribution<__int64> uniformDist(1, 45); // 1~45 범위

    std::cout << "Min Value : " << uniformDist.min() << std::endl;
    std::cout << "Max Value : " << uniformDist.max() << std::endl;

    std::set<__int64> stLotto;
    while (stLotto.size() < 6)
        stLotto.insert(uniformDist(genMT));

    for (auto nVal : stLotto)
        std::cout << nVal << std::endl;

    return 0;
}
```

- `std::mt19937_64` : Mersenne Twister 기반 엔진
- `std::uniform_int_distribution` : 균등 분포 정수 난수 생성
- `std::set` : 중복 제거 후 자동 정렬

### 📌 가중치 랜덤 (Discrete Distribution)

- 특정 값에 가중치를 두고 싶을 때 `std::discrete_distribution`을
사용합니다.
- 예: 경품 추첨에서 1등은 10%, 2등은 30%, 꽝은 60% 확률.

``` cpp
#include <random>
#include <vector>

template <typename T>
int MakeRandNum(std::vector<T> vecProb)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<T> d{ vecProb.begin(), vecProb.end() };
    return d(gen);
}
```


### 📌 다양한 분포 지원

``` cpp
#include <chrono>
#include <iostream>
#include <random>

int main() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine eng(seed);

    std::uniform_int_distribution<int> distr(0, 5);   // 균등 분포 (정수)
    std::uniform_real_distribution<double> distR(0, 5); // 균등 분포 (실수)
    std::poisson_distribution<int> distR2(1.0);      // 포아송 분포

    std::cout << distr(eng) << std::endl;
    std::cout << distR(eng) << std::endl;
    std::cout << distR2(eng) << std::endl;
}
```

- **균등 분포 (Uniform)** : 특정 범위 내에서 동일 확률
- **이산 분포 (Discrete)** : 각 값에 지정된 가중치 확률
- **정규 분포 (Normal/Gaussian)** : 평균과 표준편차를 따르는 분포
- **포아송 분포 (Poisson)** : 발생 횟수 기반 분포


### 📌 정리
- C 표준 `rand()` : 단순, 범위 제한은 `%` 연산
- C++11 `<random>` : 엔진 + 분포 구조로 다양한 난수 지원
- `std::mt19937` : 빠르고 신뢰성 높은 엔진
- `std::discrete_distribution` : 가중치 기반 난수

- C++11 random 라이브러리를 사용하면 **더 정밀하고 다양한 확률 분포 난수** 를 쉽게 생성할 수 있습니다.

---

