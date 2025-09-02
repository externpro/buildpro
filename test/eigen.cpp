#include <eigen3/Eigen/Dense>

#include <gtest/gtest.h>

TEST(EigenTest, VectorOperations)
{
  // Test vector initialization and basic operations
  Eigen::Vector3d v1(1, 2, 3);
  Eigen::Vector3d v2(4, 5, 6);

  // Test vector addition
  Eigen::Vector3d sum = v1 + v2;
  EXPECT_DOUBLE_EQ(sum[0], 5.0);
  EXPECT_DOUBLE_EQ(sum[1], 7.0);
  EXPECT_DOUBLE_EQ(sum[2], 9.0);

  // Test scalar multiplication
  Eigen::Vector3d scaled = v1 * 2.0;
  EXPECT_DOUBLE_EQ(scaled[0], 2.0);
  EXPECT_DOUBLE_EQ(scaled[1], 4.0);
  EXPECT_DOUBLE_EQ(scaled[2], 6.0);

  // Test dot product
  double dot = v1.dot(v2);
  EXPECT_DOUBLE_EQ(dot, 32.0);

  // Test cross product
  Eigen::Vector3d cross = v1.cross(v2);
  EXPECT_DOUBLE_EQ(cross[0], -3.0);
  EXPECT_DOUBLE_EQ(cross[1], 6.0);
  EXPECT_DOUBLE_EQ(cross[2], -3.0);
}

TEST(EigenTest, MatrixOperations)
{
  // Test matrix initialization and operations
  Eigen::Matrix2d m1;
  m1 << 1, 2, 3, 4;

  Eigen::Matrix2d m2;
  m2 << 5, 6, 7, 8;

  // Test matrix addition
  Eigen::Matrix2d sum = m1 + m2;
  EXPECT_DOUBLE_EQ(sum(0, 0), 6.0);
  EXPECT_DOUBLE_EQ(sum(0, 1), 8.0);
  EXPECT_DOUBLE_EQ(sum(1, 0), 10.0);
  EXPECT_DOUBLE_EQ(sum(1, 1), 12.0);

  // Test matrix multiplication
  Eigen::Matrix2d product = m1 * m2;
  EXPECT_DOUBLE_EQ(product(0, 0), 19.0);
  EXPECT_DOUBLE_EQ(product(0, 1), 22.0);
  EXPECT_DOUBLE_EQ(product(1, 0), 43.0);
  EXPECT_DOUBLE_EQ(product(1, 1), 50.0);

  // Test scalar multiplication
  Eigen::Matrix2d scaled = m1 * 2.0;
  EXPECT_DOUBLE_EQ(scaled(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(scaled(0, 1), 4.0);
  EXPECT_DOUBLE_EQ(scaled(1, 0), 6.0);
  EXPECT_DOUBLE_EQ(scaled(1, 1), 8.0);
}

TEST(EigenTest, MatrixInverseAndDeterminant)
{
  // Test matrix inversion and determinant calculation
  Eigen::Matrix3d m;
  m << 4, 7, 2, 2, 6, 1, 3, 8, 5;

  double det = m.determinant();
  EXPECT_NEAR(det, 35.0, 1e-10);

  if (det != 0)
  { // Only test inverse if matrix is invertible
    Eigen::Matrix3d inv = m.inverse();
    Eigen::Matrix3d identity = m * inv;

    // Verify that m * m^-1 is approximately the identity matrix
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        if (i == j)
        {
          EXPECT_NEAR(identity(i, j), 1.0, 1e-10);
        }
        else
        {
          EXPECT_NEAR(identity(i, j), 0.0, 1e-10);
        }
      }
    }
  }
}

TEST(EigenTest, LinearAlgebra)
{
  // Test solving linear systems
  Eigen::Matrix3d A;
  A << 1, 2, 3, 4, 5, 6, 7, 8, 10;

  Eigen::Vector3d b(3, 3, 4);

  // Solve Ax = b
  Eigen::Vector3d x = A.colPivHouseholderQr().solve(b);

  // Verify the solution
  Eigen::Vector3d expected(-2, 1, 1);
  EXPECT_NEAR((x - expected).norm(), 0.0, 1e-10);
}
