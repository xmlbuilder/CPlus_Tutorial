## 🎲 최신 난수 생성 기술
### 📌 기존 방식: rand()
```cpp
#include <cstdlib>
#include <ctime>

int main() {
    srand((unsigned)time(nullptr)); // seed 설정
    for (int i = 0; i < 10; i++) {
        std::cout << rand() << std::endl; // 0 ~ RAND_MAX 사이의 난수
    }
}
```
- 단점:
- 예측 가능성 있음 (보안에 취약)
- 난수 품질이 낮음
- 병렬 처리에 부적합

### 📌 최신 방식: <random> 헤더 사용
- C++11부터 도입된 <random>은 다양한 난수 엔진과 분포(distribution)를 제공해 훨씬 정밀하고 안전한 난수 생성이 가능해짐.
```cpp
#include <iostream>
#include <random>

int main() {
    std::random_device rd; // 하드웨어 기반 seed (가능한 경우)
    std::mt19937 gen(rd()); // Mersenne Twister 엔진
    std::uniform_int_distribution<> dist(0, 100); // 0~100 사이의 균등 분포

    for (int i = 0; i < 10; i++) {
        std::cout << dist(gen) << std::endl;
    }
}
```

- 장점:
- 다양한 엔진: mt19937, default_random_engine, xorshift, WELL
- 다양한 분포: uniform, normal, poisson, binomial 등
- 보안성 높은 난수: std::random_device 사용 시

### 📌 보안용 난수: Cryptographically Secure RNG
- std::random_device는 시스템의 하드웨어 기반 엔트로피를 사용해 **진짜 난수(TRNG)** 를 생성할 수 있음.
- 암호화 키 생성, 인증 토큰 등에 사용됨.

### 📌 고급 기술: 양자 난수 생성기 (Quantum RNG)
- 양자역학 기반으로 진정한 무작위성을 보장.
- NIST와 University of Colorado가 개발한 CURBy 시스템은 블록체인으로 검증 가능한 난수 생성도 가능.
- Nature에 발표된 최신 연구에서는 양자 얽힘을 이용해 완전히 추적 가능한 난수 생성기를 구현.

### 📌 난수 품질 테스트
- Diehard Test Suite, TestU01, NIST SP800-22 등 다양한 통계적 테스트를 통해 난수의 품질을 평가할 수 있음.

### 📌 요약
| 방식 | 특징 | 용도 |
|-----|------|-----| 
| rand() | 단순, 예측 가능 | 간단한 시뮬레이션 | 
| <random> | 다양한 엔진과 분포 | 통계, 게임, 시뮬레이션 | 
| random_device | 하드웨어 기반 | 보안, 인증 | 
| Quantum RNG | 진정한 무작위성 | 암호화, 공공 추첨 | 

---



