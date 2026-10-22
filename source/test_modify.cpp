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


/*      нестандартный размер матриц      */
TEST(Empty, NullInitialize) {
Matrix m1 = {};
Matrix m1_copy = m1;
m1_copy.clear();
EXPECT_EQ(m1.rows(), m1_copy.rows());
EXPECT_EQ(m1.columns(), m1_copy.columns());

Matrix m2 = std::initializer_list<std::initializer_list<double>>{{}};
Matrix m2_copy = m2;
m2_copy.clear();
EXPECT_EQ(m2.rows(), m2_copy.rows());
EXPECT_EQ(m2.columns(), m2_copy.columns());


Matrix m3 = std::initializer_list<std::initializer_list<double>>{{1}};
Matrix m4 = {1};
EXPECT_EQ(m3, m4);
EXPECT_EQ(m3.rows(), m4.rows());
EXPECT_EQ(m3.columns(), m4.columns());
}

TEST(Empty, ZeroSizesGiveEmptyMatrix) {
EXPECT_TRUE(Matrix(0).empty());
EXPECT_EQ(Matrix(0).columns(), 0u);
EXPECT_TRUE(Matrix(std::initializer_list<double>{}).empty());
EXPECT_TRUE(transpose(Matrix()).empty());
EXPECT_EQ(Matrix() + Matrix(), Matrix());
EXPECT_THROW(Matrix() + Matrix(1, 1), std::runtime_error);
EXPECT_THROW(Matrix()(0, 0), std::runtime_error);
}


TEST(Empty, ArithmeticWithEmpty) {
Matrix empty;
Matrix nonempty = {{1, 2}, {3, 4}};

// вычитание пустых матриц
EXPECT_EQ(empty - empty, Matrix());
EXPECT_THROW(empty - nonempty, std::runtime_error);
EXPECT_THROW(nonempty - empty, std::runtime_error);

// унарный минус
EXPECT_EQ(-empty, Matrix());

// умножение на скаляр
EXPECT_EQ(empty * 5.0, Matrix());
EXPECT_EQ(2.5 * empty, Matrix());
empty *= 10.0;
EXPECT_TRUE(empty.empty());

// матричное умножение
EXPECT_TRUE((Matrix(0, 0) * Matrix(0, 0)).empty());
}

TEST(Empty, PropertiesOfEmpty) {
Matrix empty;

EXPECT_EQ(empty.size(), 0u);
EXPECT_EQ(empty.capacity(), 0u);
EXPECT_TRUE(empty.begin() == empty.end());

// норма, след, детерминант пустой матрицы
EXPECT_THROW(empty.norm(), std::runtime_error);
EXPECT_THROW(empty.trace(), std::runtime_error);
EXPECT_THROW(empty.det(), std::runtime_error);

// ранг пустой матрицы = 0
EXPECT_EQ(empty.rank(), 0u);
}

TEST(Empty, ModifyEmpty) {
Matrix empty;

empty.reshape(0, 0);
EXPECT_TRUE(empty.empty());

empty.reserve(50);
EXPECT_EQ(empty.capacity(), 50u);
EXPECT_TRUE(empty.empty());

empty.reshape(2, 3);
EXPECT_FALSE(empty.empty());

empty.clear();
EXPECT_EQ(empty.capacity(), 50u); // сохранила capacity

empty.shrink_to_fit();
EXPECT_EQ(empty.capacity(), 0u);
}

TEST(Empty, SwapWithEmpty) {
Matrix empty1, empty2;
Matrix m = {{1, 2}, {3, 4}};

empty1.swap(empty2);
EXPECT_TRUE(empty1.empty());

empty1.swap(m);
EXPECT_FALSE(empty1.empty());
EXPECT_EQ(empty1.rows(), 2u);
EXPECT_TRUE(m.empty());
}

TEST(Empty, AssignmentWithEmpty) {
Matrix empty;

Matrix copy = empty;
EXPECT_TRUE(copy.empty());

Matrix m = {{1, 2, 3}, {4, 5, 6}};
m = empty;
EXPECT_TRUE(m.empty());
EXPECT_EQ(m.size(), 0u);
EXPECT_EQ(m.capacity(), 6u);
}

TEST(Empty, OutputEmpty) {
Matrix empty;
std::ostringstream out;
out << empty;
// пустая матрица должна вывести пустую строку или ничего
EXPECT_EQ(out.str(), "");
}