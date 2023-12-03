#pragma once

#include <vector>
#include <complex>

// Could make the functions more generating and let them take an iterator maybe even to things that are not complex numbers, but are convertible to them.
void dft(std::vector<std::complex<double>>& inOut);
void inverseDft(std::vector<std::complex<double>>& inOut);

void fft(std::vector<std::complex<double>>& inOut);
void inverseFft(std::vector<std::complex<double>>& inOut);