#pragma once

#include "BiquadFilter.h"
#include <vector>

class FormantFilter
{
public:
    FormantFilter() = default;

    void setFormant(int formantIndex, double formantFrequency, double formantBandwidth, double sampleRate)
    {
        if (formantIndex < formantFilters.size())
        {
            formantFilters[formantIndex].setBandPass(sampleRate, formantFrequency, formantBandwidth);
        }
    }

    void prepare(int numFormants)
    {
        formantFilters.resize(numFormants);
    }

    void reset()
    {
        for (auto& filter : formantFilters)
            filter.reset();
    }

    float process(float input)
    {
        float output = 0.0f;
        for (auto& filter : formantFilters)
        {
            output += filter.process(input);
        }
        return output;
    }

private:
    std::vector<BiquadFilter> formantFilters;
};
