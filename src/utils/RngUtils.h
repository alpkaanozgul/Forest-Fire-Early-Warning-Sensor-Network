#ifndef RNGUTILS_H
#define RNGUTILS_H

//
// RngUtils.h
// All random variate generation is done here using the inverse transform
// method, convolution method, and Box-Muller transform as taught in CNG-476.
// We deliberately implement these manually rather than using omnetpp's
// built-in helpers, to demonstrate understanding of the course material.
//
// Every function takes an rng index so each module can use its own
// independent RNG stream (OMNeT++ best practice).
//

#include <omnetpp.h>

using namespace omnetpp;

class RngUtils
{
public:
    // ---------------------------------------------------------------
    // Exponential(lambda)  -- inverse transform: X = -1/lambda * ln(U)
    // Used for: fire inter-arrival times, telemetry intervals, service times
    // ---------------------------------------------------------------
    static double exponentialRV(double lambda, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Uniform(a, b)  -- Y = a + (b-a)*U
    // Used for: collision back-off intervals, sensor placement jitter
    // ---------------------------------------------------------------
    static double uniformRV(double a, double b, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Bernoulli(p)  -- X = 1 if U <= p, else 0
    // Used for: fire detection (pd), false alarm, packet error
    // ---------------------------------------------------------------
    static int bernoulliRV(double p, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Geometric(p)  -- number of trials until first success
    // X = ceil(ln(U) / ln(1-p))
    // Used for: number of retransmission attempts
    // ---------------------------------------------------------------
    static int geometricRV(double p, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Binomial(n, p)  -- convolution of n Bernoulli(p) variates
    // Used for: number of sensors detecting a fire out of n
    // ---------------------------------------------------------------
    static int binomialRV(int n, double p, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Poisson(lambda)  -- convolution method from lecture:
    // count exponential inter-arrivals until sum exceeds 1
    // Used for: number of fire events in a fixed window
    // ---------------------------------------------------------------
    static int poissonRV(double lambda, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Normal(mu, sigma)  -- Box-Muller transform from lecture:
    // Z0 = sqrt(-2*ln(U1)) * cos(2*pi*U2)
    // Used for: temperature readings, humidity readings, sensor noise
    // ---------------------------------------------------------------
    static double normalRV(double mu, double sigma, int rngIndex = 0);

    // ---------------------------------------------------------------
    // Discrete Uniform {1, 2, ..., W}  -- back-off slot selection
    // ---------------------------------------------------------------
    static int discreteUniformRV(int W, int rngIndex = 0);
};

#endif
