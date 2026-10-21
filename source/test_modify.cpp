#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include "matrix.h"

using linalg::Matrix;

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
Matrix m = { {1.2, 333, 1}, {22, 1, 333}, {1.111, -1, 0.0} };
std::ostringstream out;
out << m;
EXPECT_EQ(out.str(), "|  1.2 333   1|\n|   22   1 333|\n|1.111  -1   0|\n");
}