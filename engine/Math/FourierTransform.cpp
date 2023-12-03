#include "FourierTransform.hpp"
#include "Angles.hpp"
#include "Assertions.hpp"
#include "Types.hpp"
#include <bit>

// https://en.wikipedia.org/wiki/Fourier_series
// A fourier series approximates a function as a sum of scaled sines and cosines or equivalently as offsets and an amplitudes of sinusoids.
// https://en.wikipedia.org/wiki/Fourier_transform
// A fourier transform converts a function of "time" to a function that takes a frequency and returns a phase offset and amplitude of a complex sinusoid = amplitude * e^(offset * TAU * x). This information is encoded into a complex number where the length of the number is the amplitude and the angle is the phase offset. Another way to interpret the complex number is as a sum of cosines and sines * i.

// The inverse of a fourier transform is also a fourier transform, which makes sense because the definition could be interpreted as either taking the average value of a function wrapped around a cricle or as summing up complex sinusoids. The first interpretation extracts frequencies the other adds them up.

// A fourier series requires the function to be perodic if it is made perodic in the range in which it was analysed. This isn't a limitation with a fourier tranform.

// The frequency of 0 means the offset the exponent multiplies out to 0 so the e^exponent = 1.

// If there are 2 DFTs of the same size they can be added to produce a DFT of the sum of the original data. If the data is of different sizes this doesn't work.

// A discrete fourier transform approximates or interpolates a discrete signal as a sum of complex sinusoids of different frequencies. A frequency of 1 is the whole length of the image, 2 is half the image and so one.
static void dftHelper(std::vector<std::complex<double>>& inOut, bool reverse) {
	auto result = inOut;
	for (usize i = 0; i < inOut.size(); i++) {
		std::complex<double> value = 0.0f;
		for (usize j = 0; j < inOut.size(); j++) {
			// Could optimize by precalcuating the nth roots of unity. And rasing them to the jth power here.

			// Rotate j times by the angle TAU * (i / size). i is the frequency j is the evaluated point.
			value += inOut[j] * std::exp(std::complex(0.0, (reverse ? -1 : 1) * TAU<double> * double(i) / double(inOut.size()) * double(j)));
		}
		result[i] = value;
	}
	inOut = result;
}

// Could call with true in dft() and with false in inverseDft(). If both are called with the same value the signal is looks like it is flipped. What actually happens it is wrapped around a circle starting from the angle 0 and then unwrapped from the angle 0 so the point at zero is the same while the others get reversed (x[-n] = x[size - n] so for zero it is still zero 0). So one way to reverse and array would be to insert on element to the start apply a dft twice and remove the first element.
void dft(std::vector<std::complex<double>>& inOut) {
	//dftHelper(inOut, false);
	dftHelper(inOut, true);
}

void inverseDft(std::vector<std::complex<double>>& inOut) {
	dftHelper(inOut, false);
	// Because the result is a sum of n complex sinusoids it has to be scaled down by n.
	for (auto& v : inOut) {
		v /= static_cast<double>(inOut.size());
	}
}

// Fast fourier transform is an implementation of the discrete fourier transform. It exploits the symetries in complex sinusoids (circles / roots of unity). This implementation only works for powers of 2 because it is simpler to implement. When the number is not a power of 2 the fft can be implemented to use any factorization of the number. One way to approximate the dft of arbitrary size could be to interpolate between the samples. Or the fft function could return the power of 2 dft and the inverse function would downsample it to the arbitrary size.
static auto fftHelper(std::vector<std::complex<double>>& inOut) -> void {
	if (inOut.size() <= 1)
		return;

	std::vector<std::complex<double>> even, odd;
	for (usize i = 0; i < inOut.size(); i += 2) even.push_back(inOut[i]);
	for (usize i = 1; i < inOut.size(); i += 2) odd.push_back(inOut[i]);

	fftHelper(even);
	fftHelper(odd);

	for (size_t i = 0; i < inOut.size() / 2; i++) {
		//const auto t = std::exp(std::complex(0.0, -TAU<double> / inOut.size() * i)) * odd[i];
		// Could use std::polar
		const auto t = std::polar(1.0, -2 * PI<double> *i / inOut.size()) * odd[i];
		inOut[i] = even[i] + t;
		inOut[i + inOut.size() / 2] = even[i] - t;
	}
}

auto fft(std::vector<std::complex<double>>& inOut) -> void {
	ASSERT(std::has_single_bit(inOut.size()));
	fftHelper(inOut);
}

auto inverseFft(std::vector<std::complex<double>>& inOut) -> void {
	ASSERT(std::has_single_bit(inOut.size()));

	// Other ways to reverse the array. https://www.dsprelated.com/showarticle/800.php
	// For the reversing FFT changing the sign of the complex exponential exponent doesn't work, because it also shifts the array by one a moves the last element to the first position.
	for (auto& v : inOut) v = std::conj(v);
	fftHelper(inOut);
	for (auto& v : inOut) v = std::conj(v);

	for (auto& v : inOut) {
		v /= static_cast<double>(inOut.size());
	}
}

// A fourier series takes advantage of the fact that any function can be separated into an even and odd part. It approximates the even part using cosines and the odd part using sines. When a signal is finite it can be treated a even or odd function. For example compression algorithms like JPEG and others use only cosines (second paragraph of https://en.wikipedia.org/wiki/Discrete_cosine_transform) which allows to store less data.

// Apparently the coefficients of the cosine transform can be extracted from the fourier transform.
// What is the advantage of using a fourier series over a fourier transform of finite and dicrete data.
// What are negative frequencies. The appear for complex signals.

// A fast fourier transform can also make polynomial multiplication O(n * log(n)). It turns multiplying each component which is a convolution into just multiplication. It does this by reusing the values of even functions evaluated at roots of unity. 
// polynomial terms =[FFT]=> polynomial evaluated at the roots of unity
// multiply the points of the 2 polynomials
// multiplied points of polynomial =[IFFT]=> multiplied polynomial
// A fourier transform can also speed up image kernel convluation or large number multiplication.
// 3Blue1Brown
// https://www.youtube.com/watch?v=851U557j6HE
// https://www.youtube.com/watch?v=KuXjwB4LzSA
// https://www.youtube.com/watch?v=8rrHTtUzyZA
// https://www.youtube.com/watch?v=g8RkArhtCc4
// https://www.youtube.com/watch?v=spUNpyF58BY 
// Differential equations
// https://www.youtube.com/watch?v=ly4S0oi3Yz8
// https://www.youtube.com/watch?v=ToIXSwZ1pJU
// https://www.youtube.com/watch?v=r6sGWTCMz2k // // 17:00
// FFT polynomial multiplicaiton
// https://www.youtube.com/watch?v=h7apO7q16V0
// https://www.youtube.com/watch?v=Ty0JcR6Dvis
// Mechanical fourier seires analyser.
// https://www.youtube.com/watch?v=6dW6VYXp9HM
// Vertasium
// https://www.youtube.com/watch?v=a8FTr2qMutA
// https://www.youtube.com/watch?v=Iuv6hY6zsd0
// https://www.youtube.com/watch?v=cztocbHiiqQ
// https://www.youtube.com/watch?v=GzbKb59my3U And other videos from around that time.
// Fourier transform of example images.
// https://www.youtube.com/watch?v=oACegp4iGi0
// Proofs for fourier transform and series on this website.
// https://lpsa.swarthmore.edu/Fourier/Xforms/FXFS.html