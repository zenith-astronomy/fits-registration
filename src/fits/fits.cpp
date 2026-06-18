#include "fits.h"

#include <bit>
#include <cstdint>
#include <stdexcept>

namespace
{
    bool TrimString(std::string& string);

    int GetInt(const std::string& line);

    float GetFloat(const std::string& line);

    unsigned char ReadByte(Path& file);

    std::uint16_t ReadU16BE(Path& file);
    std::uint32_t ReadU32BE(Path& file);
}

bool ParseFitsHeader(Fits& fits, Path& file)
{
    std::size_t headerBytes = 0;

    while (true)
    {
        char card[80];

        file.read(card, 80);

        if (!file)
        {
            throw std::runtime_error("Unexpected end of file while parsing FITS header");
            return false;
        }

        headerBytes += 80;

        std::string line(card, 80);

        std::string key = line.substr(0, 8);

        if (!TrimString(key))
        {
            continue;
        }

        if (key == "END")
        {
            break;
        }
        else if (key == "BITPIX")
        {
            fits.bitpix = GetInt(line);
        }
        else if (key == "NAXIS")
        {
            fits.naxis = GetInt(line);
        }
        else if (key == "NAXIS1")
        {
            fits.width = GetInt(line);
        }
        else if (key == "NAXIS2")
        {
            fits.height = GetInt(line);
        }
        else if (key == "BSCALE")
        {
            fits.bscale = GetFloat(line);
        }
        else if (key == "BZERO")
        {
            fits.bzero = GetFloat(line);
        }
    }

    std::size_t padding = (2880 - (headerBytes % 2880)) % 2880;

    file.seekg(static_cast<std::streamoff>(padding), std::ios::cur);

    return true;
}

bool ParseFitsPixels(Fits& fits, Path& file)
{
    const std::size_t pixelCount = static_cast<size_t>(fits.width * fits.height);

    Pixels pixels = {};
    pixels.reserve(pixelCount);

    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        float value = 0.0f;

        switch (fits.bitpix)
        {
            case 8:
            {
                value = static_cast<float>(ReadByte(file));
                break;
            }

            case 16:
            {
                std::uint16_t raw = ReadU16BE(file);
                std::int16_t signedValue = std::bit_cast<std::int16_t>(raw);

                value = static_cast<float>(signedValue);
                break;
            }

            case 32:
            {
                std::uint32_t raw = ReadU32BE(file);
                std::int32_t signedValue = std::bit_cast<std::int32_t>(raw);

                value = static_cast<float>(signedValue);
                break;
            }

            case -32:
            {
                std::uint32_t raw = ReadU32BE(file);
                value = std::bit_cast<float>(raw);
                break;
            }

            default:
            {
                throw std::runtime_error("Unsupported BITPIX value");
                return false;
            }
        }

        value = fits.bzero + fits.bscale * value;

        pixels.push_back(value);
    }

    fits.pixels = pixels;

    return true;
}

namespace
{
    bool TrimString(std::string& string)
    {
        std::size_t first = string.find_first_not_of(' ');
        std::size_t last = string.find_last_not_of(' ');

        if (first == std::string::npos)
        {
            return false;
        }

        string = string.substr(first, last - first + 1);

        return true;
    }

    int GetInt(const std::string& line)
    {
        if (line.size() >= 10 && line[8] == '=')
        {
            std::string value = line.substr(10);

            std::size_t commentPosition = value.find('/');

            if (commentPosition != std::string::npos)
            {
                value = value.substr(0, commentPosition);
            }

            if (TrimString(value))
            {
                return std::stoi(value);
            }
        }

        return 0;
    }

    float GetFloat(const std::string& line)
    {
        if (line.size() >= 10 && line[8] == '=')
        {
            std::string value = line.substr(10);

            std::size_t commentPosition = value.find('/');

            if (commentPosition != std::string::npos)
            {
                value = value.substr(0, commentPosition);
            }

            if (TrimString(value))
            {
                return std::stof(value);
            }
        }

        return 0.0f;
    }

    unsigned char ReadByte(Path& file)
    {
        char byte = 0;

        file.read(&byte, 1);

        if (!file)
        {
            throw std::runtime_error("Unexpected end of file while parsing FITS pixels");
        }

        return static_cast<unsigned char>(byte);
    }

    std::uint16_t ReadU16BE(Path& file)
    {
        std::uint16_t b0 = ReadByte(file);
        std::uint16_t b1 = ReadByte(file);

        return static_cast<std::uint16_t>((b0 << 8) | b1);
    }

    std::uint32_t ReadU32BE(Path& file)
    {
        std::uint32_t b0 = ReadByte(file);
        std::uint32_t b1 = ReadByte(file);
        std::uint32_t b2 = ReadByte(file);
        std::uint32_t b3 = ReadByte(file);

        return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
    }
}
