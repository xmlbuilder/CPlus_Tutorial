// #include "Anom_Utils.h"
// #include <iostream>
//
// void test_equal_n_simple() {
//     std::cout << "=== test_equal_n_simple (4 groups, equal n) ===\n";
//
//     AnomOptions opt;
//     opt.alpha = 0.05;
//     opt.assume_equal_n = true;  // equal-n ANOM 공식 사용
//     opt.bonferroni = true;
//
//     Anom anom(opt);
//
//     // 그룹 4개, 각 8개 샘플 (equal-n)
//     anom.add_group("G1", { 10.0, 10.2,  9.9, 10.1, 10.0, 10.1,  9.8, 10.0 });
//     anom.add_group("G2", { 10.5, 10.6, 10.4, 10.7, 10.5, 10.4, 10.6, 10.5 }); // HIGH
//     anom.add_group("G3", {  9.7,  9.8,  9.6,  9.7,  9.7,  9.8,  9.6,  9.7 }); // LOW
//     anom.add_group("G4", { 10.1, 10.2, 10.0, 10.1, 10.2, 10.0, 10.1, 10.1 });
//
//     anom.fit();
//
//     std::cout << "Grand mean = " << anom.grand_mean() << "\n";
//     std::cout << "s_within   = " << anom.s_within() << "\n";
//
//     for (const auto& r : anom.results()) {
//         std::cout << r.name
//                   << " n=" << r.n
//                   << " mean=" << r.mean
//                   << " margin=" << r.margin
//                   << " UDL=" << r.UDL
//                   << " LDL=" << r.LDL
//                   << " high=" << r.significant_high
//                   << " low="  << r.significant_low
//                   << "\n";
//     }
//
//     anom.save_csv("anom_equal_n.csv");
//     std::string svg = anom.render_svg();
//     std::ofstream("anom_equal_n.svg") << svg;
// }
//
// void test_unequal_n() {
//     std::cout << "\n=== test_unequal_n (3 groups, different n) ===\n";
//
//     AnomOptions opt;
//     opt.alpha = 0.05;
//     opt.assume_equal_n = true;  // true여도 n이 다르면 내부에서 unequal-n 처리
//     opt.bonferroni = true;
//
//     Anom anom(opt);
//
//     // 그룹별 sample size 다르게
//     anom.add_group("A", { 9.9, 10.1, 10.0,  9.8 });                  // n=4
//     anom.add_group("B", {10.5, 10.6, 10.4, 10.7, 10.3, 10.6 });      // n=6
//     anom.add_group("C", { 9.7,  9.6,  9.9 });                        // n=3
//
//     anom.fit();
//
//     std::cout << "Grand mean = " << anom.grand_mean() << "\n";
//     std::cout << "s_within   = " << anom.s_within() << "\n";
//
//     for (const auto& r : anom.results()) {
//         std::cout << r.name
//                   << " n=" << r.n
//                   << " mean=" << r.mean
//                   << " margin=" << r.margin
//                   << " UDL=" << r.UDL
//                   << " LDL=" << r.LDL
//                   << " high=" << r.significant_high
//                   << " low="  << r.significant_low
//                   << "\n";
//     }
//
//     anom.save_csv("anom_unequal_n.csv");
//     std::string svg = anom.render_svg();
//     std::ofstream("anom_unequal_n.svg") << svg;
// }
//
// void test_no_bonferroni() {
//     std::cout << "\n=== test_no_bonferroni (equal n, no Bonferroni) ===\n";
//
//     AnomOptions opt;
//     opt.alpha = 0.05;
//     opt.assume_equal_n = true;
//     opt.bonferroni = false;   // Bonferroni 보정 없이 tcrit 한 번만 사용
//
//     Anom anom(opt);
//
//     anom.add_group("G1", {10.0, 10.1,  9.9, 10.0});
//     anom.add_group("G2", {10.4, 10.5, 10.6, 10.5});
//     anom.add_group("G3", { 9.7,  9.8,  9.6,  9.7});
//     anom.add_group("G4", {10.2, 10.3, 10.1, 10.2});
//
//     anom.fit();
//
//     std::cout << "Grand mean = " << anom.grand_mean() << "\n";
//     std::cout << "s_within   = " << anom.s_within() << "\n";
//
//     for (const auto& r : anom.results()) {
//         std::cout << r.name
//                   << " n=" << r.n
//                   << " mean=" << r.mean
//                   << " margin=" << r.margin
//                   << " UDL=" << r.UDL
//                   << " LDL=" << r.LDL
//                   << " high=" << r.significant_high
//                   << " low="  << r.significant_low
//                   << "\n";
//     }
//
//     anom.save_csv("anom_no_bonferroni.csv");
//     std::string svg = anom.render_svg();
//     std::ofstream("anom_no_bonferroni.svg") << svg;
// }
//
// int main() {
//     try {
//         test_equal_n_simple();
//         test_unequal_n();
//         test_no_bonferroni();
//     }
//     catch (const std::exception& ex) {
//         std::cerr << "Exception: " << ex.what() << "\n";
//         return 1;
//     }
//     return 0;
// }
//
// //
// //
// // int main() {
// //     AnomOptions opt;
// //     opt.alpha = 0.05;
// //     opt.assume_equal_n = false; // allow unequal-n margins
// //     opt.bonferroni = true;
// //
// //     Anom anom(opt);
// //     anom.add_group("A", {9.9, 10.1, 10.0, 9.8});
// //     anom.add_group("B", {10.5, 10.6, 10.4, 10.7, 10.3});
// //     anom.add_group("C", {9.7, 9.6, 9.9});
// //     anom.fit();
// //
// //     std::cout << "Grand mean: " << anom.grand_mean() << "\n";
// //     std::cout << "s_within:   " << anom.s_within() << "\n";
// //
// //     for (const auto& r : anom.results()) {
// //         std::cout << r.name << " n=" << r.n
// //                   << " mean=" << r.mean
// //                   << " margin=" << r.margin
// //                   << " UDL=" << r.UDL
// //                   << " LDL=" << r.LDL
// //                   << " high=" << r.significant_high
// //                   << " low="  << r.significant_low << "\n";
// //     }
// //
// //     // Save CSV
// //     anom.save_csv("anom_results.csv");
// //
// //     // Save SVG chart
// //     std::string svg = anom.render_svg();
// //     std::ofstream("anom_chart.svg") << svg;
// //
// //     return 0;
// // }

