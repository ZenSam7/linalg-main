#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include "matrix.h"

using linalg::Matrix;


/*      пункт 6: арифметика      */

TEST(Arithmetic, Basic) {
Matrix a = { {1, 2}, {3, 4} }, b = { {5, 6}, {7, 8} };
EXPECT_EQ(+a, a);
EXPECT_EQ(a + b, (Matrix{ {6, 8}, {10, 12} }));
EXPECT_EQ(b - a, (Matrix{ {4, 4}, {4, 4} }));
EXPECT_EQ(-a, (Matrix{ {-1, -2}, {-3, -4} }));
EXPECT_EQ(a * b, (Matrix{ {19, 22}, {43, 50} }));
EXPECT_EQ(2 * a, a * 2.0);
EXPECT_EQ(a * 2.0, (Matrix{ {2, 4}, {6, 8} }));
EXPECT_NE(a, b);
}

TEST(Arithmetic, CompoundAssignments) {
Matrix a = { {1, 2}, {3, 4} };
Matrix b = a;
(a += b) += b;
EXPECT_EQ(a, b * 3.0);
a -= b;
a *= 0.5;
EXPECT_EQ(a, b);
a *= b;
EXPECT_EQ(a, (Matrix{ {7, 10}, {15, 22} }));
}

TEST(Arithmetic, FloatingPointComparison) {
EXPECT_EQ(Matrix{ 0.3 * 3 }, Matrix{ 0.9 });
}

TEST(Arithmetic, IncompatibleShapesThrow) {
Matrix a(2, 3), b(3, 2);
EXPECT_THROW(a + b, std::runtime_error);
EXPECT_THROW(a - b, std::runtime_error);
EXPECT_THROW(a * a, std::runtime_error);
EXPECT_NO_THROW(a * b);
}

/*      пункт 7: вычисления      */

TEST(Compute, NormTraceDetRank) {
Matrix v = { 3, 4 };
EXPECT_DOUBLE_EQ(v.norm(), 5.0);

Matrix a = { {2, 1, 0}, {1, 3, 1}, {0, 1, 4} };
EXPECT_DOUBLE_EQ(a.trace(), 9.0);
EXPECT_NEAR(a.det(), 18.0, 1e-9);
EXPECT_EQ(a.rank(), 3u);

Matrix s = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
EXPECT_EQ(s.rank(), 2u);
EXPECT_EQ(s.det(), 0.0);
EXPECT_THROW(Matrix(2, 3).det(), std::runtime_error);
}

TEST(Compute, GaussReturnsSelf) {
Matrix m = { {2, 1, -1, 8}, {-3, -1, 2, -11}, {-2, 1, 2, -3} };
Matrix& ref = m.gauss_forward();
EXPECT_EQ(&ref, &m);
m.gauss_backward();
EXPECT_EQ(m, (Matrix{ {1, 0, 0, 2}, {0, 1, 0, 3}, {0, 0, 1, -1} }));
}

TEST(Compute, TransposeConcatenate) {
Matrix a = { {1, 2, 3}, {4, 5, 6} };
EXPECT_EQ(transpose(a), (Matrix{ {1, 4}, {2, 5}, {3, 6} }));
EXPECT_EQ(concatenate(a, a), (Matrix{ {1, 2, 3, 1, 2, 3}, {4, 5, 6, 4, 5, 6} }));
EXPECT_EQ(a.rows(), 2u);                          // аргументы не изменились
EXPECT_THROW(concatenate(a, Matrix(3, 1)), std::runtime_error);
}

TEST(Compute, Solve) {
Matrix A = { {2, 1, 0}, {1, 3, 1}, {0, 1, 4} };
Matrix f = { 1, 2, 3 };
Matrix x = solve(A, f);
EXPECT_EQ(A * x, f);
EXPECT_THROW(solve(Matrix{ {1, 2}, {2, 4} }, Matrix{ 1, 2 }), std::runtime_error);
}

TEST(Compute, SingularInvertThrows) {
EXPECT_THROW(invert(Matrix{ {1, 2}, {2, 4} }), std::runtime_error);
}

/*      пункт 8: тождества из лабы      */

TEST(Identities, Properties) {
Matrix A1 = { {2, 1, 0}, {1, 3, 1}, {0, 1, 4} };
Matrix A2 = { {1, 2, 0}, {0, 1, 3}, {4, 0, 1} };
Matrix E  = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

EXPECT_EQ(power(A1, 0), E);                                        // A^0 = E
EXPECT_EQ(A1 * invert(A1), E);
EXPECT_EQ(invert(transpose(A1)), transpose(invert(A1)));           // (A^T)^-1 = (A^-1)^T
EXPECT_EQ(invert(A1 * A2), invert(A2) * invert(A1));               // (A1*A2)^-1 = A2^-1 * A1^-1
EXPECT_EQ(power(invert(A1), 6), power(power(A1, -2), 3));          // (A^-1)^6 = (A^-2)^3
EXPECT_EQ(power(A1, 4), A1 * A1 * A1 * A1);
}
