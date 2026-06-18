#pragma once

#include <string>
#include <vector>
#include <fstream>

using Pixels = std::vector<float>;

using Path = std::ifstream;

struct Fits
{
    int bitpix;
    int naxis;
    int height;
    int width;

    float bscale = 1.0f;
    float bzero = 0.0f;

    Pixels pixels;
};

bool ParseFitsHeader(Fits& fits, Path& file);

bool ParseFitsPixels(Fits& fits, Path& file);