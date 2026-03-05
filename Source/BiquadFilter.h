/*
  ==============================================================================

    BiquadFilter.h
    Created: 28 Feb 2026 8:46:31am
    Author:  doare

  ==============================================================================
*/

#pragma once
#include <cmath>
#include <algorithm>

class BiquadFilter
{
public:
    BiquadFilter() = default;

    // Calculate coefficients for a Low Pass filter
    void setLowPass(double sampleRate, double cutoffFreq, double Q)
    {
        // Constrain cutoff to be below Nyquist
        cutoffFreq = std::max(20.0, std::min(cutoffFreq, sampleRate * 0.49));
        Q = std::max(0.1, Q);

        double w0 = 2.0 * M_PI * cutoffFreq / sampleRate;
        double cosW0 = std::cos(w0);
        double alpha = std::sin(w0) / (2.0 * Q);

        double a0 = 1.0 + alpha;
        
        // Normalize coefficients by a0
        b0 = ((1.0 - cosW0) / 2.0) / a0;
        b1 = (1.0 - cosW0) / a0;
        b2 = ((1.0 - cosW0) / 2.0) / a0;
        a1 = (-2.0 * cosW0) / a0;
        a2 = (1.0 - alpha) / a0;
    }

     // Calculate coefficients for a Band Pass filter
    void setBandPass(double sampleRate, double centerFreq, double bandwidth)
    {
        double w0 = 2.0 * M_PI * centerFreq / sampleRate;
        double alpha = std::sin(w0) / (2.0 * (centerFreq / bandwidth));

        double a0 = 1.0 + alpha;

        // Normalize coefficients by a0
        b0 = (alpha) / a0;
        b1 = 0.0;
        b2 = (-alpha) / a0;
        a1 = (-2.0 * std::cos(w0)) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    void reset()
    {
        z1 = 0.0;
        z2 = 0.0;
    }

    float process(float input)
    {
        // Direct Form I
        double output = b0 * input + b1 * x1 + b2 * x2 - a1 * z1 - a2 * z2;
        
        // Update state
        x2 = x1;
        x1 = input;
        z2 = z1;
        z1 = output;

        return static_cast<float>(output);
    }

private:
    double b0 = 0.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
    double x1 = 0.0, x2 = 0.0; // Input history
    double z1 = 0.0, z2 = 0.0; // Output history
};
