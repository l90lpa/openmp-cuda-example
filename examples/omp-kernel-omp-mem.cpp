#include <omp.h>

#include <iostream>
#include <cassert>
#include <vector>

// Function to perform matrix-vector multiplication
template<typename ScalarType>
void mv_mult(const ScalarType * const A,
        const ScalarType * const x,
        ScalarType * const y,
        int rows,
        int cols) {

    // #pragma omp target
    #pragma omp target map(to: A[0:rows*cols], x[0:cols]) map(from: y[0:cols])
    #pragma omp teams loop
    for (int i = 0; i < rows; ++i) {
        ScalarType sum = 0;
        #pragma omp loop reduction(+:sum)
        for (int j = 0; j < cols; ++j) {
            sum += A[i * cols + j] * x[j];
        }
        y[i] = sum;
    }
}

int main() {

    const int rows = 32768;
    const int cols = rows;

    using FPType = float;

    const std::vector<FPType> A(rows * cols, 2.0);
    const std::vector<FPType> x(cols, 1.0);
    std::vector<FPType> y(cols, 0.0);
    std::vector<FPType> z(cols, 0.0);

    const FPType* const h_A = A.data();
    const FPType* const h_x = x.data();
    FPType* const h_y = y.data();
    FPType* const h_z = z.data();

    // #pragma omp target enter data map(to: h_A[0:rows*cols], h_x[0:cols]) map(alloc: h_y[0:cols], h_z[0:cols])

    mv_mult(h_A, h_x, h_y, rows, cols);

    mv_mult(h_A, h_y, h_z, rows, cols);

    // #pragma omp target exit data map(from: h_z[0:cols])

    std::cout << "z(0) = " << h_z[0] << " " << ((h_z[0] == (cols * 2.0) * (cols * 2.0)) ? "(pass)" : "(fail)") << std::endl;

    return 0;
}