#include "ResponseSurface.hpp"
#include <iostream>

// int main() {
//
//     std::locale::global(std::locale("en_US.UTF-8"));
//     std::wcout.imbue(std::locale());
//
//
//     ResponseSurface rs;
//
//     // Input data (factor1, factor2 → response)
//     rs.add_point({50, 100}, 12.3);
//     rs.add_point({60, 100}, 13.1);
//     rs.add_point({50, 120}, 11.9);
//     rs.add_point({60, 120}, 13.5);
//     rs.add_point({55, 110}, 12.8);
//     rs.add_point({65, 115}, 13.7);
//
//     // Fit model
//     rs.fit();
//
//     // Show coefficients
//     rs.summary();
//
//     // Predict at new condition
//     double yhat = rs.predict({55, 110});
//     std::cout << "Predicted response at (55,110): " << yhat << "\n";
//
//     return 0;
// }

//
//
// #include <iostream>
// #include <fstream>
// #include <random>
//
// #include "orthogonal_array.hpp"
// #include "Anom_Utils.h"
// #include "doe_anom_response.hpp"
// #include "response_surface_quadratic.hpp"
// #include "doe_full_analysis.hpp"
//
// int main() {
//     try {
//         // Use L8(2^7)
//         const OrthogonalArray& oa = OA_L8_2_7();
//
//         // 2-level factors mapped to {-1, +1}
//         std::vector<FactorLevels> all_levels(oa.factors);
//         for (int j = 0; j < oa.factors; ++j) {
//             all_levels[j].levels = { -1.0, +1.0 };
//         }
//
//         // True model using only factors A and B (index 0,1)
//         auto true_model = [](double x1, double x2) {
//             double y = 10.0
//                      + 2.0 * x1
//                      + 3.0 * x2
//                      + 1.0 * x1 * x1
//                      - 0.5 * x2 * x2;
//             return y;
//         };
//
//         // Generate responses with small noise
//         std::mt19937_64 rng(12345);
//         std::normal_distribution<double> noise(0.0, 0.2);
//
//         std::vector<double> y(oa.runs);
//         for (int r = 0; r < oa.runs; ++r) {
//             int levA = oa.at(r, 0); // factor A
//             int levB = oa.at(r, 1); // factor B
//             double x1 = all_levels[0].levels[levA];
//             double x2 = all_levels[1].levels[levB];
//             y[r] = true_model(x1, x2) + noise(rng);
//         }
//
//         // Factors to use in response surface: A and B
//         std::vector<int> rs_factors = {0, 1};
//         std::vector<std::string> factor_names = {"A","B","C","D","E","F","G"};
//
//         AnomOptions anom_opt;
//         anom_opt.alpha = 0.05;
//         anom_opt.assume_equal_n = true;
//         anom_opt.bonferroni     = true;
//
//         DoeFullAnalysis analysis = run_doe_full_analysis(
//             oa,
//             all_levels,
//             rs_factors,
//             y,
//             factor_names,
//             anom_opt
//         );
//
//         // Print response surface coefficients
//         std::cout << "=== Quadratic Response Surface ===\n";
//         std::cout << "Number of factors (RS) = " << analysis.rs_model.num_factors() << "\n";
//         std::cout << "Coefficients (beta):\n"
//                   << analysis.rs_model.coefficients() << "\n\n";
//
//         // Factor-wise ANOM summary
//         for (const auto& fa : analysis.factor_anoms) {
//             const auto& factor_name = fa.factor_name;
//             const auto& anom = fa.anom;
//             std::cout << "=== ANOM for factor " << factor_name << " ===\n";
//             std::cout << "Grand mean = " << anom.grand_mean()
//                       << ", s_within = " << anom.s_within() << "\n";
//             for (const auto& r : anom.results()) {
//                 std::cout << "  " << r.name
//                           << " n=" << r.n
//                           << " mean=" << r.mean
//                           << " UDL=" << r.UDL
//                           << " LDL=" << r.LDL
//                           << " high=" << r.significant_high
//                           << " low="  << r.significant_low
//                           << "\n";
//             }
//             std::string svg_name = "anom_factor_" + factor_name + ".svg";
//             std::ofstream(svg_name) << anom.render_svg();
//             std::cout << "  -> SVG saved to " << svg_name << "\n\n";
//         }
//     }
//     catch (const std::exception& ex) {
//         std::cerr << "Exception: " << ex.what() << "\n";
//         return 1;
//     }
//     return 0;
// }





