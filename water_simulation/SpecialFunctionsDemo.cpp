#include "SpecialFunctionsDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>
#include <bitset>
#include <imgui/imgui_internal.h>
#include <Put.hpp>
#include <engine/Math/Random.hpp>
#include <engine/Math/Color.hpp>
#include <immintrin.h>

float p0 = 1.0000001049376748;
float p1 = 1.0000008118843373;
float p2 = 0.4999861634792701;
float p3 = 0.1666342583862209;
float p4 = 0.0419417709426182;
float p5 = 0.008594001961529957;

std::pair<__m128, __m128> exp_simd(__m128 x) {
	const float invln2 = 1.4426950408889634;
	const float minusLn2 = -0.6931471805599453;

	__m128 k0 = _mm_set1_ps(invln2);
	__m128 k1 = _mm_mul_ps(k0, x);
	__m128 k = _mm_round_ps(k1, _MM_FROUND_NO_EXC);

	__m128 r0 = _mm_set1_ps(minusLn2);
	__m128 r1 = _mm_mul_ps(k, r0);
	__m128 r = _mm_add_ps(x, r1);
	return { k, r };
}

static double one = 1.0,
	halF[2] = { 0.5,-0.5, },
	huge = 1.0e+300,
	twom1000 = 9.33263618503218878990e-302,     /* 2**-1000=0x01700000,0*/
	o_threshold = 7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
	u_threshold = -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
	ln2HI[2] = { 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
		 -6.93147180369123816490e-01, },/* 0xbfe62e42, 0xfee00000 */
	ln2LO[2] = { 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
			 -1.90821492927058770002e-10, },/* 0xbdea39ef, 0x35793c76 */
	invln2 = 1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
	P1 = 1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
	P2 = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
	P3 = 6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
	P4 = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
	P5 = 4.13813679705723846039e-08; /* 0x3E663769, 0x72BEA4D0 */


#define __HI(x) *(1+(int*)&x)
#define __LO(x) *(int*)&x

double __ieee754_exp(double x)	/* default IEEE double exp */
{
	double y, hi, lo, c, t;
	int k, xsb;
	unsigned hx;

	hx = __HI(x);	/* high word of x */
	xsb = (hx >> 31) & 1;		/* sign bit of x */
	hx &= 0x7fffffff;		/* high word of |x| */

	/* filter out non-finite argument */
	if (hx >= 0x40862E42) {			/* if |x|>=709.78... */
		if (hx >= 0x7ff00000) {
			if (((hx & 0xfffff) | __LO(x)) != 0)
				return x + x; 		/* NaN */
			else return (xsb == 0) ? x : 0.0;	/* exp(+-inf)={inf,0} */
		}
		if (x > o_threshold) return huge * huge; /* overflow */
		if (x < u_threshold) return twom1000 * twom1000; /* underflow */
	}

	/* argument reduction */
	if (hx > 0x3fd62e42) {		/* if  |x| > 0.5 ln2 */
		if (hx < 0x3FF0A2B2) {	/* and |x| < 1.5 ln2 */
			hi = x - ln2HI[xsb]; lo = ln2LO[xsb]; k = 1 - xsb - xsb;
		}
		else {
			k = (int)(invln2 * x + halF[xsb]);
			t = k;
			hi = x - t * ln2HI[0];	/* t*ln2HI is exact here */
			lo = t * ln2LO[0];
		}
		x = hi - lo;
	}
	else if (hx < 0x3e300000) {	/* when |x|<2**-28 */
		if (huge + x > one) return one + x;/* trigger inexact */
	}
	else k = 0;

	/* x is now in primary range */
	t = x * x;
	c = x - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
	if (k == 0) 	return one - ((x * c) / (c - 2.0) - x);
	else 		y = one - ((lo - (x * c) / (2.0 - c)) - hi);
	if (k >= -1021) {
		__HI(y) += (k << 20);	/* add k to y's exponent */
		return y;
	}
	else {
		__HI(y) += ((k + 1000) << 20);/* add k to y's exponent */
		return y * twom1000;
	}
}

struct MaxDifferenceResult {
	float maxDifference;
	float maxDifferenceInput;
};

enum class ErrorType {
	ABSOLUTE,
	RELATIVE
};

