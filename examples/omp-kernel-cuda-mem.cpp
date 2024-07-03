#include <omp.h>
#include <cuda_runtime.h>

#include <iostream>
#include <cassert>
#include <vector>

// Function to perform matrix-vector multiplication
template<typename ScalarType>
void mv_mult(const ScalarType * const A,
             const ScalarType * const x,
             ScalarType * const y,
             long rows,
             long cols) {

    {
        #pragma omp target is_device_ptr(A, x, y)
        #pragma omp teams loop
        for (long i = 0; i < rows; ++i) {
            ScalarType sum = 0;
            #pragma omp loop reduction(+:sum)
            for (long j = 0; j < cols; ++j) {
                sum += A[i * cols + j] * x[j];
            }
            y[i] = sum;
        }
    }
}


int main() {

    const long rows = 32768;
    const long cols = rows;

    using FPType = float;

    const std::vector<FPType> h_A(rows * cols, 2.0);
    const std::vector<FPType> h_x(cols, 1.0);
    std::vector<FPType> h_y(cols, 0.0);
    std::vector<FPType> h_z(cols, 0.0);

    FPType* d_A = nullptr;
    FPType* d_x = nullptr;
    FPType* d_y = nullptr;
    FPType* d_z = nullptr;
    
    cudaMalloc(&d_A, rows * cols * sizeof(FPType));
    cudaMalloc(&d_x, cols * sizeof(FPType));
    cudaMalloc(&d_y, cols * sizeof(FPType));
    cudaMalloc(&d_z, cols * sizeof(FPType));

    cudaMemcpy(d_A, h_A.data(), rows * cols * sizeof(FPType), cudaMemcpyHostToDevice);
    cudaMemcpy(d_x, h_x.data(), cols * sizeof(FPType), cudaMemcpyHostToDevice);
    cudaMemcpy(d_y, h_y.data(), cols * sizeof(FPType), cudaMemcpyHostToDevice);
    cudaMemcpy(d_z, h_z.data(), cols * sizeof(FPType), cudaMemcpyHostToDevice);

    // Perform matrix-vector multiplication
    mv_mult(d_A, d_x, d_y, rows, cols);

    // Perform matrix-vector multiplication
    mv_mult(d_A, d_y, d_z, rows, cols);

    cudaMemcpy(h_z.data(), d_z, cols * sizeof(FPType), cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_x);
    cudaFree(d_y);
    cudaFree(d_z);

    std::cout << "z(0) = " << h_z[0] << " " << ((h_z[0] == (cols * 2.0) * (cols * 2.0)) ? "(pass)" : "(fail)") << std::endl;

    return 0;
}