#include "matrix.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <new>
#include <stdexcept>
#include <vector>

using linalg::Matrix;


/* ============================================================================
   подмена глобального new/delete: считаем живые блоки памяти и умеем
   бросить bad_alloc на N-м по счёту вызове new (имитация нехватки памяти)
   ============================================================================ */
namespace {
    long long g_live = 0;     // выделено и ещё не освобождено блоков
    size_t g_count = 0;       // сколько раз вызвали new с момента arm()
    size_t g_fail_at = 0;     // на каком вызове бросить bad_alloc (0 = не бросать)
    void arm(size_t n) { g_count = 0; g_fail_at = n; }
    void disarm()      { g_fail_at = 0; }
}
void* operator new(size_t size) {
    if (g_fail_at && ++g_count == g_fail_at) throw std::bad_alloc();
    void* p = std::malloc(size ? size : 1);
    if (!p) throw std::bad_alloc();
    ++g_live;
    return p;
}
void operator delete(void* p) noexcept { if (p) { --g_live; std::free(p); } }
void operator delete(void* p, size_t) noexcept { operator delete(p); }


template <class F> double ms(F f) {
    auto t0 = std::chrono::steady_clock::now(); f();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

// std::setw считает байты, а не символы, и на русских буквах ломает выравнивание
static std::string pad(const std::string& s, size_t width, bool right = false) {
    size_t len = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++len;
    std::string fill(len < width ? width - len : 0, ' ');
    return right ? fill + s : s + fill;
}
static void title(const char* s) { std::cout << "\n=== " << s << " ===\n"; }

// ??????? ????????? ?? ????? ?????????? (??????: ??? ???????? ??????, ??? ? ? ??????????)
Matrix naive_multiply(const Matrix& a, const Matrix& b) {
    const size_t n = a.rows(), m = a.columns(), p = b.columns();
    Matrix r;
    r.reshape(n, p);
    const double* pa = a.begin(); const double* pb = b.begin(); double* pc = r.begin();
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < p; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < m; ++k)
                sum += pa[i * m + k] * pb[k * p + j];     // b ???????? ?? ???????: ??? p*8 ????, ?????? ??? ???? ????
            pc[i * p + j] = sum;
        }
    return r;
}

void demo_multiply() {
    title("0. перемножение: i-j-k (наивное) vs i-k-j (библиотечное)");
    std::cout << pad("размер", 12, true) << pad("наивное", 12, true) << pad("библиотека", 12, true)
              << pad("ускорение", 12, true) << "\n";
    for (size_t n : {128, 256, 512}) {
        Matrix m(n, n);
        for (size_t i = 0; i < m.size(); ++i) m.begin()[i] = double(i % 13) / 7;
        double best_naive = 1e18, best_lib = 1e18;
        Matrix a, b;
        for (int rep = 0; rep < 3; ++rep) {
            best_naive = std::min(best_naive, ms([&]{ a = naive_multiply(m, m); }));
            best_lib   = std::min(best_lib,   ms([&]{ b = m * m; }));
        }
        std::cout << std::setw(8) << n << "x" << std::left << std::setw(3) << n << std::right
                  << std::setw(12) << best_naive << std::setw(12) << best_lib
                  << std::setw(11) << best_naive / best_lib << "x" << (a == b ? "" : "  ?????????? ??????") << "\n";
    }
}


/* ============================================================================
   1. транспонирование: наивное (на сырых указателях, т.е. честно) против блочного
   ============================================================================ */
Matrix naive_transpose(const Matrix& m) {
    const size_t R = m.rows(), C = m.columns();
    Matrix r;
    r.reshape(C, R);                               // та же выделяемая память, что и у transpose()
    const double* s = m.begin();
    double* d = r.begin();
    for (size_t i = 0; i < R; ++i)
        for (size_t j = 0; j < C; ++j)
            d[j * R + i] = s[i * C + j];
    return r;
}

