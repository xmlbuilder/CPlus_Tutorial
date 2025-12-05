#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <string>

// Basic orthogonal array structure
struct OrthogonalArray
{
    int runs    = 0;              // number of experimental runs (rows)
    int factors = 0;              // number of factors (columns)
    int levels  = 0;              // maximum number of levels used (0..levels-1)
    std::vector<int> data;        // row-major: data[run * factors + factor] = level index

    int at(int run, int factor) const
    {
        return data[run * factors + factor];
    }
};

// Physical numeric levels for each factor
struct FactorLevels
{
    std::vector<double> levels;   // e.g., 2-level: {low, high}, 3-level: {low, mid, high}
};

// -----------------------------------------------------------------------------
// Predefined Taguchi orthogonal arrays (0-based levels)
// -----------------------------------------------------------------------------

// L4(2^3): 4 runs, 3 factors, 2 levels (0,1)
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

// L8(2^7): 8 runs, 7 factors, 2 levels (0,1)
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

// L9(3^4): 9 runs, 4 factors, 3 levels (0,1,2)
inline const OrthogonalArray& OA_L9_3_4()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 9;
        o.factors = 4;
        o.levels  = 3;
        o.data = {
            // F1 F2 F3 F4 (original 1..3 mapped to 0..2)
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
        return o;
    }();
    return oa;
}

// L18(2^1 × 3^7): 18 runs, 8 factors
// Factor 1: 2-level (0,1), Factors 2-8: 3-level (0,1,2)
inline const OrthogonalArray& OA_L18_2_1_3_7()
{
    static const OrthogonalArray oa = []{
        OrthogonalArray o;
        o.runs    = 18;
        o.factors = 8;
        o.levels  = 3; // maximum level count
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

// -----------------------------------------------------------------------------
// Build full design matrix using all factors
// design[run][factor] = numeric level value
// -----------------------------------------------------------------------------
inline std::vector<std::vector<double>>
build_design_from_orthogonal_array(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& factors)
{
    if ((int)factors.size() < oa.factors)
        throw std::runtime_error("build_design_from_orthogonal_array: not enough FactorLevels");

    std::vector<std::vector<double>> design(oa.runs, std::vector<double>(oa.factors));

    for (int r = 0; r < oa.runs; ++r) {
        for (int f = 0; f < oa.factors; ++f) {
            int level_index = oa.at(r, f);
            const auto& fl = factors[f];
            if (level_index < 0 || level_index >= (int)fl.levels.size())
                throw std::runtime_error("build_design_from_orthogonal_array: level index out of range");
            design[r][f] = fl.levels[level_index];
        }
    }

    return design;
}

// -----------------------------------------------------------------------------
// Build design matrix for specific factor indices only
// design[run][k] where k is the index in factor_indices
// -----------------------------------------------------------------------------
inline std::vector<std::vector<double>>
build_design_from_orthogonal_array_for_factors(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& all_levels,
    const std::vector<int>& factor_indices)
{
    if (factor_indices.empty())
        throw std::runtime_error("build_design_from_orthogonal_array_for_factors: no factor_indices");

    int max_idx = *std::max_element(factor_indices.begin(), factor_indices.end());
    if (max_idx >= oa.factors)
        throw std::runtime_error("build_design_from_orthogonal_array_for_factors: factor index out of OA range");
    if (max_idx >= (int)all_levels.size())
        throw std::runtime_error("build_design_from_orthogonal_array_for_factors: factor index out of level size");

    int runs = oa.runs;
    int k    = static_cast<int>(factor_indices.size());
    std::vector<std::vector<double>> design(runs, std::vector<double>(k));

    for (int r = 0; r < runs; ++r) {
        for (int j = 0; j < k; ++j) {
            int f_idx    = factor_indices[j];
            int level_id = oa.at(r, f_idx);
            const auto& fl = all_levels[f_idx];
            if (level_id < 0 || level_id >= (int)fl.levels.size())
                throw std::runtime_error("build_design_from_orthogonal_array_for_factors: level index out of range");
            design[r][j] = fl.levels[level_id];
        }
    }

    return design;
}
