#pragma once
#include <vector>
#include <string>

#include "orthogonal_array.hpp"
#include "Anom_Utils.h"
#include "doe_anom_response.hpp"
#include "response_surface_quadratic.hpp"

// Combined analysis: quadratic response surface + factor-wise ANOM
struct DoeFullAnalysis {
    ResponseSurfaceQuadratic rs_model;
    std::vector<FactorAnomResult> factor_anoms;
};

// Run full DOE analysis:
// - ResponseSurfaceQuadratic on selected factors
// - ANOM on all factors
inline DoeFullAnalysis run_doe_full_analysis(
    const OrthogonalArray& oa,
    const std::vector<FactorLevels>& all_levels,
    const std::vector<int>& factor_indices_for_rs,
    const std::vector<double>& y,
    const std::vector<std::string>& factor_names = {},
    const AnomOptions& anom_opt = AnomOptions{})
{
    if ((int)y.size() != oa.runs)
        throw std::runtime_error("run_doe_full_analysis: y size must match oa.runs");

    // Build design matrix for selected factors
    auto design = build_design_from_orthogonal_array_for_factors(
        oa, all_levels, factor_indices_for_rs);

    // Fit quadratic response surface
    ResponseSurfaceQuadratic rs;
    if (!rs.fit(design, y))
        throw std::runtime_error("run_doe_full_analysis: ResponseSurfaceQuadratic::fit failed");

    // Factor-wise ANOM (all factors)
    auto all_factor_anoms = build_anom_for_all_factors(
        oa, y, factor_names, anom_opt);

    DoeFullAnalysis out;
    out.rs_model     = rs;
    out.factor_anoms = std::move(all_factor_anoms);
    return out;
}
