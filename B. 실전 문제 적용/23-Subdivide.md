# 1D Metric Field and Normalized Parameter Division (`divide.md`)

This document describes the algorithm and formulas used in the C++ function:

```cpp
bool Build1DMetricFieldAutoN(
    double L,
    double m_start0,
    double m_end0,
    double m_max0,
    std::vector<double>& metrics,
    std::vector<double>& u,
    double ramp_fraction = 0.25,
    int    max_segments  = 2000,
    double tol_sum       = 1e-9
);
```

The goal is to construct a **1D sizing field** (`metrics`) and corresponding **normalized parameters** (`u`)  
that can be used to subdivide a curve in a reasonable way.

---

## 1. Problem Setup

- We consider a 1D interval (for example, a curve projected to a parameter range) with:
  - Total length: `L > 0`
  - Desired metric (segment length) at the left end: `m_start0 >= 0`
  - Desired metric at the right end: `m_end0 >= 0`
  - Maximum allowed metric anywhere: `m_max0 > 0`

- We want to produce:
  - `metrics[i]` for `i = 0 .. N-1` (N is chosen automatically)
    - Each `metrics[i]` is a segment length
    - All `metrics[i] > 0`
    - All `metrics[i] <= m_max` (after clamping)
    - The sum of all metrics is approximately `L`:
      - `sum_{i=0..N-1} metrics[i] ~= L`
    - The first and last metrics approximate desired end values:
      - `metrics[0] ~ m_start0`
      - `metrics[N-1] ~ m_end0`

  - Normalized parameters `u[j]` for `j = 0 .. N`:
    - `u[0] = 0`
    - `u[N] = 1`
    - `u[j+1] - u[j]` is proportional to `metrics[j] / L`
    - These `u` values define a subdivision of the unit interval `[0,1]`
      that can be mapped onto the actual curve.

- Internally we clamp the input metrics:

```text
eps     = 1e-9
m_max   = max(m_max0, eps)
m_start = clamp(m_start0, eps, m_max)
m_end   = clamp(m_end0,   eps, m_max)
```

- where:

```text
clamp(x, a, b) = min( max(x, a), b )
```

- The parameter `ramp_fraction` controls how much of the parametric domain is used for "ramp" regions:
  
  - On the left:  `u in [0, ramp_fraction]`
  - On the right: `u in [1 - ramp_fraction, 1]`
  - The middle region is a plateau near `m_max`.

- We clamp:

```text
ramp_fraction = clamp(ramp_fraction, 0.0, 0.5)
```

---

## 2. Very Short Length Case

- If the total length `L` is too short to keep both end metrics as they are, we handle a special case:

```text
if L <= m_start + m_end + eps:
    // cannot keep both end metrics as requested
```

### 2.1. Scaling of End Metrics

- We scale the two end metrics proportionally so that their sum is `L`:

```text
s  = L / (m_start + m_end)
h0 = m_start * s
h1 = m_end   * s
```

- We then clamp each to `m_max` (and minimum `eps`), and if the sum deviates from `L`,
- we apply a second scale to correct:

```text
h0 = clamp(h0, eps, m_max)
h1 = clamp(h1, eps, m_max)

sum = h0 + h1
if |sum - L| > tol_sum and sum > eps:
    s2 = L / sum
    h0 *= s2
    h1 *= s2
```

- Finally we set:

```text
metrics[0] = h0
metrics[1] = h1
N = 2
```

### 2.2. Normalized Parameters in Short Case

We build `u` using cumulative sums:

```text
u[0] = 0
u[1] = metrics[0] / L
u[2] = (metrics[0] + metrics[1]) / L
u[N] = 1   // enforced
```

- This completes the very short length case.

---

## 3. General Case

- For the general case we assume:

```text
L > m_start + m_end + eps
```

- Thus there is room for internal segments.

- Define:

```text
sum_end  = m_start + m_end
S_target = L - sum_end   // desired sum of all internal metrics
```

- If `S_target < eps`, we again fall back to the 2-segment case.

---

## 4. Minimum Internal Segment Count

- The maximum metric allowed anywhere is `m_max`. If there are `K` internal segments, then in the best case:

```text
K * m_max >= S_target
```

- Therefore a minimum internal segment count is:

```text
K_min = ceil( S_target / m_max )
K_min = max(K_min, 1)
```

- Total segment count is:

```text
N_min = 2 + K_min
N     = N_min
```

We ensure:

```text
N >= 2
max_segments = max(max_segments, N)
```

---

