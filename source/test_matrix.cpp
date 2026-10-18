#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "matrix.h"

using linalg::Matrix;

/*      пункты 2-3: конструкторы и присваивания      */

TEST(Constructors, SizesAndShapes) {
Matrix m0;
EXPECT_TRUE(m0.empty());

Matrix m1(4), m2(4, 6);
EXPECT_EQ(m1.rows(), 4u);  EXPECT_EQ(m1.columns(), 1u);
EXPECT_EQ(m2.rows(), 4u);  EXPECT_EQ(m2.columns(), 6u);
EXPECT_EQ(m2.size(), 24u);
for (double v : m2) EXPECT_EQ(v, 0.0);           // дефолтные элементы = нули

Matrix m5 = { {1, 2, 3}, {4, 5, 6} };
Matrix m6 = { {1, 2, 3, 4, 5, 6} };
Matrix m7 = { 1, 2, 3, 4, 5, 6 };
EXPECT_EQ(m5.rows(), 2u);  EXPECT_EQ(m5.columns(), 3u);
EXPECT_EQ(m6.rows(), 1u);  EXPECT_EQ(m6.columns(), 6u);
EXPECT_EQ(m7.rows(), 6u);  EXPECT_EQ(m7.columns(), 1u);
EXPECT_EQ(m5(1, 2), 6.0);
}

TEST(Constructors, CopyAndMove) {
Matrix a = { {1, 2}, {3, 4} };
Matrix copy(a);
EXPECT_EQ(copy, a);
copy(0, 0) = 100;
EXPECT_EQ(a(0, 0), 1.0);                          // копия независима

Matrix moved(std::move(a));
EXPECT_EQ(moved(1, 1), 4.0);
EXPECT_TRUE(a.empty());
}

TEST(Constructors, RaggedInitListThrows) {
EXPECT_THROW((Matrix{ {1, 2}, {3} }), std::runtime_error);
}

TEST(Assignment, CopyReusesCapacity) {
Matrix big(4, 6), small = { {1, 2}, {3, 4} };
size_t cap = big.capacity();
big = small;
EXPECT_EQ(big, small);
EXPECT_EQ(big.capacity(), cap);                   // память не перевыделялась
}

TEST(Assignment, Move) {
Matrix m(2, 2);
m = Matrix{ 1, 2, 3, 4, 5, 6 };
EXPECT_EQ(m.rows(), 6u);
EXPECT_EQ(m(5, 0), 6.0);
}

/*      пункт 4: модификация и доступ      */

TEST(Modify, ReshapeReserveClearShrink) {
Matrix m(2, 3);
m.reshape(3, 2);
EXPECT_EQ(m.rows(), 3u);  EXPECT_EQ(m.columns(), 2u);
EXPECT_EQ(m.capacity(), 6u);

m.reserve(100);
EXPECT_EQ(m.capacity(), 100u);
m.reshape(1, 1);
EXPECT_EQ(m.capacity(), 100u);                    // уменьшение размера память не трогает

m.shrink_to_fit();
EXPECT_EQ(m.capacity(), 1u);

m.clear();
EXPECT_TRUE(m.empty());
EXPECT_EQ(m.capacity(), 1u);
}

TEST(Modify, Swap) {
Matrix a = { 1, 2 }, b = { {1, 2, 3} };
a.swap(b);
EXPECT_EQ(a.rows(), 1u);  EXPECT_EQ(b.rows(), 2u);
swap(a, b);
EXPECT_EQ(a.rows(), 2u);  EXPECT_EQ(b.rows(), 1u);
}

TEST(Access, CallOperator) {
Matrix m = { {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0} };
EXPECT_EQ(m(0, 2), 3.0);
m(0, 2) = 7.0;
EXPECT_EQ(m(0, 2), 7.0);
EXPECT_THROW(m(2, 0), std::runtime_error);

const Matrix c = m;
// присваивание в константную матрицу не должно компилироваться
static_assert(!std::is_assignable<decltype(c(0, 0)), double>::value, "const matrix must be read-only");
EXPECT_EQ(c(0, 2), 7.0);
}

/*      пункт 5: вывод      */

TEST(Output, Aligned) {
Matrix m = { {1, 333, 1}, {22, 1, 333} };
std::ostringstream out;
out << m;
EXPECT_EQ(out.str(), "| 1 333   1|\n|22   1 333|\n");
}

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

/*
 * Matrix m1(4) даст матрицу 4x1, это верно. Но Matrix m{4} вызовет initializer_list
 * (матрица из одного элемента 4.0). Это нормальное поведение C++, просто помните об этом в тестах.
 * */