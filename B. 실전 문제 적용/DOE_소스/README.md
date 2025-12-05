
# DOE / ANOM / Response Surface Utilities

This document summarizes the C++ code and tests we built for:

- Taguchi orthogonal arrays (OA)
- ANOM (Analysis of Means) chart data computation
- Quadratic response surface regression (2nd-order model)
- Integration: OA + response + ANOM + response surface
- Test cases verifying each component

All code is ASCII-only (no non-English comments) and is designed to be embedded in a C++ project using Eigen.

---

## 1. Overall Structure

Main components:

1. `orthogonal_array.hpp`  
   - Defines `OrthogonalArray`, `FactorLevels`
   - Provides built-in Taguchi arrays: L4, L8, L9, L18
   - Utility functions to build numeric design matrices from OA

2. `anom.hpp`  
   - Statistical utilities: normal and t quantile approximations
   - ANOM option and result structs
   - `Anom` class: computes ANOM decision limits and outputs SVG and CSV

3. `doe_anom_response.hpp`  
   - Functions connecting OA + response values to factor-wise ANOM

4. `response_surface_quadratic.hpp`  
   - `ResponseSurfaceQuadratic` class: 2nd-order (quadratic) regression using Eigen and QR

5. `doe_full_analysis.hpp`  
   - `run_doe_full_analysis`: wrapper that runs response surface regression + factor-wise ANOM in one call

6. `doe_all_tests.cpp`  
   - Six tests:
     - basic ANOM (equal-n)
     - ANOM with unequal n
     - OA → design
     - OA + response → ANOM
     - Response surface regression
     - Full DOE pipeline with L8 array

---

## 2. Orthogonal Arrays (`orthogonal_array.hpp`)

### 2.1. Data Structures