#include <iostream>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <random>
#include <string>
#include <cassert>

#include "orthogonal_array.hpp"
#include "Anom_Utils.h"
#include "doe_anom_response.hpp"
#include "response_surface_quadratic.hpp"
#include "doe_full_analysis.hpp"

// Simple helper for approximate comparison
static bool approx_equal(double a, double b, double tol = 1e-6) {
    return std::fabs(a - b) <= tol;
}

// -----------------------------------------------------------------------------
// Test 1: Basic ANOM with equal group sizes
// -----------------------------------------------------------------------------
void test_anom_equal_n_basic() {
    std::cout << "[TEST] test_anom_equal_n_basic\n";

    AnomOptions opt;
    opt.alpha = 0.05;
    opt.assume_equal_n = true;
    opt.bonferroni = true;

    Anom anom(opt);

    // 3 groups, equal n, small within variance
    anom.add_group("G1", {10.0, 10.1,  9.9, 10.0});
    anom.add_group("G2", {10.5, 10.6, 10.4, 10.5});
    anom.add_group("G3", { 9.7,  9.8,  9.6,  9.7});

    anom.fit();

    double gm = anom.grand_mean();
    double s  = anom.s_within();

    std::cout << "  grand_mean = " << gm << "\n";
    std::cout << "  s_within   = " << s  << "\n";

    const auto& results = anom.results();
    assert(results.size() == 3);

    // Check group names and n
    assert(results[0].name == "G1");
    assert(results[1].name == "G2");
    assert(results[2].name == "G3");
    assert(results[0].n == 4);
    assert(results[1].n == 4);
    assert(results[2].n == 4);

    // Basic sanity checks on means
    std::cout << "  G1 mean = " << results[0].mean << "\n";
    std::cout << "  G2 mean = " << results[1].mean << "\n";
    std::cout << "  G3 mean = " << results[2].mean << "\n";

    // Mean order should roughly be G3 < G1 < G2
    assert(results[2].mean < results[0].mean);
    assert(results[0].mean < results[1].mean);

    // Save sample CSV and SVG (optional)
    anom.save_csv("test1_anom_equal_n.csv");
    std::ofstream("test1_anom_equal_n.svg") << anom.render_svg();

    std::cout << "  -> CSV: test1_anom_equal_n.csv\n";
    std::cout << "  -> SVG: test1_anom_equal_n.svg\n";
}

