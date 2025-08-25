# 📘 1. 배경 – 적분과 ODE의 차이

### 적분 문제

$\[ I = \int_a^b f(x)\, dx \]$

- 함수 \(f(x)\)를 여러 점에서 평가해 면적을 근사.


### 초기값 문제 (ODE)

$\[ y'(t) = f(t,y), \quad y(t_0) = y_0 \]$

- 적분과 비슷해 보이지만, 여기서는 미지수 $\(y(t)\)$ 자체를 수치적으로 추적해야 함.  
- 즉, $\(f\)$ 를 직접 적분하는 게 아니라, **미분방정식의 해를 적분적 접근으로 근사**.

---

# 📘 2. Trapezoidal / Simpson과 ODE 해법

### Trapezoidal rule applied to ODE

$\[ y_{n+1} = y_n + \frac{h}{2} \big( f(t_n,y_n) + f(t_{n+1}, y_{n+1}) \big) \]$

- Implicit method: \(y_{n+1}\)이 양변에 있음 → 해 찾기 위해 보통 Newton iteration 필요.

---

### Simpson’s rule applied to ODE

$\[ y_{n+1} = y_n + \frac{h}{6} \Big(f(t_n,y_n) + 4 f(t_{n+\tfrac{1}{2}}, y_{n+\tfrac{1}{2}}) + f(t_{n+1}, y_{n+1}) \Big) \]$

- 역시 **암시적(implicit)** → $\(y_{n+1}\)$ 을 포함하므로 계산이 번거롭다.

---

# 📘 3. Runge–Kutta (Explicit RK)

Runge–Kutta는 사실상 Simpson/Trapezoid 아이디어를 ODE에 맞게 **명시적(explicit)**으로 재구성한 것.

### 4차 Runge–Kutta (RK4) 수식

스텝 크기 $\(h\)$ , 현재 상태 $\(y_n\)$ , 시간 $\(t_n\)$ :

$$
\[
\begin{aligned}
k_1 &= f(t_n, y_n), \\
k_2 &= f\left(t_n + \tfrac{h}{2}, \, y_n + \tfrac{h}{2}k_1\right), \\
k_3 &= f\left(t_n + \tfrac{h}{2}, \, y_n + \tfrac{h}{2}k_2\right), \\
k_4 &= f\left(t_n + h, \, y_n + h k_3\right), \\
y_{n+1} &= y_n + \tfrac{h}{6}(k_1 + 2k_2 + 2k_3 + k_4).
\end{aligned}
\]
$$

- $\(k_1, k_2, k_3, k_4\)$ 는 서로 다른 시점에서의 “슬로프(derivative)”  
- 가중합을 통해 Simpson 공식과 비슷한 포물선 보간을 흉내냄.  
- 하지만 **명시적(implicit equation 없이 계산 가능)**.

---

# 📘 4. 차이점 요약

| 구분 | Trapezoid / Simpson (적분) | Runge–Kutta (ODE) |
|------|-----------------------------|--------------------|
| 문제 | $\(\int f(x)dx\)$ | $\(y'(t) = f(t,y)\)$ |
| 근사 | 사다리꼴(1차), 포물선(2차) | 다중 슬로프 평가 후 가중합 |
| 성질 | 수치적분 전용 | ODE 해법 (적분식 재구성) |
| 암시성 | Trapezoid, Simpson은 implicit 형태로 확장됨 | RK는 explicit 방식으로 재구성 가능 |
| 정확도 | Trapezoid: 2차, Simpson: 4차 | RK4: 4차, RK45: 5차 적응적 스텝 |

---

# 📘 5. 직관 그림

- **Trapezoid**: 처음/끝 기울기 평균으로 적분  
- **Simpson**: 처음/중간/끝 세 점으로 포물선 적분  
- **RK4**: “처음, 중간, 중간, 끝” 4번의 slope 평가 → Simpson을 explicit하게 바꾼 버전

---

✅ **정리:**  
- Trapezoid/Simpson은 적분 공식을 ODE에 억지로 적용하면 implicit → 계산이 무겁다.  
- Runge–Kutta는 이를 explicit 형태로 바꿔 ODE에 맞게 설계된 방법.  
- 그래서 ODE 해석에는 RK 계열이 주로 쓰인다.