## 5. Target Profile `d[i]` (Ramp + Plateau)

We construct a target profile `d[i]` for `i = 0..N-1`:

- `d[0] = m_start`
- `d[N-1] = m_end`
- Internal `d[i]` (for `i = 1..N-2`) are built by a ramp function in the parametric domain:

Define a parametric coordinate:

```text
u_i = i / (N - 1),   i = 0..N-1
```

Let:

```text
left_end  = ramp_fraction
right_beg = 1.0 - ramp_fraction
```

Then:

```text
if u_i <= left_end:
    // left ramp from m_start to m_max
    s = u_i / left_end      // 0..1
    t = m_start + s * (m_max - m_start)
else if u_i >= right_beg:
    // right ramp from m_max to m_end
    s = (1.0 - u_i) / ramp_fraction  // 0..1
    t = m_end + s * (m_max - m_end)
else:
    // plateau
    t = m_max
```

- We clamp:

```text
d[i] = clamp(t, eps, m_max)
```

- We then compute the sum:

```text
sumD = sum_{i=0..N-1} d[i]
```

---

## 6. Increasing N Until `sumD >= L`

- We want to be able to **scale internal elements down** (by a factor <= 1) to reach the target length `L` without exceeding `m_max`.
- For this, we need:

```text
sumD >= L
```

- If `sumD < L - tol_sum`, we increase `N` by 1 and rebuild the profile `d`:

```text
while true:
    build_target_profile(N, d, sumD) // using the rules in section 5

    if sumD + tol_sum >= L:
        break   // good enough
    N += 1

    if N > max_segments:
        return false // cannot satisfy constraints with given max_segments
```

- After this loop:

- `N` is chosen
- `d[0] = m_start`, `d[N-1] = m_end`
- `sumD >= L` (within tolerance)

---

## 7. Scaling the Internal Part to Match `L`

- We then form the final `metrics[i]`:

```text
metrics[0]     = m_start
metrics[N - 1] = m_end
```

- Let:

```text
sumD_end   = d[0] + d[N-1]
sumD_inner = sumD - sumD_end
S_target   = L - sum_end  // from section 3
```

### 7.1. Internal Sum is Almost Zero

- If:

```text
sumD_inner <= eps
```

- we use a uniform internal metric:

```text
if N > 2:
    m_inner = S_target / (N - 2)
    m_inner = clamp(m_inner, eps, m_max)
    metrics[i] = m_inner  for i = 1..N-2
```

### 7.2. General Internal Scaling

- Otherwise, we use a scale factor:

```text
scale = S_target / sumD_inner
```

- Since `sumD >= L` and `sumD_end` is fixed, we expect `scale <= 1`.  
- If due to rounding:

```text
scale > 1 + small_tolerance
```

- we clamp:

```text
scale = 1
```

Then:

```text
for i in 1..N-2:
    metrics[i] = d[i] * scale
    metrics[i] = clamp(metrics[i], eps, m_max)
```

---

## 8. Final Sum Correction

- We compute:

```text
sum_final = sum_{i=0..N-1} metrics[i]
diff      = L - sum_final
```

If the absolute difference is larger than tolerance and `N > 2`:

```text
if |diff| > tol_sum and N > 2:
    per = diff / (N - 2)
    for i in 1..N-2:
        metrics[i] += per
        metrics[i] = clamp(metrics[i], eps, m_max)
```

- This distributes the residual error uniformly across the internal segments while keeping them within the `[eps, m_max]` range.

---

## 9. Building Normalized Parameter Array `u`

- Once `metrics[i]` are finalized, we build a normalized parametric partition `u[j]`:

```text
u has size N+1

cum = 0
u[0] = 0
for i in 0..N-1:
    cum += metrics[i]
    u[i+1] = cum / L
```

- Finally we enforce:

```text
u[0] = 0
u[N] = 1
```

- The u-array is thus a normalized cumulative length:
  - `u[0] = 0`
  - `u[1] = metrics[0] / L`
  - ...
  - `u[i+1] - u[i] = metrics[i] / L`
  - `u[N] = 1`

These `u` values can later be used as approximate subdivision parameters for a curve.  
If the actual curve length is used, you can re-adjust u (via arc-length reparameterization)  
while keeping the **count and approximate distribution** provided by this algorithm.

---

## 10. Summary of Constraints

- The algorithm guarantees:

- 1. `metrics[i] > 0` for all i
- 2. `metrics[i] <= m_max` for all i
- 3. `metrics[0]` and `metrics[N-1]` stay as close as possible to `m_start0` and `m_end0`  
   (exact in the general case, proportionally scaled in very short cases)