// -----------------------------------------------------------------------------
// Test 2: ANOM with unequal group sizes
// -----------------------------------------------------------------------------
void test_anom_unequal_n() {
    std::cout << "[TEST] test_anom_unequal_n\n";

    AnomOptions opt;
    opt.alpha = 0.05;
    opt.assume_equal_n = true; // will detect unequal n internally and fall back
    opt.bonferroni = true;

    Anom anom(opt);

    anom.add_group("A", {9.9, 10.1, 10.0, 9.8});              // n=4
    anom.add_group("B", {10.5, 10.6, 10.4, 10.7, 10.3, 10.6}); // n=6
    anom.add_group("C", {9.7, 9.6, 9.9});                      // n=3

    anom.fit();

    const auto& results = anom.results();
    assert(results.size() == 3);
    assert(results[0].n == 4);
    assert(results[1].n == 6);
    assert(results[2].n == 3);

    std::cout << "  grand_mean = " << anom.grand_mean() << "\n";
    std::cout << "  s_within   = " << anom.s_within() << "\n";
    for (const auto& r : results) {
        std::cout << "  " << r.name
                  << " n=" << r.n
                  << " mean=" << r.mean
                  << " margin=" << r.margin
                  << " UDL=" << r.UDL
                  << " LDL=" << r.LDL
                  << " high=" << r.significant_high
                  << " low="  << r.significant_low
                  << "\n";
    }

    anom.save_csv("test2_anom_unequal_n.csv");
    std::ofstream("test2_anom_unequal_n.svg") << anom.render_svg();

    std::cout << "  -> CSV: test2_anom_unequal_n.csv\n";
    std::cout << "  -> SVG: test2_anom_unequal_n.svg\n";
}

// -----------------------------------------------------------------------------
// Test 3: Orthogonal array + FactorLevels -> design matrix
// Using OA_L4_2_3 with simple 2-level factors {-1, +1}
// -----------------------------------------------------------------------------
void test_orthogonal_array_design() {
    std::cout << "[TEST] test_orthogonal_array_design\n";

    const OrthogonalArray& oa = OA_L4_2_3();
    assert(oa.runs == 4);
    assert(oa.factors == 3);
    assert(oa.levels == 2);

    // All factors: 2-level coded as {-1, +1}
    std::vector<FactorLevels> fl(oa.factors);
    for (int j = 0; j < oa.factors; ++j) {
        fl[j].levels = {-1.0, +1.0};
    }

    auto design = build_design_from_orthogonal_array(oa, fl);
    assert((int)design.size() == oa.runs);
    assert((int)design[0].size() == oa.factors);

    // Print design matrix
    for (int r = 0; r < oa.runs; ++r) {
        std::cout << "  Run " << r << ": ";
        for (int f = 0; f < oa.factors; ++f) {
            std::cout << design[r][f] << " ";
        }
        std::cout << "\n";
    }

    // Check a few known positions
    // OA_L4_2_3 first row is (0,0,0) -> (-1,-1,-1)
    assert(approx_equal(design[0][0], -1.0));
    assert(approx_equal(design[0][1], -1.0));
    assert(approx_equal(design[0][2], -1.0));

    // Last row (1,1,0) -> (+1,+1,-1)
    assert(approx_equal(design[3][0], +1.0));
    assert(approx_equal(design[3][1], +1.0));
    assert(approx_equal(design[3][2], -1.0));
}