```cpp
struct OrthogonalArray
{
    int runs;            // number of experimental runs (rows)
    int factors;         // number of factors (columns)
    int levels;          // maximum level count (0..levels-1)
    std::vector<int> data; // row-major: data[run * factors + factor] = level index

    int at(int run, int factor) const {
        return data[run * factors + factor];
    }
};

struct FactorLevels
{
    std::vector<double> levels; // physical numeric levels for one factor
};


OrthogonalArray describes a Taguchi/OA table in terms of integer level indices.

FactorLevels maps each factor’s level index (0,1,2,…) to a physical numeric value (e.g., height = {100, 200, 300}).

2.2. Built-in Taguchi Arrays

All arrays are defined with 0-based levels.

L4(2^3): 4 runs, 3 factors, 2 levels
inline const OrthogonalArray& OA_L4_2_3()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 4;
        o.factors = 3;
        o.levels  = 2;
        o.data = {
            // F1 F2 F3
            0, 0, 0,
            0, 1, 1,
            1, 0, 1,
            1, 1, 0
        };
        return o;
    }();
    return oa;
}

L8(2^7): 8 runs, 7 factors, 2 levels
inline const OrthogonalArray& OA_L8_2_7()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 8;
        o.factors = 7;
        o.levels  = 2;
        o.data = {
            // F1 F2 F3 F4 F5 F6 F7
            0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 1, 1,
            0, 1, 1, 0, 0, 1, 1,
            0, 1, 1, 1, 1, 0, 0,
            1, 0, 1, 0, 1, 0, 1,
            1, 0, 1, 1, 0, 1, 0,
            1, 1, 0, 0, 1, 1, 0,
            1, 1, 0, 1, 0, 0, 1
        };
        return o;
    }();
    return oa;
}

L9(3^4): 9 runs, 4 factors, 3 levels
inline const OrthogonalArray& OA_L9_3_4()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 9;
        o.factors = 4;
        o.levels  = 3;
        o.data = {
            // F1 F2 F3 F4 (1..3 mapped to 0..2)
            0, 0, 0, 0,
            0, 1, 1, 1,
            0, 2, 2, 2,
            1, 0, 1, 2,
            1, 1, 2, 0,
            1, 2, 0, 1,
            2, 0, 2, 1,
            2, 1, 0, 2,
            2, 2, 1, 0
        };
        return oa;
    }();
    return oa;
}

L18(2^1 × 3^7): 18 runs, 8 factors

Factor 1: 2-level

Factors 2-8: 3-level

inline const OrthogonalArray& OA_L18_2_1_3_7()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 18;
        o.factors = 8;
        o.levels  = 3; // maximum levels
        o.data = {
            // F1 F2 F3 F4 F5 F6 F7 F8
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 2, 2, 2, 2, 2, 2,
            0, 1, 0, 0, 1, 1, 2, 2,
            0, 1, 1, 1, 2, 2, 0, 0,
            0, 1, 2, 2, 0, 0, 1, 1,
            0, 2, 0, 1, 0, 2, 1, 2,
            0, 2, 1, 2, 1, 0, 2, 0,
            0, 2, 2, 0, 2, 1, 0, 1,
            1, 0, 0, 2, 2, 1, 1, 0,
            1, 0, 1, 0, 0, 2, 2, 1,
            1, 0, 2, 1, 1, 0, 0, 2,
            1, 1, 0, 1, 2, 0, 2, 1,
            1, 1, 1, 2, 0, 1, 0, 2,
            1, 1, 2, 0, 1, 2, 1, 0,
            1, 2, 0, 2, 1, 2, 0, 1,
            1, 2, 1, 0, 2, 0, 1, 2,
            1, 2, 2, 1, 0, 1, 2, 0
        };
        return o;
    }();
    return oa;
}

2.3. Building Design Matrices

Design matrix: numeric levels for each run.

All factors
std::vector<std::vector<double>>
build_design_from_orthogonal_array(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& factors);


For each run r and factor f, the function:

Looks up level index lev = oa.at(r, f)

Looks up physical level factors[f].levels[lev]

Fills design[r][f] with that value

Result: design[run][factor].

Selected factors only
std::vector<std::vector<double>>
build_design_from_orthogonal_array_for_factors(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& all_levels,
    const std::vector<int>& factor_indices);


Similar to above, but only for the columns listed in factor_indices

Returned design has shape: runs × (number of selected factors)

3. ANOM (Analysis of Means) (anom.hpp)

ANOM is used to compare group means against the grand mean, with decision limits derived from t-distribution.

3.1. Basic Notation

Suppose we have a groups. For group i:

Observations: x_{ij}, j = 1..n_i

Group size: n_i

Group mean:

mean_i = (1 / n_i) * sum_{j=1..n_i} x_{ij}

Grand mean (weighted by group sizes):

N = sum_{i=1..a} n_i
grand_mean = (1 / N) * sum_{i=1..a} (n_i * mean_i)

Pooled within-group sum of squares:

ss_within = sum_{i=1..a} sum_{j=1..n_i} (x_{ij} − mean_i)^2

Degrees of freedom:

df_within = sum_{i=1..a} (n_i − 1)

Pooled within-group variance:

MSE = ss_within / df_within

Pooled within-group standard deviation:

s = sqrt(MSE)

3.2. Quantile Approximations
Normal quantile

normal_quantile_approx(p)

Approximates z = Phi^{-1}(p), where Phi is standard normal CDF.
Implementation uses Moro/Wichura-style rational approximation with different formula for tails and central region.

Used as a building block for t quantiles.

Student t quantile

student_t_quantile_approx(p, df)

Approximates quantile t_p(df) of Student t with df degrees of freedom.

For large df (df > 30):
t_p ≈ z, where z = normal_quantile_approx(p)

For smaller df:
t_p ≈ z + (z^3 + z) / (4 * df)

3.3. ANOM h and t critical
Equal-n Bonferroni h

anom_h_bonferroni_equal_n(alpha, a, n, df)

alpha: global significance (0 < alpha < 1)

a: number of groups

n: group size (all equal)

df: within-group degrees of freedom

Steps:

Bonferroni adjust across groups:
alpha_per_group = alpha / a

Two-sided test → use p = 1 − alpha_per_group / 2

tcrit = t_p(df)

h = tcrit * sqrt((a − 1) / a)

Equal-n ANOM margin per group i:

margin_i = h * s * sqrt(1 / n_i)
(same n_i for all i)

General Bonferroni t critical

anom_tcrit_bonferroni(alpha, a, df)

alpha_per_group = alpha / a

p = 1 − alpha_per_group / 2

tcrit = t_p(df)

Unequal-n margin per group i:

margin_i = tcrit * s * sqrt(1 / n_i)

3.4. Options and Result Structures
struct AnomOptions {
    double alpha = 0.05;
    bool assume_equal_n = true;
    bool bonferroni = true;

    double svg_width  = 900.0;
    double svg_height = 500.0;
    double svg_margin = 60.0;
};

struct AnomGroupResult {
    std::string name;
    int n;
    double mean;
    double margin;
    double UDL;  // upper decision limit
    double LDL;  // lower decision limit
    bool significant_high;
    bool significant_low;
};


assume_equal_n controls whether we try to use equal-n ANOM h.

If assume_equal_n is true but actual group sizes differ, code detects unequal n and falls back to generalized t-based margins.

3.5. Anom Class

Key methods:

class Anom {
public:
    explicit Anom(AnomOptions opt = {});

    void add_group(const std::string& name, const std::vector<double>& values);
    void add_group(const std::string& name, std::initializer_list<double> values);

    void clear();
    void fit();

    double grand_mean() const;
    double s_within() const;
    const std::vector<AnomGroupResult>& results() const;

    void save_csv(const std::string& path) const;
    std::string render_svg() const;
};


Workflow:

Construct Anom with AnomOptions.

Call add_group(name, values) for each group.

Call fit():

computes grand_mean, MSE, s_within, and decision limits.

Use results() to inspect per-group data.

Optionally, call:

save_csv("...") to save table

render_svg() to get a simple ANOM chart as SVG

SVG rendering

X-axis: groups (equally spaced)

Y-axis: response scale

Lines:

Grand mean (dashed blue)

Global min LDL (green)

Global max UDL (red)

Per group:

Mean point (circle, colored: red for high sig, green for low sig, gray otherwise)

Short horizontal UDL/LDL ticks above/below the point

Group name text below X-axis

4. Response Surface Quadratic (response_surface_quadratic.hpp)

This class fits a 2nd-order polynomial model:

Number of factors: k

Model:

y ≈ beta0
+ sum_{i=1..k} beta_i * x_i
+ sum_{i=1..k} beta_ii * x_i^2
+ sum_{1 <= i < j <= k} beta_ij * x_i * x_j

4.1. Basis Vector Construction

For an input vector x of length k, we construct a feature vector phi of length:

m = 1 + k + k + k*(k-1)/2

Order:

Constant term: 1

Linear terms: x_1, ..., x_k

Squared terms: x_1^2, ..., x_k^2

Interaction terms: x_1 x_2, x_1 x_3, ..., x_{k-1} x_k (i < j)

4.2. Least Squares Fitting (QR)
class ResponseSurfaceQuadratic {
public:
    bool fit(const std::vector<std::vector<double>>& design,
             const std::vector<double>& y);

    double predict(const std::vector<double>& x) const;

    int num_factors() const;
    const Eigen::VectorXd& coefficients() const;

private:
    int k_ = 0;
    bool fitted_ = false;
    Eigen::VectorXd beta_;
};


design: N × k array (N runs, k factors)

y: N responses

Steps:

Build Phi (N × m) and Y (N × 1)

Use Eigen’s ColPivHouseholderQR:

beta = argmin ||Phi * beta − Y||_2

Accepts rank-deficient designs (e.g. Taguchi arrays where full quadratic is not fully estimable):

QR returns a least-squares (minimum norm) solution even if rank < m

We can optionally check qr.rank() if we want to detect aliasing

4.3. Prediction

predict(x)

Construct phi(x) in the same order

Return dot product: beta · phi(x)

5. DOE + ANOM Wrapper (doe_anom_response.hpp, doe_full_analysis.hpp)
5.1. From OA + response to ANOM per factor
Anom build_anom_for_factor(
    const OrthogonalArray& oa,
    const std::vector<double>& y,
    int factor_idx,
    const std::string& factor_name,
    const AnomOptions& opt = AnomOptions{});


Input:

oa: orthogonal array

y: length oa.runs response vector

factor_idx: index of factor (0..oa.factors-1)

factor_name: label for that factor (e.g., "A")

Process:

For each run r:

Get level index lev = oa.at(r, factor_idx)

Append y[r] into level_values[lev]

For each level lev:

Create group name: <factor_name>_L<lev+1>

Add group to local Anom instance

Call fit()

Return Anom object

struct FactorAnomResult {
    std::string factor_name;
    Anom anom;
};

std::vector<FactorAnomResult> build_anom_for_all_factors(
    const OrthogonalArray& oa,
    const std::vector<double>& y,
    const std::vector<std::string>& factor_names = {},
    const AnomOptions& opt = AnomOptions{});


For each factor j, builds an Anom via build_anom_for_factor.

factor_names is optional; if empty, names are "A", "B", "C", … by index.

5.2. Full DOE Analysis (RS + ANOM)
struct DoeFullAnalysis {
    ResponseSurfaceQuadratic rs_model;
    std::vector<FactorAnomResult> factor_anoms;
};

DoeFullAnalysis run_doe_full_analysis(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& all_levels,
    const std::vector<int>& factor_indices_for_rs,
    const std::vector<double>& y,
    const std::vector<std::string>& factor_names = {},
    const AnomOptions& anom_opt = AnomOptions{});


Steps:

Response surface design
Use build_design_from_orthogonal_array_for_factors with factor_indices_for_rs
Example: for L8(2^7), we may only use factors A and B (indices {0,1}) for RS.

Fit ResponseSurfaceQuadratic
rs_model.fit(design, y) using QR.

Factor-wise ANOM
factor_anoms = build_anom_for_all_factors(oa, y, factor_names, anom_opt)

Return DoeFullAnalysis containing:

rs_model (global quadratic approximation on selected factors)

factor_anoms (ANOM results for each factor)

6. Test Scenarios (doe_all_tests.cpp)

doe_all_tests.cpp contains 6 tests. Each test exercises a part of the system.

6.1. Helper: approximate equality
static bool approx_equal(double a, double b, double tol = 1e-6) {
    return std::fabs(a - b) <= tol;
}

6.2. Test 1: test_anom_equal_n_basic

Goal:

Verify ANOM on three equal-n groups

Check ordering and basic consistency

Setup:

Options: alpha = 0.05, assume_equal_n = true, bonferroni = true

Groups:

G1: [10.0, 10.1, 9.9, 10.0]
G2: [10.5, 10.6, 10.4, 10.5]
G3: [9.7, 9.8, 9.6, 9.7]

Checks:

grand_mean and s_within are printed

results size = 3

n = 4 for each

means roughly: G3 < G1 < G2

CSV saved to test1_anom_equal_n.csv

SVG saved to test1_anom_equal_n.svg

6.3. Test 2: test_anom_unequal_n

Goal:

Verify ANOM when group sizes differ

Confirm that code gracefully handles unequal n and uses per-group margins

Setup:

Options: same as Test 1.

Groups:

A: 4 points
B: 6 points
C: 3 points

Checks:

results.size() == 3

n values = 4, 6, 3

Means, margins, UDL, LDL, and flags are printed

CSV: test2_anom_unequal_n.csv

SVG: test2_anom_unequal_n.svg

6.4. Test 3: test_orthogonal_array_design

Goal:

Verify OA → design matrix mapping

Setup:

Use OA_L4_2_3 (4 runs, 3 factors, 2 levels)

All factors have levels mapped to {-1, +1}:

FactorLevels[j].levels = { -1.0, +1.0 }

Build design: build_design_from_orthogonal_array(oa, fl)

Checks:

Design size: 4 × 3

Print each run’s factor values

Check some known positions:

First row: (0,0,0) → (-1,-1,-1)

Last row: (1,1,0) → (+1,+1,-1)

6.5. Test 4: test_build_anom_for_factor

Goal:

Connect OA + response → ANOM for a specific factor.

Example: Factor A (index 0) main effect.

Setup:

OA: OA_L8_2_7 (L8)

Response y is generated only from factor A:

If levelA = 0 → mean ≈ 10
If levelA = 1 → mean ≈ 12
Add Gaussian noise with std=0.2

Options: alpha=0.05, equal_n, bonferroni=true

Procedure:

Anom anomA = build_anom_for_factor(oa, y, 0, "A", opt);

Inspect anomA.results():

Groups: "A_L1" and "A_L2"

n (per level) depends on OA structure

Means should show Level 2 > Level 1

Also:

auto all_anoms = build_anom_for_all_factors(oa, y, {"A","B","C","D","E","F","G"}, opt);

all_anoms.size() == 7

all_anoms[0].factor_name == "A"

SVG for factor A: test4_anom_factor_A.svg

6.6. Test 5: test_response_surface_quadratic_fit

Goal:

Check that ResponseSurfaceQuadratic can recover known quadratic coefficients when design is rich enough (full grid).

True model:

Two factors, x1 and x2:

y = b0

b1 * x1

b2 * x2

b11 * x1^2

b22 * x2^2

b12 * x1 * x2

Example coefficients:

b0 = 10.0

b1 = 2.0

b2 = -1.0

b11 = 0.5

b22 = -0.3

b12 = 1.2

Design:

x1, x2 in small grid: i,j from -2 to 2 with step 0.5

For each (x1, x2): evaluate y = true_model(x1, x2)

Fit ResponseSurfaceQuadratic with this design and y

Checks:

coefficients() vector (length 6 for k=2) matches known parameters within tolerance ~1e-6

Predict at a random point and compare y_true vs y_pred

6.7. Test 6: test_doe_full_analysis

Goal:

Run full pipeline on Taguchi L8(2^7):

generate responses from known model depending only on A,B

run run_doe_full_analysis

inspect response surface model and factor-wise ANOM

Setup:

OA: OA_L8_2_7

Factors (A..G) all 2-level, mapped to {-1, +1}

True model uses only factor A (x1) and B (x2):

y = 10

2 * x1

3 * x2

1 * x1^2
− 0.5 * x2^2

noise

Only A,B used for response surface regression:

factor_indices_for_rs = {0,1}

ANOM is computed for all factors A..G.

Procedure:

Construct all_levels mapping each factor to {-1, +1}

Construct y using OA level indices for A,B and true model

Call:

DoeFullAnalysis analysis = run_doe_full_analysis(oa, all_levels, rs_factors, y, factor_names, anom_opt);

Inspect:

analysis.rs_model.coefficients() (size = 6)

For each FactorAnomResult fa in analysis.factor_anoms:

fa.factor_name (e.g., "A")

fa.anom.grand_mean(), fa.anom.s_within()

per-group results: mean, UDL, LDL, significance flags

SVG for each factor: test6_anom_factor_<name>.svg

Expected behavior:

Response surface coefficients approximate the true model, though not exact (L8 is not a full quadratic design).

ANOM should show clear difference for factor A and B (depending on the chosen noise level and main effects), while other factors should have negligible effects.

7. Summary

This DOE toolkit in C++ provides:

Hard-coded Taguchi orthogonal arrays for common designs (L4, L8, L9, L18)

A mapping from symbolic OA (0/1/2 levels) to numeric design points

An ANOM engine that:

computes group means and pooled variance

derives decision limits using t-distribution with optional Bonferroni correction

exports CSV and SVG plots

A quadratic response surface class that:

builds 2nd-order polynomial basis vectors

fits coefficients via robust QR least squares (handles rank-deficient designs)

Integration:

OA + responses → factor-wise ANOM

OA + responses + selected factors → response surface + ANOM in one call

A comprehensive test harness verifying:

ANOM for equal/unequal group sizes

OA → design mapping

OA + response → factor-wise ANOM

Quadratic regression accuracy

Full DOE pipeline behavior

You can extend this framework by:

Adding more orthogonal arrays or custom designs

Restricting response surface basis (main effects only, main + interactions, etc.)

Adding F-tests and p-values on regression coefficients

Generating CSV output for response surface predictions on a grid, to plot 3D surfaces and contour plots in Python or another tool.