template <typename Function1, typename Function2>
MaxDifferenceResult maxDifference(Function1 correct, Function2 approximation, float min, float max, ErrorType errorType) {
	const auto count = 2000;
	float maxDif = -std::numeric_limits<float>::infinity();
	float maxDifferenceInput = 0.0f;
	for (int i = 0; i < count; i++) {
		float t = float(i) / float(count - 1);
		float x = ImLerp(min, max, t);
		float y1 = correct(x);
		float y2 = approximation(x);
		float dif = 0.0f;
		switch (errorType) {
		case ErrorType::ABSOLUTE:
			dif = std::abs(y1 - y2);
			break;
		case ErrorType::RELATIVE:
			dif = std::abs((y1 - y2) / y1);
			break;
		}
		if (dif > maxDif) {
			maxDifferenceInput = x;
			maxDif = dif;
		}
	}
	return MaxDifferenceResult{
		.maxDifference = maxDif,
		.maxDifferenceInput = maxDifferenceInput
	};
}

std::pair<int, float> reduce_exp(float x) {
	const float ln2 = 0.6931471805599453; // log(2)
	const float invln2 = 1.4426950408889634; // 1 / log(2)
	//const auto k = floor(muladd(x, invln2, 0.5)); // floor(x / log(2) + 1 / 2)
	//const float k = floorf(x * invln2 + 0.5); // floor(x / log(2) + 1 / 2)
	//const float k = round(x * invln2); // floor(x / log(2) + 1 / 2)
	const float k = round(x * invln2); // floor(x / log(2) + 1 / 2)
	const float r = (k * -ln2 + x); // x - k * ln2

	/*float y = x;
	u32 test = reinterpret_cast<u32>(&y);
	u32 sign = (test >> 31) & 1;
	i32 exponent = i32(test >> 23) & 0xFF - 127;*/

	const auto x1 = _mm_set1_ps(x);
	const auto out = exp_simd(x1);

	/*return (trunc(Int, k), r);*/
	return { int(k), r };
	//return { out.first.m128_f32[0], out.second.m128_f32[0] };
}

int factorial(int n) {
	int result = 1;
	for (int i = 2; i <= n; i++) {
		result *= i;
	}
	return result;
}

float integer_expontentiate(float x, int n) {
	float out = 1.0f;
	for (int i = 1; i <= n; i++) {
		out *= x;
	}
	return out;
}

float exp_taylor(float x, int n) {
	float r = 0.0f;
	for (int i = 0; i < n; i++) {
		r += integer_expontentiate(x, i) / factorial(i);
	}
	return r;
}

float exp_reduce_taylor(float x, int n) {
	const auto [k, r] = reduce_exp(x);
	return exp2(k) * exp_taylor(r, n); // 2 ^ k * exp(r);
}

float ramezPolynomial2(float x) {
	const float R1 = x - (float(P1) * pow(x, 2.0f) + float(P1) * pow(x, 4.0f) + float(P3) * pow(x, 6.0f) + float(P4) * pow(x, 8.0f) + float(P5) * pow(x, 10.0f));
	return 1 + x + (x * R1) / (2 - R1);
}

void printBits(u32 value) {
	std::cout << std::bitset<32>(value) << '\n';
}

const u32 F32_SIGN_BITS = 1;
const u32 F32_EXPONENT_BITS = 8;
const u32 F32_MANTISSA_BITS = 23;
const u32 F32_EXPONENT_MASK = 0xFF << (32 - F32_SIGN_BITS - F32_EXPONENT_BITS);
const u32 F32_EXPONENT_SHIFT = F32_MANTISSA_BITS;
const u32 F32_EXPONENT_BIAS = 127;