// -----------------------------------------------------------------------------
// Test 4: OA + response -> factor-wise ANOM
// Using OA_L8_2_7, with a simple model on factor A only.
// -----------------------------------------------------------------------------
void test_build_anom_for_factor() {
    std::cout << "[TEST] test_build_anom_for_factor\n";

    const OrthogonalArray& oa = OA_L8_2_7();
    assert(oa.runs == 8);
    assert(oa.factors == 7);
    assert(oa.levels == 2);

    // Factor A (index 0) will have a main effect:
    // Level 0 -> mean = 10
    // Level 1 -> mean = 12
    // Add small noise for within variance.
    std::mt19937_64 rng(42);
    std::normal_distribution<double> noise(0.0, 0.2);

    std::vector<double> y(oa.runs);
    for (int r = 0; r < oa.runs; ++r) {
        int levelA = oa.at(r, 0);
        double meanA = (levelA == 0 ? 10.0 : 12.0);
        y[r] = meanA + noise(rng);
    }

    AnomOptions opt;
    opt.alpha = 0.05;
    opt.assume_equal_n = true;
    opt.bonferroni = true;

    // Build ANOM for factor A only
    Anom anomA = build_anom_for_factor(oa, y, 0, "A", opt);
    const auto& resA = anomA.results();
    assert(resA.size() == 2); // 2 levels

    std::cout << "  Factor A ANOM:\n";
    for (const auto& r : resA) {
        std::cout << "    " << r.name
                  << " n=" << r.n
                  << " mean=" << r.mean
                  << " UDL=" << r.UDL
                  << " LDL=" << r.LDL
                  << " high=" << r.significant_high
                  << " low="  << r.significant_low
                  << "\n";
    }

    // Level groups are A_L1 (level 0), A_L2 (level 1)
    // We expect A_L2 mean > A_L1 mean
    assert(resA[0].name == "A_L1");
    assert(resA[1].name == "A_L2");
    assert(resA[1].mean > resA[0].mean);

    // Build ANOM for all factors
    auto all_anoms = build_anom_for_all_factors(oa, y, {"A","B","C","D","E","F","G"}, opt);
    assert(all_anoms.size() == 7);
    // Check that first result corresponds to factor A
    assert(all_anoms[0].factor_name == "A");

    // Save SVG for factor A
    std::ofstream("test4_anom_factor_A.svg") << anomA.render_svg();
    std::cout << "  -> SVG: test4_anom_factor_A.svg\n";
}

// -----------------------------------------------------------------------------
// Test 5: ResponseSurfaceQuadratic fitting accuracy
// Fit a known quadratic in 2 variables and check coefficients.
// -----------------------------------------------------------------------------
void test_response_surface_quadratic_fit() {
    std::cout << "[TEST] test_response_surface_quadratic_fit\n";

    // True model:
    // y = b0 + b1*x1 + b2*x2 + b11*x1^2 + b22*x2^2 + b12*x1*x2
    const double b0  = 10.0;
    const double b1  =  2.0;
    const double b2  = -1.0;
    const double b11 =  0.5;
    const double b22 = -0.3;
    const double b12 =  1.2;

    auto true_model = [&](double x1, double x2) {
        return b0
             + b1  * x1
             + b2  * x2
             + b11 * x1 * x1
             + b22 * x2 * x2
             + b12 * x1 * x2;
    };

    // Build design points on a small grid
    std::vector<std::vector<double>> design;
    std::vector<double> y;
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 2; ++j) {
            double x1 = i * 0.5;
            double x2 = j * 0.5;
            design.push_back({x1, x2});
            y.push_back(true_model(x1, x2));
        }
    }

    ResponseSurfaceQuadratic rs;
    bool ok = rs.fit(design, y);
    assert(ok);

    const auto& beta = rs.coefficients();
    // For k=2, the order is:
    // beta[0] = b0
    // beta[1] = b1, beta[2] = b2
    // beta[3] = b11, beta[4] = b22
    // beta[5] = b12

    std::cout << "  Estimated coefficients:\n" << beta << "\n";

    assert(approx_equal(beta[0], b0, 1e-6));
    assert(approx_equal(beta[1], b1, 1e-6));
    assert(approx_equal(beta[2], b2, 1e-6));
    assert(approx_equal(beta[3], b11, 1e-6));
    assert(approx_equal(beta[4], b22, 1e-6));
    assert(approx_equal(beta[5], b12, 1e-6));

    // Test prediction at a random point
    std::vector<double> x_test = {0.7, -0.4};
    double y_true = true_model(x_test[0], x_test[1]);
    double y_pred = rs.predict(x_test);
    std::cout << "  y_true = " << y_true << ", y_pred = " << y_pred << "\n";
    assert(approx_equal(y_true, y_pred, 1e-6));
}

