#include "registration.h"

#include <iostream>
#include <algorithm>

float Median(Pixels& values)
{
    int numValues = values.size();

    std::sort(values.begin(), values.end());

    float median = 0.0f;

    if (numValues % 2 == 1)
    {
        median = values[numValues / 2];
    }
    else
    {
        float lower = values[(numValues / 2) - 1];
        float upper = values[numValues / 2];

        median = (lower + upper) / 2.0f;
    }

    return median;
}

float Abs(float value)
{
    return value >= 0 ? value : -value;
}

float MAD(Pixels values)
{
    float median = Median(values);

    int numValues = values.size();

    Pixels distances;
    distances.reserve(numValues);
    distances.clear();

    for (int i = 0; i < numValues; i++)
    {
        distances.push_back(Abs(values[i] - median));
    }

    std::sort(distances.begin(), distances.end());

    float MAD = Median(distances);

    return MAD;
}