float exp_reduce(float x) {
	if (x == -10.0f) {
		int l = 5;
	}
	const auto [k, r] = reduce_exp(x);
	//ASSERT(r >= -0.15051499783 && r <= 0.15051499783);
	//float n = 0.17133096490424785 * (r * r * r) + 0.5011293350199512 * (r * r) + 0.9999325707637068 * r + 0.9999958162681298;
	//float n = 0.1821059865978567 * (r * r * r) + 0.4982706111435472 * (r * r) + 1.0000615265526434 * r + 0.9999996585784304;
	//float n = 0.9999999999998458 + r * (1.0000000004678462 + r * (0.5000000001411278 + r * (0.16666653216374752 + r * (0.041666643575736434 + r * (0.008342999585897208 + r * (0.001390208446652496))))));
	float n = 1.0000000754895593 + r * (1.0000000647006064 + r * (0.4999886914692487 + r * (0.16666325650514743 + r * (0.041917526523052265 + r * (0.008381111717943628)))));

	return exp2(k) * n;
}

/*
Range reduction:
Find an integer k and r that is between -ln(2)/2 and ln(2)/2 such that x = k * ln(2) + r.
This can be done because if r = ln(2)/2 then k * ln(2) + ln(2)/2 = (k + 1) * ln(2) - ln(2)/2.

Calculating r:
k * ln(2) + r = x
r = x - k * ln(2)

Calculating function after range reduction:
e^(k * ln(2) + r) =
e^(k * ln(2)) * e^r =
2^k * e^r
*/
__m256 expSimd(__m256 x) {
	const auto minusLn2 = _mm256_set1_ps(-0.6931471805599453);
	const auto ln2Inv = _mm256_set1_ps(1.4426950408889634);

	const auto kFloat = _mm256_round_ps(_mm256_mul_ps(x, ln2Inv), _MM_FROUND_NO_EXC);
	const auto r = _mm256_fmadd_ps(kFloat, minusLn2, x);

	const auto a0 = _mm256_set1_ps(1.0000000754895593);
	const auto a1 = _mm256_set1_ps(1.0000000647006064);
	const auto a2 = _mm256_set1_ps(0.4999886914692487);
	const auto a3 = _mm256_set1_ps(0.16666325650514743);
	const auto a4 = _mm256_set1_ps(0.041917526523052265);
	const auto a5 = _mm256_set1_ps(0.008381111717943628);

	// exp(r) approximation
	__m256 m;
	m = _mm256_fmadd_ps(r, a5, a4);
	m = _mm256_fmadd_ps(r, m, a3);
	m = _mm256_fmadd_ps(r, m, a2);
	m = _mm256_fmadd_ps(r, m, a1);
	m = _mm256_fmadd_ps(r, m, a0);

	auto kInt = _mm256_cvtps_epi32(kFloat);
	auto exponent = _mm256_add_epi32(kInt, _mm256_set1_epi32(F32_EXPONENT_BIAS));
	// TODO: Maybe could do satured add instead of clamping. The probem is that it has to be able to handle negative values of k and I couldn't find an instruction that converts u32 into u8 in such a way that the u8 are stored in the lower bytes of the u32s.
	exponent = _mm256_max_epi32(_mm256_min_epi32(exponent, _mm256_set1_epi32(255)), _mm256_set1_epi32(0));
	const auto twoToK = _mm256_slli_epi32(exponent, F32_EXPONENT_SHIFT);
	return _mm256_mul_ps(_mm256_castsi256_ps(twoToK), m);
}

