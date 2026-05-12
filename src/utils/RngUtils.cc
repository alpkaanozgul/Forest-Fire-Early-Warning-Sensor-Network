#include "RngUtils.h"
#include <cmath>

// Helper: generate U ~ Uniform(0,1) using OMNeT++ 6.2 API.
// getContext() returns the module that is currently executing, so getRNG(k)
// picks the correct per-module RNG stream for independent replication.
static inline double U01(int rngIndex)
{
    omnetpp::cComponent *ctx = omnetpp::getSimulation()->getContext();
    double u = ctx->getRNG(rngIndex)->doubleRand();
    if (u <= 0.0) u = 1e-12;
    return u;
}

// ---------------------------------------------------------------
// Exponential(lambda)
// Inverse CDF: X = -1/lambda * ln(U)
// ---------------------------------------------------------------
double RngUtils::exponentialRV(double lambda, int rngIndex)
{
    return -1.0 / lambda * std::log(U01(rngIndex));
}

// ---------------------------------------------------------------
// Uniform(a, b)
// Y = a + (b - a) * U
// ---------------------------------------------------------------
double RngUtils::uniformRV(double a, double b, int rngIndex)
{
    return a + (b - a) * U01(rngIndex);
}

// ---------------------------------------------------------------
// Bernoulli(p)
// X = 1 if U <= p, else 0
// ---------------------------------------------------------------
int RngUtils::bernoulliRV(double p, int rngIndex)
{
    return (U01(rngIndex) <= p) ? 1 : 0;
}

// ---------------------------------------------------------------
// Geometric(p)
// Inverse CDF: X = ceil(ln(U) / ln(1-p))
// ---------------------------------------------------------------
int RngUtils::geometricRV(double p, int rngIndex)
{
    return (int)std::ceil(std::log(U01(rngIndex)) / std::log(1.0 - p));
}

// ---------------------------------------------------------------
// Binomial(n, p)
// Convolution: sum of n independent Bernoulli(p) variates
// ---------------------------------------------------------------
int RngUtils::binomialRV(int n, double p, int rngIndex)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += (U01(rngIndex) <= p) ? 1 : 0;
    return sum;
}

// ---------------------------------------------------------------
// Poisson(lambda)
// Convolution method: multiply uniforms until product < e^{-lambda}
// ---------------------------------------------------------------
int RngUtils::poissonRV(double lambda, int rngIndex)
{
    int N = 0;
    double product = 1.0;
    double threshold = std::exp(-lambda);
    while (product >= threshold) {
        product *= U01(rngIndex);
        N++;
    }
    return N - 1;
}

// ---------------------------------------------------------------
// Normal(mu, sigma)
// Box-Muller: Z0 = sqrt(-2*ln(U1)) * cos(2*pi*U2)
// ---------------------------------------------------------------
double RngUtils::normalRV(double mu, double sigma, int rngIndex)
{
    double U1 = U01(rngIndex);
    double U2 = U01(rngIndex);
    double Z0 = std::sqrt(-2.0 * std::log(U1)) * std::cos(2.0 * M_PI * U2);
    return mu + sigma * Z0;
}

// ---------------------------------------------------------------
// Discrete Uniform {1, 2, ..., W}
// X = floor(W * U) + 1
// ---------------------------------------------------------------
int RngUtils::discreteUniformRV(int W, int rngIndex)
{
    return (int)(W * U01(rngIndex)) + 1;
}
