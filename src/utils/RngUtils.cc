#include "RngUtils.h"
#include <cmath>

//
// RngUtils.cc
// Implements all random variate generators using methods taught in CNG-476.
// All generators start from U ~ Uniform(0,1) obtained via OMNeT++'s
// uniform(0, 1, rngIndex) which uses a separate LCG stream per index.
//

// ---------------------------------------------------------------
// Exponential(lambda)
// Inverse CDF: F(x) = 1 - e^{-lambda*x}  =>  X = -1/lambda * ln(U)
// Note: we use ln(U) not ln(1-U) since 1-U is also Uniform(0,1)
// ---------------------------------------------------------------
double RngUtils::exponentialRV(double lambda, int rngIndex)
{
    double U = uniform(0, 1, rngIndex);
    // guard against U=0 (LCG never produces exactly 0, but be safe)
    if (U <= 0.0) U = 1e-12;
    return -1.0 / lambda * std::log(U);
}

// ---------------------------------------------------------------
// Uniform(a, b)
// Y = a + (b - a) * U
// ---------------------------------------------------------------
double RngUtils::uniformRV(double a, double b, int rngIndex)
{
    double U = uniform(0, 1, rngIndex);
    return a + (b - a) * U;
}

// ---------------------------------------------------------------
// Bernoulli(p)
// Inverse transform for discrete distribution:
// X = 1 if U <= p,  X = 0 if U > p
// ---------------------------------------------------------------
int RngUtils::bernoulliRV(double p, int rngIndex)
{
    double U = uniform(0, 1, rngIndex);
    return (U <= p) ? 1 : 0;
}

// ---------------------------------------------------------------
// Geometric(p)
// Number of trials until first success (including the success).
// Inverse CDF: X = ceil(ln(U) / ln(1-p))
// Simplified: X = floor(ln(U) / ln(1-p)) + 1
// ---------------------------------------------------------------
int RngUtils::geometricRV(double p, int rngIndex)
{
    double U = uniform(0, 1, rngIndex);
    if (U <= 0.0) U = 1e-12;
    return (int)std::ceil(std::log(U) / std::log(1.0 - p));
}

// ---------------------------------------------------------------
// Binomial(n, p)
// Convolution method: sum n independent Bernoulli(p) variates
// X = Y1 + Y2 + ... + Yn,  Yi ~ Bernoulli(p)
// ---------------------------------------------------------------
int RngUtils::binomialRV(int n, double p, int rngIndex)
{
    int sum = 0;
    for (int i = 0; i < n; i++) {
        double U = uniform(0, 1, rngIndex);
        sum += (U <= p) ? 1 : 0;
    }
    return sum;
}

// ---------------------------------------------------------------
// Poisson(lambda)
// Convolution method from lecture:
// Generate exponential inter-arrivals Ei = -1/lambda * ln(Ui)
// Count N such that product of Ui falls below e^{-lambda}
// Equivalent: count arrivals until cumulative sum of Ei exceeds 1
// ---------------------------------------------------------------
int RngUtils::poissonRV(double lambda, int rngIndex)
{
    int N = 0;
    double product = 1.0;
    double threshold = std::exp(-lambda);
    while (product >= threshold) {
        double U = uniform(0, 1, rngIndex);
        if (U <= 0.0) U = 1e-12;
        product *= U;
        N++;
    }
    return N - 1;
}

// ---------------------------------------------------------------
// Normal(mu, sigma)
// Box-Muller transform from lecture:
// Z0 = sqrt(-2 * ln(U1)) * cos(2*pi*U2)
// Z1 = sqrt(-2 * ln(U1)) * sin(2*pi*U2)
// We return Z0 and discard Z1 for simplicity.
// Then X = mu + sigma * Z0
// ---------------------------------------------------------------
double RngUtils::normalRV(double mu, double sigma, int rngIndex)
{
    double U1 = uniform(0, 1, rngIndex);
    double U2 = uniform(0, 1, rngIndex);
    if (U1 <= 0.0) U1 = 1e-12;
    double Z0 = std::sqrt(-2.0 * std::log(U1)) * std::cos(2.0 * M_PI * U2);
    return mu + sigma * Z0;
}

// ---------------------------------------------------------------
// Discrete Uniform {1, 2, ..., W}
// X = floor(W * U) + 1
// ---------------------------------------------------------------
int RngUtils::discreteUniformRV(int W, int rngIndex)
{
    double U = uniform(0, 1, rngIndex);
    return (int)(W * U) + 1;
}
