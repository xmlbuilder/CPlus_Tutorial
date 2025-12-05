#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <algorithm>
#include <sstream>
#include <fstream>

namespace stat_util {

// -----------------------------------------------------------------------------
// Standard normal quantile Phi^{-1}(p) approximation
// Based on a Moro/Wichura-style rational approximation.
// -----------------------------------------------------------------------------
inline double normal_quantile_approx(double p) {
    if (p <= 0.0 || p >= 1.0)
        throw std::runtime_error("normal_quantile_approx: p must be in (0,1)");

    static const double a1 = -3.969683028665376e+01;
    static const double a2 =  2.209460984245205e+02;
    static const double a3 = -2.759285104469687e+02;
    static const double a4 =  1.383577518672690e+02;
    static const double a5 = -3.066479806614716e+01;
    static const double a6 =  2.506628277459239e+00;

    static const double b1 = -5.447609879822406e+01;
    static const double b2 =  1.615858368580409e+02;
    static const double b3 = -1.556989798598866e+02;
    static const double b4 =  6.680131188771972e+01;
    static const double b5 = -1.328068155288572e+01;

    static const double c1 = -7.784894002430293e-03;
    static const double c2 = -3.223964580411365e-01;
    static const double c3 = -2.400758277161838e+00;
    static const double c4 = -2.549732539343734e+00;
    static const double c5 =  4.374664141464968e+00;
    static const double c6 =  2.938163982698783e+00;

    static const double d1 =  7.784695709041462e-03;
    static const double d2 =  3.224671290700398e-01;
    static const double d3 =  2.445134137142996e+00;
    static const double d4 =  3.754408661907416e+00;

    double q, r;
    if (p < 0.02425) {
        // lower tail
        q = std::sqrt(-2.0 * std::log(p));
        return (((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
               ((((d1*q + d2)*q + d3)*q + d4)*q + 1.0);
    } else if (p > 1.0 - 0.02425) {
        // upper tail
        q = std::sqrt(-2.0 * std::log(1.0 - p));
        return -(((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
                 ((((d1*q + d2)*q + d3)*q + d4)*q + 1.0);
    } else {
        // central region
        q = p - 0.5;
        r = q * q;
        return (((((a1*r + a2)*r + a3)*r + a4)*r + a5)*r + a6) * q /
               (((((b1*r + b2)*r + b3)*r + b4)*r + b5)*r + 1.0);
    }
}

// -----------------------------------------------------------------------------
// Student t quantile t_p(df) approximation
// p in (0,1), df > 0.
// -----------------------------------------------------------------------------
inline double student_t_quantile_approx(double p, double df) {
    if (df <= 0.0)
        throw std::runtime_error("student_t_quantile_approx: df must be > 0");
    double z = normal_quantile_approx(p);
    if (df > 30.0)
        return z;
    double z3 = z * z * z;
    return z + (z3 + z) / (4.0 * df); // simple small-df correction
}

// -----------------------------------------------------------------------------
// Bonferroni-based ANOM h for equal-n case
// a  : number of groups
// n  : observations per group
// df : within-group degrees of freedom
// -----------------------------------------------------------------------------
inline double anom_h_bonferroni_equal_n(double alpha, int a, int n, int df) {
    if (alpha <= 0.0 || alpha >= 1.0)
        throw std::runtime_error("anom_h_bonferroni_equal_n: alpha in (0,1)");
    if (a <= 1 || n <= 0 || df <= 0)
        throw std::runtime_error("anom_h_bonferroni_equal_n: invalid a/n/df");

    double alpha_per_group = alpha / static_cast<double>(a);
    double p = 1.0 - alpha_per_group / 2.0; // two-sided
    double tcrit = student_t_quantile_approx(p, static_cast<double>(df));

    // Classic ANOM scaling: sqrt((a-1)/a)
    return tcrit * std::sqrt(static_cast<double>(a - 1) / a);
}

// -----------------------------------------------------------------------------
// Bonferroni-based t critical value (unequal-n case)
// -----------------------------------------------------------------------------
inline double anom_tcrit_bonferroni(double alpha, int a, int df) {
    if (alpha <= 0.0 || alpha >= 1.0)
        throw std::runtime_error("anom_tcrit_bonferroni: alpha in (0,1)");
    if (a <= 1 || df <= 0)
        throw std::runtime_error("anom_tcrit_bonferroni: invalid a/df");
    double alpha_per_group = alpha / static_cast<double>(a);
    double p = 1.0 - alpha_per_group / 2.0;
    return student_t_quantile_approx(p, static_cast<double>(df));
}

} // namespace stat_util

// ============================================================================
// ANOM main structures
// ============================================================================

struct AnomOptions {
    double alpha = 0.05;        // global significance level
    bool assume_equal_n = true; // if true and all groups have same n, use equal-n ANOM h
    bool bonferroni = true;     // if true, apply Bonferroni correction across groups

    // SVG drawing options
    double svg_width  = 900.0;
    double svg_height = 500.0;
    double svg_margin = 60.0;
};

struct AnomGroupResult {
    std::string name;
    int n = 0;
    double mean   = std::numeric_limits<double>::quiet_NaN();
    double margin = std::numeric_limits<double>::quiet_NaN();
    double UDL    = std::numeric_limits<double>::quiet_NaN();
    double LDL    = std::numeric_limits<double>::quiet_NaN();
    bool significant_high = false;
    bool significant_low  = false;
};

class Anom {
public:
    explicit Anom(AnomOptions opt = {}) : opt_(opt) {}

    // Add group from std::vector<double>
    void add_group(const std::string& name, const std::vector<double>& values) {
        if (values.empty())
            throw std::runtime_error("Anom::add_group: group has no values: " + name);
        groups_.push_back({name, values});
        computed_ = false;
    }

    // Add group from initializer_list<double>
    void add_group(const std::string& name, std::initializer_list<double> values) {
        if (values.size() == 0)
            throw std::runtime_error("Anom::add_group: group has no values: " + name);
        groups_.push_back({name, std::vector<double>(values)});
        computed_ = false;
    }

    // Clear all groups and results
    void clear() {
        groups_.clear();
        results_.clear();
        computed_   = false;
        grand_mean_ = mse_ = s_within_ = std::numeric_limits<double>::quiet_NaN();
    }

    // -------------------------------------------------------------------------
    // Fit ANOM: compute group means, pooled variance, grand mean, decision limits
    // -------------------------------------------------------------------------
    void fit() {
        if (groups_.empty())
            throw std::runtime_error("Anom::fit: no groups to fit");

        int a = static_cast<int>(groups_.size());
        std::vector<double> means(a);
        std::vector<int>    ns(a);

        int N = 0;
        for (int i = 0; i < a; ++i) {
            const auto& v = groups_[i].values;
            ns[i] = static_cast<int>(v.size());
            N += ns[i];
            double s = 0.0;
            for (double x : v) s += x;
            means[i] = s / ns[i];
        }

        // Grand mean (weighted by group sizes)
        double grand_sum = 0.0;
        for (int i = 0; i < a; ++i) grand_sum += means[i] * ns[i];
        grand_mean_ = grand_sum / static_cast<double>(N);

        // Pooled within-group variance (MSE)
        int df_within = 0;
        double ss_within = 0.0;
        for (int i = 0; i < a; ++i) {
            const auto& v = groups_[i].values;
            double mi = means[i];
            for (double x : v) {
                double d = x - mi;
                ss_within += d * d;
            }
            df_within += (ns[i] - 1);
        }
        if (df_within <= 0)
            throw std::runtime_error("Anom::fit: insufficient degrees of freedom");

        mse_      = ss_within / static_cast<double>(df_within);
        s_within_ = std::sqrt(mse_);

        // Decide margins per group
        results_.clear();
        results_.reserve(a);

        bool equal_n = opt_.assume_equal_n && all_equal(ns);
        double h     = std::numeric_limits<double>::quiet_NaN();
        double tcrit = std::numeric_limits<double>::quiet_NaN();

        if (opt_.bonferroni) {
            if (equal_n) {
                // Equal-n ANOM: use h factor
                h = stat_util::anom_h_bonferroni_equal_n(opt_.alpha, a, ns[0], df_within);
            } else {
                // Unequal-n: use t critical
                tcrit = stat_util::anom_tcrit_bonferroni(opt_.alpha, a, df_within);
            }
        } else {
            // No Bonferroni: standard two-sided t critical
            double p = 1.0 - opt_.alpha / 2.0;
            tcrit = stat_util::student_t_quantile_approx(p, static_cast<double>(df_within));
        }

        for (int i = 0; i < a; ++i) {
            AnomGroupResult r;
            r.name = groups_[i].name;
            r.n    = ns[i];
            r.mean = means[i];

            double margin_i = 0.0;
            if (equal_n && !std::isnan(h)) {
                // equal-n ANOM:
                // margin_i = h * s * sqrt(1 / n_i)
                margin_i = h * s_within_ * std::sqrt(1.0 / ns[i]);
            } else {
                // general t-based margin:
                // margin_i = tcrit * s * sqrt(1 / n_i)
                margin_i = tcrit * s_within_ * std::sqrt(1.0 / ns[i]);
            }

            r.margin = margin_i;
            r.UDL    = grand_mean_ + margin_i;
            r.LDL    = grand_mean_ - margin_i;
            r.significant_high = (r.mean > r.UDL);
            r.significant_low  = (r.mean < r.LDL);

            results_.push_back(r);
        }

        computed_ = true;
    }

    // Grand mean
    double grand_mean() const { ensure_computed(); return grand_mean_; }

    // Pooled within-group standard deviation
    double s_within() const { ensure_computed(); return s_within_; }

    // All group results
    const std::vector<AnomGroupResult>& results() const { ensure_computed(); return results_; }

    // Save ANOM results to CSV
    void save_csv(const std::string& path) const {
        ensure_computed();
        std::ofstream ofs(path);
        if (!ofs)
            throw std::runtime_error("Anom::save_csv: cannot open file: " + path);
        ofs << "group,n,mean,margin,UDL,LDL,significant_high,significant_low\n";
        for (const auto& r : results_) {
            ofs << r.name << ","
                << r.n << ","
                << r.mean << ","
                << r.margin << ","
                << r.UDL << ","
                << r.LDL << ","
                << (r.significant_high ? 1 : 0) << ","
                << (r.significant_low  ? 1 : 0) << "\n";
        }
    }

    // Render ANOM chart as simple SVG
    std::string render_svg() const {
        ensure_computed();
        const double W = opt_.svg_width;
        const double H = opt_.svg_height;
        const double M = opt_.svg_margin;
        const double plotW = W - 2 * M;
        const double plotH = H - 2 * M;

        // Determine y-range from LDL/UDL and means
        double ymin = grand_mean_, ymax = grand_mean_;
        for (const auto& r : results_) {
            ymin = std::min({ymin, r.mean, r.LDL});
            ymax = std::max({ymax, r.mean, r.UDL});
        }
        double span = (ymax - ymin);
        if (span <= 0.0) span = 1.0;
        double pad = 0.05 * span;
        ymin -= pad; ymax += pad;

        auto y_to_px = [&](double y) {
            double t = (y - ymin) / (ymax - ymin);
            return H - M - t * plotH;
        };

        int a = static_cast<int>(results_.size());
        auto x_for_i = [&](int i) {
            double t = (a == 1 ? 0.5 : static_cast<double>(i) / (a - 1));
            return M + t * plotW;
        };

        std::ostringstream ss;
        ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << W
           << "\" height=\"" << H << "\">\n";

        // Background and axes
        ss << "<rect x=\"0\" y=\"0\" width=\"" << W << "\" height=\"" << H
           << "\" fill=\"#ffffff\"/>\n";
        ss << "<line x1=\"" << M << "\" y1=\"" << H - M << "\" x2=\"" << W - M
           << "\" y2=\"" << H - M << "\" stroke=\"#000\"/>\n"; // X axis
        ss << "<line x1=\"" << M << "\" y1=\"" << M << "\" x2=\"" << M
           << "\" y2=\"" << H - M << "\" stroke=\"#000\"/>\n"; // Y axis

        // Grand mean line
        ss << "<line x1=\"" << M << "\" y1=\"" << y_to_px(grand_mean_) << "\" x2=\"" << W - M
           << "\" y2=\"" << y_to_px(grand_mean_) << "\" stroke=\"#1f77b4\" stroke-dasharray=\"6,4\"/>\n";

        // Global min LDL and max UDL
        double minLDL = std::numeric_limits<double>::infinity();
        double maxUDL = -std::numeric_limits<double>::infinity();
        for (const auto& r : results_) {
            minLDL = std::min(minLDL, r.LDL);
            maxUDL = std::max(maxUDL, r.UDL);
        }
        ss << "<line x1=\"" << M << "\" y1=\"" << y_to_px(maxUDL) << "\" x2=\"" << W - M
           << "\" y2=\"" << y_to_px(maxUDL) << "\" stroke=\"#d62728\" stroke-width=\"1.5\"/>\n";
        ss << "<line x1=\"" << M << "\" y1=\"" << y_to_px(minLDL) << "\" x2=\"" << W - M
           << "\" y2=\"" << y_to_px(minLDL) << "\" stroke=\"#2ca02c\" stroke-width=\"1.5\"/>\n";

        // Group points and per-group UDL/LDL ticks
        for (int i = 0; i < a; ++i) {
            const auto& r = results_[i];
            double x = x_for_i(i);
            double y = y_to_px(r.mean);
            std::string color = (r.significant_high ? "#d62728"
                                : (r.significant_low ? "#2ca02c" : "#555555"));
            double radius = 5.0;

            // Mean point
            ss << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << radius
               << "\" fill=\"" << color << "\"/>\n";

            // UDL/LDL ticks for this group
            double yUDL = y_to_px(r.UDL);
            double yLDL = y_to_px(r.LDL);
            ss << "<line x1=\"" << x - 12 << "\" y1=\"" << yUDL << "\" x2=\"" << x + 12
               << "\" y2=\"" << yUDL << "\" stroke=\"#d62728\"/>\n";
            ss << "<line x1=\"" << x - 12 << "\" y1=\"" << yLDL << "\" x2=\"" << x + 12
               << "\" y2=\"" << yLDL << "\" stroke=\"#2ca02c\"/>\n";

            // Group name label
            ss << "<text x=\"" << x << "\" y=\"" << (H - M + 18)
               << "\" font-size=\"12\" text-anchor=\"middle\" fill=\"#000\">" << r.name << "</text>\n";
        }

        // Simple y-axis labels: max, grand mean, min
        ss << "<text x=\"" << (M - 8) << "\" y=\"" << y_to_px(ymax)
           << "\" font-size=\"11\" text-anchor=\"end\">" << round2(ymax) << "</text>\n";
        ss << "<text x=\"" << (M - 8) << "\" y=\"" << y_to_px(grand_mean_)
           << "\" font-size=\"11\" text-anchor=\"end\">" << round2(grand_mean_) << "</text>\n";
        ss << "<text x=\"" << (M - 8) << "\" y=\"" << y_to_px(ymin)
           << "\" font-size=\"11\" text-anchor=\"end\">" << round2(ymin) << "</text>\n";

        ss << "</svg>\n";
        return ss.str();
    }

private:
    struct Group {
        std::string name;
        std::vector<double> values;
    };

    static bool all_equal(const std::vector<int>& ns) {
        for (size_t i = 1; i < ns.size(); ++i)
            if (ns[i] != ns[0]) return false;
        return true;
    }

    static double round2(double x) {
        return std::round(x * 100.0) / 100.0;
    }

    void ensure_computed() const {
        if (!computed_)
            throw std::runtime_error("Anom: fit() has not been called");
    }

    AnomOptions opt_;
    std::vector<Group> groups_;
    bool computed_ = false;

    double grand_mean_ = std::numeric_limits<double>::quiet_NaN();
    double mse_        = std::numeric_limits<double>::quiet_NaN();
    double s_within_   = std::numeric_limits<double>::quiet_NaN();
    std::vector<AnomGroupResult> results_;
};