- 4. The total sum of metrics approximates `L`:

   ```text
   sum_{i=0..N-1} metrics[i] ≈ L
   ```

   with the residual error pushed inside tolerance `tol_sum`.

- 5. The number of segments `N` is **automatically chosen** based on the relation between `L`, `m_start`, `m_end`, and `m_max`.
- 6. The normalized parameters `u` form a valid partition of `[0,1]` aligned with the metric distribution.

- You can now treat `metrics` as a 1D sizing field and `u` as the corresponding subdivision parameters,  
  and combine this with an existing arc-length based inverse mapping to refine the curve subdivision if needed.

### Subdivide Image
![Subdivide Result](/image/subdivide_result.png)

---

## 소스 코드
```cpp
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

static inline double clamp_val(double x, double a, double b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

bool Build1DMetricFieldAutoN(
        double L,
        double m_start0,
        double m_end0,
        double m_max0,
        std::vector<double>& metrics,
        std::vector<double>& u,
        double ramp_fraction = 0.25,
        int    max_segments  = 2000,
        double tol_sum       = 1e-9
)
{
    metrics.clear();
    u.clear();

    const double eps = 1e-9;
    if (!(L > 0.0) || !(m_max0 > 0.0))
        return false;

    double m_max = std::max(m_max0, eps);
    double m_start = clamp_val(m_start0, eps, m_max);
    double m_end   = clamp_val(m_end0,   eps, m_max);

    ramp_fraction = clamp_val(ramp_fraction, 0.0, 0.5);

    // ------------------------------------------------------------
    // Case 1: Very short length
    // ------------------------------------------------------------
    if (L <= m_start + m_end + eps)
    {
        double s = L / (m_start + m_end);
        double h0 = m_start * s;
        double h1 = m_end   * s;

        h0 = clamp_val(h0, eps, m_max);
        h1 = clamp_val(h1, eps, m_max);

        double sum = h0 + h1;
        if (std::fabs(sum - L) > tol_sum && sum > eps)
        {
            double s2 = L / sum;
            h0 *= s2;
            h1 *= s2;
        }

        metrics = {h0, h1};

        // Build normalized u
        u.resize(3);
        double cum = 0.0;
        u[0] = 0.0;
        cum += metrics[0];
        u[1] = cum / L;
        cum += metrics[1];
        u[2] = cum / L;
        u[0] = 0.0;
        u[2] = 1.0;

        return true;
    }

    // ------------------------------------------------------------
    // General case
    // ------------------------------------------------------------
    double sum_end = m_start + m_end;
    double S_target = L - sum_end;

    if (S_target < eps)
    {
        metrics = {m_start, clamp_val(L - m_start, eps, m_max)};
        u.resize(3);

        double cum = 0.0;
        u[0] = 0.0;
        cum += metrics[0];
        u[1] = cum / L;
        cum += metrics[1];
        u[2] = cum / L;

        u[0] = 0.0;
        u[2] = 1.0;
        return true;
    }

    // minimal internal segment count
    int k_min = (int)std::ceil(S_target / m_max);
    k_min = std::max(k_min, 1);

    int N = 2 + k_min;
    max_segments = std::max(max_segments, N);

    std::vector<double> d;
    d.reserve(max_segments);

    auto build_profile = [&](int Nlocal, std::vector<double>& dvec, double& sumD)
    {
        dvec.assign(Nlocal, 0.0);
        dvec[0] = m_start;
        dvec[Nlocal - 1] = m_end;

        double left_end = ramp_fraction;
        double right_beg = 1.0 - ramp_fraction;

        for (int i = 1; i < Nlocal - 1; ++i)
        {
            double uu = (double)i / (double)(Nlocal - 1);
            double t = m_max;

            if (ramp_fraction > 0.0)
            {
                if (uu <= left_end)
                {
                    double s = uu / left_end;
                    t = m_start + s * (m_max - m_start);
                }
                else if (uu >= right_beg)
                {
                    double s = (1.0 - uu) / ramp_fraction;
                    t = m_end + s * (m_max - m_end);
                }
                else {
                    t = m_max;
                }
            }
            else {
                t = m_max;
            }

            dvec[i] = clamp_val(t, eps, m_max);
        }

        sumD = 0.0;
        for (double v : dvec) sumD += v;
    };

    double sumD = 0.0;

    while (true)
    {
        if (N > max_segments)
            return false;

        build_profile(N, d, sumD);
        if (sumD + tol_sum >= L)
            break;

        ++N;
    }

    metrics.assign(N, 0.0);
    metrics[0] = m_start;
    metrics[N - 1] = m_end;

    double sumD_end = d[0] + d[N - 1];
    double sumD_inner = sumD - sumD_end;

    if (sumD_inner <= eps)
    {
        if (N > 2)
        {
            double m_inner = S_target / (N - 2);
            m_inner = clamp_val(m_inner, eps, m_max);
            for (int i = 1; i < N - 1; ++i)
                metrics[i] = m_inner;
        }
    }
    else
    {
        double scale = S_target / sumD_inner;
        if (scale > 1.0 + 1e-6)
            scale = 1.0;

        for (int i = 1; i < N - 1; ++i)
        {
            metrics[i] = clamp_val(d[i] * scale, eps, m_max);
        }
    }

    double sum_final = 0;
    for (double v : metrics) sum_final += v;

    double diff = L - sum_final;

    if (std::fabs(diff) > tol_sum && N > 2)
    {
        double per = diff / (N - 2);
        for (int i = 1; i < N - 1; ++i)
        {
            metrics[i] = clamp_val(metrics[i] + per, eps, m_max);
        }
    }

    // Build final u
    u.resize(N + 1);
    double cum = 0.0;
    u[0] = 0.0;
    for (int i = 0; i < N; ++i)
    {
        cum += metrics[i];
        u[i + 1] = cum / L;
    }
    u[0] = 0.0;
    u[N] = 1.0;

    return true;
}
```
```cpp
// ---- include your function header here ----
// or paste the full Build1DMetricFieldAutoN implementation above this line

// For example:
// bool Build1DMetricFieldAutoN(...);
#include <iostream>
#include <vector>
#include <iomanip>

// ---- include your function header here ----
// or paste the full Build1DMetricFieldAutoN implementation above this line

// For example:
// bool Build1DMetricFieldAutoN(...);

int main()
{
    double L        = 10.0;
    double m_start  = 0.1;
    double m_end    = 0.1;
    double m_max    = 1.0;

    std::vector<double> metrics;
    std::vector<double> u;

    bool ok = Build1DMetricFieldAutoN(
            L,
            m_start,
            m_end,
            m_max,
            metrics,
            u,
            0.25,      // ramp_fraction
            2000,      // max_segments
            1e-9       // tol_sum
    );

    if (!ok)
    {
        std::cout << "Build1DMetricFieldAutoN FAILED\n";
        return 0;
    }

    int N = (int)metrics.size();

    std::cout << "-------------------------------------------\n";
    std::cout << "RESULT (C++): N = " << N << " segments\n";
    std::cout << "-------------------------------------------\n";

    std::cout << std::fixed << std::setprecision(12);

    // Print metrics
    std::cout << "\nmetrics:\n";
    for (int i = 0; i < N; ++i)
    {
        std::cout << "i=" << i << "  h=" << metrics[i] << "\n";
    }

    // Print sum
    double sum = 0.0;
    for (double v : metrics) sum += v;
    std::cout << "\nsum(metrics) = " << sum << "\n";
    std::cout << "target L     = " << L << "\n";

    // Print u[]
    std::cout << "\nu (normalized positions):\n";
    for (int i = 0; i <= N; ++i)
    {
        std::cout << "u[" << i << "] = " << u[i] << "\n";
    }

    return 0;
}
```

```
-------------------------------------------
RESULT (C++): N = 14 segments
-------------------------------------------
metrics:
i=0  h=0.100000000000
i=1  h=0.372248062016
i=2  h=0.645736434109
i=3  h=0.919224806202
i=4  h=0.987596899225
i=5  h=0.987596899225
i=6  h=0.987596899225
i=7  h=0.987596899225
i=8  h=0.987596899225
i=9  h=0.987596899225
i=10  h=0.919224806202
i=11  h=0.645736434109
i=12  h=0.372248062016
i=13  h=0.100000000000

sum(metrics) = 10.000000000000
target L     = 10.000000000000

u (normalized positions):
u[0] = 0.000000000000
u[1] = 0.010000000000
u[2] = 0.047224806202
u[3] = 0.111798449612
u[4] = 0.203720930233
u[5] = 0.302480620155
u[6] = 0.401240310078
u[7] = 0.500000000000
u[8] = 0.598759689922
u[9] = 0.697519379845
u[10] = 0.796279069767
u[11] = 0.888201550388
u[12] = 0.952775193798
u[13] = 0.990000000000
u[14] = 1.000000000000
```

