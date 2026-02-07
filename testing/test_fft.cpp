#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <cassert>

// Copying iterative FFT for testing
const double PI = 3.14159265358979323846;
void fft_iterative(std::vector<std::complex<double>>& a) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len;
        std::complex<double> wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1);
            for (int j = 0; j < len / 2; j++) {
                std::complex<double> u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

int main() {
    const int N = 8;
    std::vector<std::complex<double>> data(N);
    
    // DC component
    for (int i = 0; i < N; ++i) data[i] = 1.0;
    fft_iterative(data);
    assert(std::abs(data[0].real() - 8.0) < 1e-9);
    for (int i = 1; i < N; ++i) assert(std::abs(data[i]) < 1e-9);
    
    // Sine wave at bin 1
    for (int i = 0; i < N; ++i) data[i] = std::complex<double>(cos(2.0 * PI * i / N), 0);
    fft_iterative(data);
    // Bin 1 and N-1 should be 4.0
    assert(std::abs(data[1].real() - 4.0) < 1e-9);
    assert(std::abs(data[7].real() - 4.0) < 1e-9);

    std::cout << "FFT logic tests passed!" << std::endl;
    return 0;
}
