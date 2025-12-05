#pragma once
#include <vector>
#include <string>
#include <stdexcept>

#include "orthogonal_array.hpp"
#include "Anom_Utils.h"

// Build ANOM for a single factor from OA + responses
inline Anom build_anom_for_factor(
    const OrthogonalArray& oa,
    const std::vector<double>& y,
    int factor_idx,
    const std::string& factor_name,
    const AnomOptions& opt = AnomOptions{})
{
    if (factor_idx < 0 || factor_idx >= oa.factors)
        throw std::runtime_error("build_anom_for_factor: factor_idx out of range");
    if ((int)y.size() != oa.runs)
        throw std::runtime_error("build_anom_for_factor: y size must match oa.runs");

    int L = oa.levels;

    std::vector<std::vector<double>> level_values(L);
    for (int r = 0; r < oa.runs; ++r) {
        int lev = oa.at(r, factor_idx);
        if (lev < 0 || lev >= L)
            throw std::runtime_error("build_anom_for_factor: level index out of range");
        level_values[lev].push_back(y[r]);
    }

    Anom anom(opt);
    for (int lev = 0; lev < L; ++lev) {
        if (level_values[lev].empty())
            continue;
        std::string gname = factor_name + "_L" + std::to_string(lev + 1);
        anom.add_group(gname, level_values[lev]);
    }

    anom.fit();
    return anom;
}

// Factor-wise ANOM for all factors
struct FactorAnomResult {
    std::string factor_name;
    Anom anom;
};

inline std::vector<FactorAnomResult> build_anom_for_all_factors(
    const OrthogonalArray& oa,
    const std::vector<double>& y,
    const std::vector<std::string>& factor_names = {},
    const AnomOptions& opt = AnomOptions{})
{
    if ((int)y.size() != oa.runs)
        throw std::runtime_error("build_anom_for_all_factors: y size must match oa.runs");

    std::vector<std::string> names;
    names.reserve(oa.factors);
    if (!factor_names.empty()) {
        if ((int)factor_names.size() != oa.factors)
            throw std::runtime_error("build_anom_for_all_factors: factor_names size mismatch");
        names = factor_names;
    } else {
        for (int i = 0; i < oa.factors; ++i) {
            char c = static_cast<char>('A' + i);
            names.push_back(std::string(1, c));
        }
    }

    std::vector<FactorAnomResult> out;
    out.reserve(oa.factors);
    for (int j = 0; j < oa.factors; ++j) {
        Anom anom_j = build_anom_for_factor(oa, y, j, names[j], opt);
        out.push_back(FactorAnomResult{names[j], std::move(anom_j)});
    }
    return out;
}
