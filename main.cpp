#include "iostream"
#include "matrix.h"

using linalg::Matrix;

int main() {
    Matrix m = {{1,2,333},{-4, 0.12345, 0}};
    std::cout << m;
}