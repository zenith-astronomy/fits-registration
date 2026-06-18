#include "dir.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
    std::string ToLower(std::string string);

    bool IsFits(const fs::path& path);

    void WriteCard(std::ofstream& file, std::size_t& bytesWritten, std::string card);

    std::string IntCard(const std::string& key, int value);

    void WriteFloatBE(std::ofstream& file, float value);
}

std::vector<fs::path> GetFitsInDir(const fs::path& dir)
{
    if (!fs::exists(dir))
    {
        throw std::runtime_error("The specified directory does not exist");
    }

    if (!fs::is_directory(dir))
    {
        throw std::runtime_error("The specified path is not a directory");
    }

    std::vector<fs::path> paths;

    for (const fs::directory_entry& entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file() && IsFits(entry.path()))
        {
            paths.push_back(entry.path());
        }
    }

    return paths;
}

void WriteToDir(const Fits& fits, const fs::path& dir)
{
    fs::path outputDir = dir / "output";
    fs::create_directories(outputDir);

    fs::path outputPath = outputDir / "result.fits";

    std::ofstream file(outputPath, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Could not create output FITS file");
    }

    std::size_t headerBytes = 0;

    WriteCard(file, headerBytes, "SIMPLE  =                    T");
    WriteCard(file, headerBytes, "BITPIX  =                  -32");
    WriteCard(file, headerBytes, "NAXIS   =                    2");
    WriteCard(file, headerBytes, IntCard("NAXIS1", fits.width));
    WriteCard(file, headerBytes, IntCard("NAXIS2", fits.height));
    WriteCard(file, headerBytes, "END");

    std::size_t headerPadding = (2880 - (headerBytes % 2880)) % 2880;
    std::string spaces(headerPadding, ' ');
    file.write(spaces.data(), spaces.size());

    for (float pixel : fits.pixels)
    {
        WriteFloatBE(file, pixel);
    }

    std::size_t pixelBytes = fits.pixels.size() * sizeof(float);
    std::size_t pixelPadding = (2880 - (pixelBytes % 2880)) % 2880;
    std::string zeroes(pixelPadding, '\0');
    file.write(zeroes.data(), zeroes.size());
}

namespace
{
    std::string ToLower(std::string string)
    {
        std::transform(string.begin(), string.end(), string.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });

        return string;
    }

    bool IsFits(const fs::path& path)
    {
        std::string extension = ToLower(path.extension().string());

        return extension == ".fit" ||
               extension == ".fits" ||
               extension == ".fts";
    }

    void WriteCard(std::ofstream& file, std::size_t& bytesWritten, std::string card)
    {
        card.resize(80, ' ');

        file.write(card.data(), 80);

        bytesWritten += 80;
    }

    std::string IntCard(const std::string& key, int value)
    {
        std::ostringstream stream;

        stream << std::left << std::setw(8) << key << "= "
               << std::right << std::setw(20) << value;

        return stream.str();
    }

    void WriteU32BE(std::ofstream& file, std::uint32_t value)
    {
        unsigned char bytes[4] =
        {
            static_cast<unsigned char>((value >> 24) & 0xff),
            static_cast<unsigned char>((value >> 16) & 0xff),
            static_cast<unsigned char>((value >> 8) & 0xff),
            static_cast<unsigned char>(value & 0xff)
        };

        file.write(reinterpret_cast<const char*>(bytes), 4);
    }

    void WriteFloatBE(std::ofstream& file, float value)
    {
        std::uint32_t raw = std::bit_cast<std::uint32_t>(value);
        WriteU32BE(file, raw);
    }
}
