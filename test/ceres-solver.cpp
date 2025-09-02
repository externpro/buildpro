#include <vector>

#include <ceres/ceres.h>
#include <gtest/gtest.h>

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solve;
using ceres::Solver;

// Data generated using the quadratic y = 0.5x^2 - 2x + 1 with some noise
const std::vector<double> x_data = {
  0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
const std::vector<double> y_data = {
  1.1, -0.6, -1.9, -1.1, 1.2, 4.1, 7.8, 13.2, 17.1, 24.3};

// Cost function for curve fitting
struct QuadraticCostFunction
{
  QuadraticCostFunction(double x, double y) : x_(x), y_(y) { }

  template <typename T>
  bool operator()(const T* const a,
                  const T* const b,
                  const T* const c,
                  T* residual) const
  {
    // f(x) = ax^2 + bx + c
    residual[0] = y_ - (a[0] * x_ * x_ + b[0] * x_ + c[0]);
    return true;
  }

private:
  const double x_;
  const double y_;
};

TEST(CeresSolverTest, QuadraticCurveFitting)
{
  // Initial parameter values
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;

  // Build the problem
  Problem problem;
  for (size_t i = 0; i < x_data.size(); ++i)
  {
    CostFunction* cost_function =
      new AutoDiffCostFunction<QuadraticCostFunction, 1, 1, 1, 1>(
        new QuadraticCostFunction(x_data[i], y_data[i]));
    problem.AddResidualBlock(cost_function, nullptr, &a, &b, &c);
  }

  // Run the solver
  Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = true;
  options.logging_type = ceres::SILENT;
  Solver::Summary summary;
  Solve(options, &problem, &summary);

  // Print the results
  std::cout << "\nCeres Solver Report:\n";
  std::cout << summary.BriefReport() << "\n";
  std::cout << "Final a: " << a << " b: " << b << " c: " << c << "\n\n";

  // Check if the solution is reasonable
  // We expect a ≈ 0.5, b ≈ -2.0, c ≈ 1.0
  EXPECT_NEAR(a, 0.5, 0.2);
  EXPECT_NEAR(b, -2.0, 0.5);
  EXPECT_NEAR(c, 1.0, 0.5);
}
