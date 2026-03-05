#pragma once

//
// Box-Muller transform for Gaussian distribution
// https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
//

#include <cmath>
#include <random>

class RandomNormalGenerator
{
public:
    RandomNormalGenerator(uint32_t seed = 0)
        : rng(seed == 0 ? std::random_device{}() : seed),
          runif(0.0, 1.0),
          hasSpare(false),
          spare(0.0)
    {
    }

    void setSeed(uint32_t seed)
    {
        rng.seed(seed);
        hasSpare = false;
        spare = 0.0;
    }

    // Generate a random float between 0 and 1
    float uniform()
    {
        return static_cast<float>(runif(rng));
    }

    // Generate a normal distributed random value
    // mu: mean, sigma: standard deviation
    double normal(double mu = 0.0, double sigma = 1.0)
    {
        constexpr double two_pi = 2.0 * M_PI;

        if (hasSpare)
        {
            hasSpare = false;
            return spare * sigma + mu;
        }

        hasSpare = true;
        double u1, u2;
        do
        {
            u1 = runif(rng);
        } while (u1 <= 0.0);  // Make sure u1 is greater than zero
        u2 = runif(rng);

        double mag = sqrt(-2.0 * log(u1));
        spare = mag * sin(two_pi * u2);
        return mag * cos(two_pi * u2) * sigma + mu;
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> runif;
    bool hasSpare;
    double spare;
};