// -----------------------------------------------------------------------------
// Test 6: Full DOE analysis using L8(2^7)
// - Builds responses from a known quadratic model in A,B
// - Runs run_doe_full_analysis (RS + ANOM)
// -----------------------------------------------------------------------------
void test_doe_full_analysis() {
    std::cout << "[TEST] test_doe_full_analysis\n";

    const OrthogonalArray& oa = OA_L8_2_7();
    assert(oa.runs == 8);
    assert(oa.factors == 7);

    // Map 2-level factors to {-1, +1}
    std::vector<FactorLevels> all_levels(oa.factors);
    for (int j = 0; j < oa.factors; ++j) {
        all_levels[j].levels = {-1.0, +1.0};
    }

    // True model using only factor A (x1) and B (x2)
    auto true_model = [](double x1, double x2) {
        double y = 10.0
                 + 2.0 * x1
                 + 3.0 * x2
                 + 1.0 * x1 * x1
                 - 0.5 * x2 * x2;
        return y;
    };

    std::mt19937_64 rng(12345);
    std::normal_distribution<double> noise(0.0, 0.2);

    std::vector<double> y(oa.runs);
    for (int r = 0; r < oa.runs; ++r) {
        int levA = oa.at(r, 0);
        int levB = oa.at(r, 1);
        double x1 = all_levels[0].levels[levA];
        double x2 = all_levels[1].levels[levB];
        y[r] = true_model(x1, x2) + noise(rng);
    }

    // Factors used in response surface: A,B
    std::vector<int> rs_factors = {0, 1};
    std::vector<std::string> factor_names = {"A","B","C","D","E","F","G"};

    AnomOptions anom_opt;
    anom_opt.alpha = 0.05;
    anom_opt.assume_equal_n = true;
    anom_opt.bonferroni = true;

    DoeFullAnalysis analysis = run_doe_full_analysis(
        oa,
        all_levels,
        rs_factors,
        y,
        factor_names,
        anom_opt
    );

    // Check response surface dimension
    assert(analysis.rs_model.num_factors() == 2);
    const auto& beta = analysis.rs_model.coefficients();
    std::cout << "  RS coefficients:\n" << beta << "\n";

    // Quick sanity check: shape of beta for k=2 is 6
    assert(beta.size() == 6);

    // Factor-wise ANOM
    const auto& factor_anoms = analysis.factor_anoms;
    assert(factor_anoms.size() == 7); // A..G

    for (const auto& fa : factor_anoms) {
        const auto& anom = fa.anom;
        std::cout << "  ANOM factor " << fa.factor_name << ": grand_mean = "
                  << anom.grand_mean()
                  << ", s_within = " << anom.s_within() << "\n";
        for (const auto& r : anom.results()) {
            std::cout << "    " << r.name
                      << " n=" << r.n
                      << " mean=" << r.mean
                      << " UDL=" << r.UDL
                      << " LDL=" << r.LDL
                      << " high=" << r.significant_high
                      << " low="  << r.significant_low
                      << "\n";
        }
        std::string svg_name = "test6_anom_factor_" + fa.factor_name + ".svg";
        std::ofstream(svg_name) << anom.render_svg();
        std::cout << "    -> SVG: " << svg_name << "\n";
    }
}

// -----------------------------------------------------------------------------
// Main: run all tests
// -----------------------------------------------------------------------------
int main() {
    try {
        test_anom_equal_n_basic();
        test_anom_unequal_n();
        test_orthogonal_array_design();
        test_build_anom_for_factor();
        test_response_surface_quadratic_fit();
        test_doe_full_analysis();

        std::cout << "\nAll tests finished without assertion failures.\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in tests: " << ex.what() << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception in tests.\n";
        return 1;
    }
    return 0;
}