/*
Range reduction:
Find an integer k and f that is between 0 and 1 such that x = 2^k * (f + 1).
This can be done because if f = 0 then x = 2^k and if f = 1 then x = 2^k * (1 + 1) = 2^(k+1).

Calculating f:
x = 2^k * (f + 1)
x = 2^k * f + 2^k
x - 2^k = 2^k * f
(x - 2^k) / 2^k = f
x / 2^k - 1 = f

Calculating function after range reduction:
ln(x) = 
ln(2^k * (f + 1)) =
ln(2^k) + ln(f + 1) =
log2(2^k)/log2(e) + ln(f + 1) =
k/log2(e) + ln(f + 1)

TODO: Apparently there is a way to range reduce further into (sqrt(2)/2, sqrt(2)). Used for example in fdlibm and also mentionted here https://math.stackexchange.com/questions/3619158/most-efficient-way-to-calculate-logarithm-numerically.
*/
__m256 lnSimd(__m256 x) {
	x = _mm256_max_ps(x, _mm256_set1_ps(0.0f));
	const auto xBytes = _mm256_castps_si256(x);
	// k is the exponent of x.
	// Calculate 2^k by just making away the mantissa and sign bits of x.
	const auto twoToKBytes = _mm256_and_epi32(xBytes, _mm256_set1_epi32(F32_EXPONENT_MASK));
	const auto twoToK = _mm256_castsi256_ps(twoToKBytes);
	// Bitshift k and debias to get the integer value of it.
	const auto kInt = _mm256_sub_epi32(_mm256_srli_epi32(twoToKBytes, F32_EXPONENT_SHIFT), _mm256_set1_epi32(F32_EXPONENT_BIAS));
	const auto k = _mm256_cvtepi32_ps(kInt);

	const auto f = _mm256_add_ps(_mm256_div_ps(x, twoToK), _mm256_set1_ps(-1.0f));

	const auto a0 = _mm256_set1_ps(6.071404325574954e-05);
	const auto a1 = _mm256_set1_ps(0.9965407439405233);
	const auto a2 = _mm256_set1_ps(-0.46783477377051086);
	const auto a3 = _mm256_set1_ps(0.22089156008190866);
	const auto a4 = _mm256_set1_ps(-0.05657177777848718);

	// minimax log(f + 1) approximation
	__m256 m;
	m = _mm256_fmadd_ps(f, a4, a3);
	m = _mm256_fmadd_ps(f, m, a2);
	m = _mm256_fmadd_ps(f, m, a1);
	m = _mm256_fmadd_ps(f, m, a0);

	const auto invLogBase2OfE = _mm256_set1_ps(0.69314718056);
	return _mm256_fmadd_ps(k, invLogBase2OfE, m);
}

#define LOG_POLY_DEGREE 5
#define POLY0(x, c0) _mm_set1_ps(c0)
#define POLY1(x, c0, c1) _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x, c0, c1, c2) _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x, c0, c1, c2, c3) _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x, c0, c1, c2, c3, c4) _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x, c0, c1, c2, c3, c4, c5) _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), _mm_set1_ps(c0))

__m128 log2f4(__m128 x)
{
	__m128i exp = _mm_set1_epi32(0x7F800000);
	__m128i mant = _mm_set1_epi32(0x007FFFFF);

	__m128 one = _mm_set1_ps(1.0f);

	__m128i i = _mm_castps_si128(x);

	__m128 e = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(i, exp), 23), _mm_set1_epi32(127)));

	__m128 m = _mm_or_ps(_mm_castsi128_ps(_mm_and_si128(i, mant)), one);

	__m128 p;

	/* Minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[ */
#if LOG_POLY_DEGREE == 6
	p = POLY5(m, 3.1157899f, -3.3241990f, 2.5988452f, -1.2315303f, 3.1821337e-1f, -3.4436006e-2f);
#elif LOG_POLY_DEGREE == 5
	p = POLY4(m, 2.8882704548164776201f, -2.52074962577807006663f, 1.48116647521213171641f, -0.465725644288844778798f, 0.0596515482674574969533f);
#elif LOG_POLY_DEGREE == 4
	p = POLY3(m, 2.61761038894603480148f, -1.75647175389045657003f, 0.688243882994381274313f, -0.107254423828329604454f);
#elif LOG_POLY_DEGREE == 3
	p = POLY2(m, 2.28330284476918490682f, -1.04913055217340124191f, 0.204446009836232697516f);
#else
#error
#endif

   /* This effectively increases the polynomial degree by one, but ensures that log2(1) == 0*/
	p = _mm_mul_ps(p, _mm_sub_ps(m, one));

	return _mm_add_ps(p, e);
}


