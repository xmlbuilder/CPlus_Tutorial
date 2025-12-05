#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <iostream>

// Simple linear algebra utilities
namespace linalg {
    using Vec = std::vector<double>;
    using Mat = std::vector<Vec>;

    // Transpose of a matrix
    Mat transpose(const Mat& A) {
        int m = A.size(), n = A[0].size();
        Mat T(n, Vec(m));
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                T[j][i] = A[i][j];
        return T;
    }

    // Matrix multiplication
    Mat multiply(const Mat& A, const Mat& B) {
        int m = A.size(), n = A[0].size(), p = B[0].size();
        Mat C(m, Vec(p, 0.0));
        for (int i = 0; i < m; ++i)
            for (int k = 0; k < p; ++k)
                for (int j = 0; j < n; ++j)
                    C[i][k] += A[i][j] * B[j][k];
        return C;
    }

    // Matrix inverse using Gauss-Jordan (for small matrices)
    Mat inverse(Mat A) {
        int n = A.size();
        Mat I(n, Vec(n, 0.0));
        for (int i = 0; i < n; ++i) I[i][i] = 1.0;

        for (int i = 0; i < n; ++i) {
            double pivot = A[i][i];
            if (std::fabs(pivot) < 1e-12) throw std::runtime_error("Singular matrix");
            for (int j = 0; j < n; ++j) {
                A[i][j] /= pivot;
                I[i][j] /= pivot;
            }
            for (int k = 0; k < n; ++k) {
                if (k == i) continue;
                double factor = A[k][i];
                for (int j = 0; j < n; ++j) {
                    A[k][j] -= factor * A[i][j];
                    I[k][j] -= factor * I[i][j];
                }
            }
        }
        return I;
    }
}

class ResponseSurface {
public:
    // Add one data point (x vector and response y)
    void add_point(const std::vector<double>& x, double y) {
        if (x.size() != 2) throw std::runtime_error("This example supports 2 factors only");
        data_.push_back({x, y});
    }

    // Fit quadratic response surface model
    void fit() {
        int n = data_.size();
        if (n == 0) throw std::runtime_error("No data");

        // Design matrix X (n × p), response vector Y (n × 1)
        int p = 6; // β0, β1, β2, β11, β22, β12
        linalg::Mat X(n, linalg::Vec(p));
        linalg::Mat Y(n, linalg::Vec(1));

        for (int i = 0; i < n; ++i) {
            double x1 = data_[i].x[0];
            double x2 = data_[i].x[1];
            Y[i][0] = data_[i].y;
            X[i] = {1.0, x1, x2, x1*x1, x2*x2, x1*x2};
        }

        // β = (X'X)^-1 X'Y
        auto Xt = linalg::transpose(X);
        auto XtX = linalg::multiply(Xt, X);
        auto XtY = linalg::multiply(Xt, Y);
        auto XtX_inv = linalg::inverse(XtX);
        auto B = linalg::multiply(XtX_inv, XtY);

        beta_.resize(p);
        for (int i = 0; i < p; ++i) beta_[i] = B[i][0];
    }

    // Predict response for new x
    double predict(const std::vector<double>& x) const {
        if (beta_.empty()) throw std::runtime_error("Call fit() first");
        double x1 = x[0], x2 = x[1];
        return beta_[0] + beta_[1]*x1 + beta_[2]*x2
             + beta_[3]*x1*x1 + beta_[4]*x2*x2 + beta_[5]*x1*x2;
    }

    // Print coefficients
    void summary() const {
        if (beta_.empty()) throw std::runtime_error("Call fit() first");
        std::cout << "Response Surface coefficients:\n";
        std::cout << "β0=" << beta_[0] << " β1=" << beta_[1]
                  << " β2=" << beta_[2] << " β11=" << beta_[3]
                  << " β22=" << beta_[4] << " β12=" << beta_[5] << "\n";
    }

private:
    struct Point { std::vector<double> x; double y; };
    std::vector<Point> data_;
    std::vector<double> beta_;
};