#include <gtest/gtest.h>
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