void demo_transpose() {
    title("1. транспонирование: наивное vs блочное мс");
    std::cout << pad("размер", 12, true) << pad("наивное", 12, true) << pad("блочное", 12, true)
              << pad("ускорение", 12, true) << "\n";
    for (size_t n : {2000, 4000, 4096}) {
        Matrix m(n, n);
        for (size_t i = 0; i < m.size(); ++i) m.begin()[i] = double(i % 97);
        double best_naive = 1e18, best_block = 1e18;
        Matrix a, b;
        for (int rep = 0; rep < 5; ++rep) {
            best_naive = std::min(best_naive, ms([&]{ a = naive_transpose(m); }));
            best_block = std::min(best_block, ms([&]{ b = transpose(m); }));
        }
        std::cout << std::setw(8) << n << "x" << std::left << std::setw(3) << n << std::right
                  << std::setw(12) << best_naive << std::setw(12) << best_block
                  << std::setw(11) << best_naive / best_block << "x" << (a == b ? "" : "  РЕЗУЛЬТАТЫ РАЗНЫЕ") << "\n";
    }
}


/* ============================================================================
   2. гаусс: что именно даёт выбор ведущего и относительный порог
   ============================================================================ */

// учебниковый гаусс: ведущий = просто a(k,k), без перестановок, без допусков
Matrix naive_solve(Matrix a, Matrix f) {
    const size_t n = a.rows();
    for (size_t k = 0; k < n; ++k)
        for (size_t i = k + 1; i < n; ++i) {
            double factor = a(i, k) / a(k, k);
            for (size_t j = k; j < n; ++j) a(i, j) -= factor * a(k, j);
            f(i, 0) -= factor * f(k, 0);
        }
    Matrix x(n, 1);
    for (size_t i = n; i-- > 0;) {
        double s = f(i, 0);
        for (size_t j = i + 1; j < n; ++j) s -= a(i, j) * x(j, 0);
        x(i, 0) = s / a(i, i);
    }
    return x;
}

struct Res { size_t rank; double det; };
// то же, что делает библиотека (выбор ведущего), но порог нуля задаётся снаружи
Res eliminate(Matrix a, double tol) {
    const size_t n = a.rows();
    bool odd = false; size_t rank = 0; double det = 1.0;
    for (size_t col = 0; col < n && rank < n; ++col) {
        size_t best = rank;
        for (size_t r = rank + 1; r < n; ++r) if (std::fabs(a(r, col)) > std::fabs(a(best, col))) best = r;
        if (std::fabs(a(best, col)) <= tol) continue;
        if (best != rank) { for (size_t c = 0; c < n; ++c) std::swap(a(best, c), a(rank, c)); odd = !odd; }
        for (size_t r = rank + 1; r < n; ++r) {
            double factor = a(r, col) / a(rank, col);
            for (size_t c = col; c < n; ++c) a(r, c) -= factor * a(rank, c);
        }
        det *= a(rank, col);
        ++rank;
    }
    if (rank < n) det = 0.0; else if (odd) det = -det;
    return {rank, det};
}
double max_abs(const Matrix& m) { double r = 0; for (double v : m) r = std::max(r, std::fabs(v)); return r; }

