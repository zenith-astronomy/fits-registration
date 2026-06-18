#pragma once

#include "fits.h"

#include <string>
#include <vector>

#include <filesystem>

namespace fs = std::filesystem;

std::vector<fs::path> GetFitsInDir(const fs::path& dir);

void WriteToDir(const Fits& fits, const fs::path& dir);
