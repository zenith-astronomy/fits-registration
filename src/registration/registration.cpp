#include "registration.h"

#include <iostream>
#include <algorithm>

namespace
{
    float Abs(float value)
    {
        return value >= 0 ? value : -value;
    }
}

float EstimateBackground(const Fits& fits)
{
    std::cout << "Estimating background median...\n";

    int pixelsCount = fits.pixels.size();

    std::sort(fits.pixels.begin(), fits.pixels.end());

    float median = 0.0f;

    if (pixelsCount % 2 == 1)
    {
        median = fits.pixels[pixelsCount / 2];
    }
    else
    {
        float lower = fits.pixels[(pixelsCount / 2) - 1];
        float upper = fits.pixels[pixelsCount / 2];

        median = (lower + upper) / 2.0f;
    }

    return median;
}

float EstimateMAD(const Fits& fits)
{
    float background = EstimateBackground(fits);

    int pixelsCount = fits.pixels.size();

    Pixels values;
    values.reserve(pixelsCount);
    values.clear();

    for (int i = 0; i < pixelsCount; i++)
    {
        values.push_back(Abs(fits.pixels[i] - background));
    }

    std::sort(values.begin(), values.end());

    float MAD = 0.0f;

    if (pixelsCount % 2 == 1)
    {
        MAD = fits.pixels[pixelsCount / 2];
    }
    else
    {
        float lower = fits.pixels[(pixelsCount / 2) - 1];
        float upper = fits.pixels[pixelsCount / 2];

        MAD = (lower + upper) / 2.0f;
    }

    return MAD;
}