/*
x = 2^k * (1 + f)
x / (1 + f) = 2^k
log2(x / (1 + f)) = log2(2^k)
log2(x) - log(1 + f) = k

x = 2^k + 2^k * f
x - 2^k = 2^k * f
(x - 2^k) / 2^k = f = x / 2^k - 1

ln(2^k * (1 + f)) = ln(2^k) + ln(1 + f) = log2(2^k) / log2(e) = k / log2(e) + ln(1 + f)
*/
float ln_test(float x) {
	x = fmax(x, 0);
	u32 xBytes = std::bit_cast<u32>(x);
	const auto twoToK = std::bit_cast<float>(xBytes & F32_EXPONENT_MASK);
	const int k = ((xBytes & F32_EXPONENT_MASK) >> F32_EXPONENT_SHIFT) - F32_EXPONENT_BIAS;
	// For some reason there is a difference of 1 between log2(x) and k
	//const int k2 = log2(x);
	/*const auto f = x / exp2(float(k)) - 1.0f;*/
	const auto f = x / twoToK - 1.0f;
	const auto r = f;
	const auto n = 6.071404325574954e-05 + r * (0.9965407439405233 + r * (-0.46783477377051086 + r * (0.22089156008190866 + r * (-0.05657177777848718))));

	return lnSimd(_mm256_set1_ps(x)).m256_f32[0];

	return float(k) / log2(2.71828182846) + n;

	//return f;
	//return log2f4(_mm_set1_ps(x)).m128_f32[0] / log2(2.71828182846);
}

float expTest(float x) {
	const float ln2 = 0.6931471805599453;
	const float invln2 = 1.4426950408889634; 
	int k = int(round(x * invln2)); 
	const float r = (k * -ln2 + x); 

	float n = 1.0000000754895593 + r * (1.0000000647006064 + r * (0.4999886914692487 + r * (0.16666325650514743 + r * (0.041917526523052265 + r * (0.008381111717943628)))));

	//u32 nBits = std::bit_cast<u32>(n);
	//u32 exponent = nBits & F32_EXPONENT_MASK;
	//exponent >>= F32_EXPONENT_SHIFT;
	//const auto oldExponent = exponent;
	//exponent += k;
	//exponent &= 0xFF;
	//exponent = std::max(exponent, oldExponent);
	//exponent <<= F32_EXPONENT_SHIFT;
	//exponent &= F32_EXPONENT_MASK;
	//u32 nWithClearedExponent = nBits & ~F32_EXPONENT_MASK;
	//u32 nWithModifiedExponent = nWithClearedExponent | exponent;
	//return std::bit_cast<float>(nWithModifiedExponent);

	//k = 0xFF;
	auto exponent = k + 127;
	exponent = std::clamp(exponent, 0, 255);
	const auto twoToK = std::bit_cast<float>(exponent << F32_EXPONENT_SHIFT);
	const auto t1 = n * twoToK;
	const auto t2 = expSimd(_mm256_set1_ps(x)).m256_f32[0];
	return t2;
}

#ifdef __STDC__
static const double
#else
static double
#endif
ln2_hi = 6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
ln2_lo = 1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
two54 = 1.80143985094819840000e+16,  /* 43500000 00000000 */
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

static double zero = 0.0;

double __ieee754_log(double x)
{
	double hfsq, f, s, z, R, w, t1, t2, dk;
	int k, hx, i, j;
	unsigned lx;

	hx = __HI(x);		/* high word of x */
	lx = __LO(x);		/* low  word of x */

	k = 0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
		if (((hx & 0x7fffffff) | lx) == 0)
			return -two54 / zero;		/* log(+-0)=-inf */
		if (hx < 0) return (x - x) / zero;	/* log(-#) = NaN */
		k -= 54; x *= two54; /* subnormal number, scale up x */
		hx = __HI(x);		/* high word of x */
	}
	if (hx >= 0x7ff00000) return x + x;
	k += (hx >> 20) - 1023;
	hx &= 0x000fffff;
	i = (hx + 0x95f64) & 0x100000;
	__HI(x) = hx | (i ^ 0x3ff00000);	/* normalize x or x/2 */
	k += (i >> 20);
	f = x - 1.0;
	if ((0x000fffff & (2 + hx)) < 3) {	/* |f| < 2**-20 */
		if (f == zero) if (k == 0) return zero;  else {
			dk = (double)k;
			return dk * ln2_hi + dk * ln2_lo;
		}
		R = f * f * (0.5 - 0.33333333333333333 * f);
		if (k == 0) return f - R; else {
			dk = (double)k;
			return dk * ln2_hi - ((R - dk * ln2_lo) - f);
		}
	}
	return f;
	s = f / (2.0 + f);
	dk = (double)k;
	z = s * s;
	i = hx - 0x6147a;
	w = z * z;
	j = 0x6b851 - hx;
	t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
	t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
	i |= j;
	R = t2 + t1;
	if (i > 0) {
		hfsq = 0.5 * f * f;
		if (k == 0) return f - (hfsq - s * (hfsq + R)); else
			return dk * ln2_hi - ((hfsq - (s * (hfsq + R) + dk * ln2_lo)) - f);
	}
	else {
		if (k == 0) return f - s * (f - R); else
			return dk * ln2_hi - ((s * (f - R) - dk * ln2_lo) - f);
	}
}