void demo_gauss() {
    title("2a. выбор ведущего элемента: решаем A*x = f, учебниковый гаусс vs библиотека");
    struct Case { const char* name; Matrix A, f; const char* expect; };
    Case cases[] = {
        {"A=[[1e-20,1],[1,1]], f=[1,2]", {{1e-20, 1}, {1, 1}}, {1, 2}, "(1, 1)"},
        {"A=[[0,1],[1,0]],     f=[1,2]", {{0, 1}, {1, 0}},     {1, 2}, "(2, 1)"},
    };
    for (auto& c : cases) {
        Matrix a = naive_solve(c.A, c.f), b = solve(c.A, c.f);
        std::cout << c.name << "   верно " << c.expect << "\n"
                  << "    учебниковый: (" << a(0, 0) << ", " << a(1, 0) << ")\n"
                  << "    библиотека:  (" << b(0, 0) << ", " << b(1, 0) << ")\n";
    }
    // случайная система: сравниваем невязку |A*x - f|
    for (size_t n : {100, 300}) {
        Matrix A(n, n), f(n, 1);
        unsigned long long s = 12345;
        auto rnd = [&]{ s = s * 6364136223846793005ULL + 1442695040888963407ULL; return double(s >> 11) / double(1ULL << 53) * 2 - 1; };
        for (double& v : A) v = rnd();
        for (double& v : f) v = rnd();
        double r1 = (A * naive_solve(A, f) - f).norm(), r2 = (A * solve(A, f) - f).norm();
        std::cout << "случайная " << n << "x" << n << ": невязка учебниковый " << r1 << ", библиотека " << r2 << "\n";
    }

}


/* ============================================================================
   3. безопасность исключений: тест с принудительным отказом new на каждой точке
   ============================================================================ */
struct Snap { std::vector<double> v; size_t rows, cols, cap; };
Snap snap(const Matrix& m) { return {std::vector<double>(m.begin(), m.end()), m.rows(), m.columns(), m.capacity()}; }
bool same(const Matrix& m, const Snap& s) {
    return m.rows() == s.rows && m.columns() == s.cols && m.capacity() == s.cap && std::equal(m.begin(), m.end(), s.v.begin());
}
// инвариант: size() <= capacity() и (capacity == 0 <=> нет массива)
bool valid(const Matrix& m) { return m.size() <= m.capacity() && ((m.capacity() == 0) == (m.begin() == nullptr)); }

struct Out { bool threw = false, valid = true, unchanged = true; };

// перебирает n = 1, 2, 3...: «n-й вызов new бросает bad_alloc», пока операция не начнёт выполняться без отказа
template <class Scenario>
void run(const char* name, bool has_state, Scenario sc) {
    int failures = 0, leaks = 0, broken = 0, changed = 0;
    for (size_t n = 1; n < 500; ++n) {
        long long live0 = g_live;
        Out o;
        sc(n, o);
        if (g_live != live0) ++leaks;               // после выхода из сценария все матрицы уничтожены: живых блоков стать больше не должно
        if (!o.threw) break;
        ++failures;
        if (!o.valid) ++broken;
        if (!o.unchanged) ++changed;
    }
    std::cout << pad(name, 38) << pad(std::to_string(failures), 9, true) << pad(std::to_string(leaks), 9, true)
              << pad(std::to_string(broken), 11, true) << pad(has_state ? std::to_string(changed) : "-", 14, true) << "\n";
}

// игрушечный буфер с НЕПРАВИЛЬНЫМ порядком в присваивании: сначала delete[], потом new
struct WeakBuf;
__attribute__((noinline)) void weak_assign(WeakBuf& dst, const WeakBuf& src);
struct WeakBuf {
    double* p = nullptr; size_t n = 0;
    explicit WeakBuf(size_t k) : p(new double[k]()), n(k) {}
    ~WeakBuf() { delete[] p; }
};
void weak_assign(WeakBuf& dst, const WeakBuf& src) {
    delete[] dst.p; dst.p = nullptr; dst.n = 0;       // старое уже уничтожено...
    dst.p = new double[src.n];                        // ...а если вот тут бросит - данные потеряны
    dst.n = src.n; std::copy(src.p, src.p + src.n, dst.p);
}

