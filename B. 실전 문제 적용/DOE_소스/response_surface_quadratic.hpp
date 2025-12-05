#pragma once
#include <vector>
#include <stdexcept>
#include <Eigen/Dense>

// Quadratic response surface:
// y ≈ β0 + Σ β_i x_i + Σ β_ii x_i^2 + Σ β_ij x_i x_j (i<j)
class ResponseSurfaceQuadratic {
public:
    ResponseSurfaceQuadratic() = default;

    // Fit using linear least squares with column-pivoted QR.
    // This is robust even if Phi^T Phi is singular (rank-deficient design).
    bool fit(const std::vector<std::vector<double>>& design,
             const std::vector<double>& y)
    {
        int N = static_cast<int>(design.size());
        if (N == 0) return false;
        if ((int)y.size() != N) return false;

        k_ = static_cast<int>(design[0].size());
        for (int i = 1; i < N; ++i) {
            if ((int)design[i].size() != k_)
                return false;
        }

        int num_cross = k_ * (k_ - 1) / 2;
        int m = 1 + k_ + k_ + num_cross; // 1 + linear + quadratic + interactions

        Eigen::MatrixXd Phi(N, m);
        Eigen::VectorXd Y(N);

        for (int r = 0; r < N; ++r) {
            const auto& x = design[r];
            int col = 0;

            // constant term
            Phi(r, col++) = 1.0;

            // linear terms
            for (int i = 0; i < k_; ++i)
                Phi(r, col++) = x[i];

            // squared terms
            for (int i = 0; i < k_; ++i)
                Phi(r, col++) = x[i] * x[i];

            // interaction terms (i<j)
            for (int i = 0; i < k_; ++i) {
                for (int j = i + 1; j < k_; ++j) {
                    Phi(r, col++) = x[i] * x[j];
                }
            }

            Y(r) = y[r];
        }

        // Column-pivoted QR for least squares: min ||Phi * beta - Y||
        Eigen::ColPivHouseholderQR<Eigen::MatrixXd> qr(Phi);
        int rank = qr.rank();

        if (rank < m) {
            // Rank-deficient design: not all coefficients are uniquely identifiable.
            // We still compute a least-squares solution (minimum-norm in the QR sense).
            // You can log or assert here if you want to detect aliasing.
            // std::cerr << "Warning: ResponseSurfaceQuadratic: design is rank-deficient (rank="
            //           << rank << " < " << m << ")\n";
        }

        beta_ = qr.solve(Y); // works even if rank < m
        fitted_ = true;
        return true;
    }

    // Predict at a single point x
    double predict(const std::vector<double>& x) const {
        if (!fitted_)
            throw std::runtime_error("ResponseSurfaceQuadratic::predict: model not fitted yet");
        if ((int)x.size() != k_)
            throw std::runtime_error("ResponseSurfaceQuadratic::predict: dimension mismatch");

        int num_cross = k_ * (k_ - 1) / 2;
        int m = 1 + k_ + k_ + num_cross;

        Eigen::VectorXd phi(m);
        int col = 0;

        // constant term
        phi(col++) = 1.0;

        // linear terms
        for (int i = 0; i < k_; ++i)
            phi(col++) = x[i];

        // squared terms
        for (int i = 0; i < k_; ++i)
            phi(col++) = x[i] * x[i];

        // interaction terms
        for (int i = 0; i < k_; ++i) {
            for (int j = i + 1; j < k_; ++j) {
                phi(col++) = x[i] * x[j];
            }
        }

        return beta_.dot(phi);
    }

    int num_factors() const { return k_; }
    const Eigen::VectorXd& coefficients() const { return beta_; }

private:
    int k_ = 0;
    bool fitted_ = false;
    Eigen::VectorXd beta_;
};