double ramezPolynomial(double x) {
	const auto R1 = x - (P1 * pow(x, 2.0f) + P2 * pow(x, 4.0f) + P3 * pow(x, 6.0f) + P4 * pow(x, 8.0f) + P5 * pow(x, 10.0f));
	return 1 + x + (x * R1) / (2 - R1);
}

template<typename Function>
void plotFunction(Function f, Vec3 color = Color3::WHITE) {
	const auto plotRect = ImPlot::GetPlotLimits();
	std::vector<float> xs;
	std::vector<float> ys;
	const auto count = 400;
	for (int i = 0; i < count; i++) {
		float x = plotRect.X.Min + (plotRect.X.Max - plotRect.X.Min) * (i / float(count - 1));
		float y = f(x);
		xs.push_back(x);
		ys.push_back(y);
	}
	ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color.x, color.y, color.z, 1.0f));
	ImPlot::PlotLine("test", xs.data(), ys.data(), xs.size());
	ImPlot::PopStyleColor();
}

float sinTest(float x) {
	//u32 xSign = std::bit_cast<u32>(x) & ;
	const auto pi = 3.14159265359;
	float m = floor(x / pi);
	x -= m * pi;
	return x;
}

SpecialFunctionsDemo::SpecialFunctionsDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ln_test(-1.0f);
	//const auto res = maxDifference(expf, expTest, -20.0f, 20.0f, ErrorType::RELATIVE);
	const auto r1 = maxDifference(expf, expTest, -50.0f, 50.0f, ErrorType::RELATIVE);
	put("exp max error: % on %", r1.maxDifference, r1.maxDifferenceInput);
	const auto r2 = maxDifference(logf, ln_test, 0.01f, 100000.0f, ErrorType::RELATIVE);
	put("exp max error: % on %", r2.maxDifference, r2.maxDifferenceInput);
}

float cosa(float x) noexcept
{
	static constexpr float M_PI = 3.14159265359f;
	constexpr float tp = 1. / (2. * M_PI);
	x *= tp;
	x -= float(.25) + std::floor(x + float(.25));
	x *= float(16.) * (std::abs(x) - float(.5));
	x += float(.225) * x * (std::abs(x) - float(1.));
	return x;
}

void SpecialFunctionsDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	using namespace ImGui;

	auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);

	static bool firstFrame = true;
	if (firstFrame) {
		DockBuilderRemoveNode(id);
		DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		DockBuilderDockWindow("plot", rightId);
		DockBuilderDockWindow("plot settings", leftId);

		DockBuilderFinish(id);
		firstFrame = false;
	}

	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar);

	static int order = 5;

	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f))) {

		/*plotFunction(expf);
		plotFunction(__ieee754_exp);*/
		//plotFunction([&](float x) { return expf(x) - __ieee754_exp(x); });
		/*plotFunction(ramezPolynomial, Color3::RED);
		plotFunction(expf, Color3::BLUE);*/
		//plotFunction(expTest, Color3::RED);
		//plotFunction(expf, Color3::BLUE);
		//plotFunction(__ieee754_log, Color3::BLUE);
		/*plotFunction(ln_test, Color3::BLUE);
		plotFunction(logf, Color3::RED);*/
		//plotFunction(ln_test, Color3::BLUE);
		//plotFunction(cosa<float>, Color3::RED);
		plotFunction(cosf, Color3::BLUE);
		/*plotFunction(ln_test, Color3::RED);
		plotFunction(logf, Color3::BLUE);*/

		//plotFunction([&](float x) { return expf(x) - exp_reduce(x); }, Color3::RED);

		ImPlot::EndPlot();
	}

	End();

	Begin("plot settings");

	ImGui::InputInt("exp reduce order", &order);

	End();
}