void demo_exceptions() {
    title("3. безопасность исключений: n-й вызов new бросает bad_alloc, n = 1, 2, 3, ...");
    std::cout << pad("операция", 38) << pad("отказов", 9, true) << pad("утечек", 9, true)
              << pad("инвариант", 11, true) << pad("состояние", 14, true) << "\n"
              << pad("", 38) << pad("(точек)", 9, true) << pad("(штук)", 9, true)
              << pad("(наруш.)", 11, true) << pad("(изменилось)", 14, true) << "\n";

    run("b = a  (нужно перевыделение)", true, [](size_t n, Out& o) {
        Matrix a(4, 4), b = {{1, 2}, {3, 4}};
        Snap before = snap(b);
        arm(n); try { b = a; } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(b); if (o.threw) o.unchanged = same(b, before);
    });
    run("m.reserve(100)", true, [](size_t n, Out& o) {
        Matrix m = {{1, 2}, {3, 4}};
        Snap before = snap(m);
        arm(n); try { m.reserve(100); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(m); if (o.threw) o.unchanged = same(m, before);
    });
    run("m.shrink_to_fit()", true, [](size_t n, Out& o) {
        Matrix m = {{1, 2}, {3, 4}}; m.reserve(100);
        Snap before = snap(m);
        arm(n); try { m.shrink_to_fit(); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(m); if (o.threw) o.unchanged = same(m, before);
    });
    run("m.reshape(50, 50)", true, [](size_t n, Out& o) {
        Matrix m = {{1, 2}, {3, 4}};
        Snap before = snap(m);
        arm(n); try { m.reshape(50, 50); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(m); if (o.threw) o.unchanged = same(m, before);
    });
    run("x *= y  (матричное умножение)", true, [](size_t n, Out& o) {
        Matrix x = {{1, 2}, {3, 4}}, y = {{5, 6}, {7, 8}};
        Snap before = snap(x);
        arm(n); try { x *= y; } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(x); if (o.threw) o.unchanged = same(x, before);
    });
    run("x = solve(A, f)", true, [](size_t n, Out& o) {
        Matrix A = {{2, 1, 0}, {1, 3, 1}, {0, 1, 4}}, f = {1, 2, 3}, x = {9, 9, 9};
        Snap before = snap(x);
        arm(n); try { x = solve(A, f); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(x); if (o.threw) o.unchanged = same(x, before);
    });
    run("x = power(A, -5)  (invert + умножения)", true, [](size_t n, Out& o) {
        Matrix A = {{2, 1, 0}, {1, 3, 1}, {0, 1, 4}}, x = {{1, 1}, {1, 1}};
        Snap before = snap(x);
        arm(n); try { x = power(A, -5); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
        o.valid = valid(x); if (o.threw) o.unchanged = same(x, before);
    });
    run("Matrix c(a)  (конструктор копирования)", false, [](size_t n, Out& o) {
        Matrix a(10, 10);
        arm(n); try { Matrix c(a); } catch (const std::bad_alloc&) { o.threw = true; } disarm();
    });
    run("Matrix m = {{..},{..}}  (init list)", false, [](size_t n, Out& o) {
        arm(n); try { Matrix m = {{1, 2, 3}, {4, 5, 6}}; } catch (const std::bad_alloc&) { o.threw = true; } disarm();
    });

    std::cout << "\nдля сравнения: присваивание с неправильным порядком (delete[] до new):\n";
    int failures = 0, changed = 0;
    for (size_t n = 1; n < 10; ++n) {
        WeakBuf a(4), b(2); double* old = b.p; size_t old_n = b.n; bool threw = false;
        arm(n); try { weak_assign(b, a); } catch (const std::bad_alloc&) { threw = true; } disarm();
        if (!threw) break;
        ++failures; if (b.p != old || b.n != old_n) ++changed;
    }
    std::cout << "    отказов " << failures << ", из них состояние потеряно: " << changed << "  (объект валиден, но данные пропали)\n";
}

#if defined(_WIN64) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64)
#include <windows.h>
#endif
int main() {
#if defined(_WIN64) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64)
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "КОМПИЛИРОВАТЬ ЧЕРЕЗ Release!!!!! (так прирост больше)\n\n";
    demo_multiply();
    demo_transpose();
    demo_gauss();
    demo_exceptions();
}
