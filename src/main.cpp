#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "fits/fits.h"
#include "fits/dir.h"

#include "algorithms/average.h"
#include "algorithms/median.h"
#include "algorithms/min-max.h"

Fits LoadFits(const fs::path& path)
{
    Fits fits;

    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Could not open FITS file");
    }

    if (!ParseFitsHeader(fits, file))
    {
        std::cerr << "Couldn't parse FITS header\n";
    }

    if (!ParseFitsPixels(fits, file))
    {
        std::cerr << "Couldn't parse FITS pixels\n";
    }

    return fits;
}

int main()
{
    std::string input;
    std::string command;

    while (std::getline(std::cin, input))
    {
        command.clear();

        std::istringstream stream(input);
        stream >> command;

        if(command == "stack" ||
           command == "integrate" ||
           command == "s")
        {
            auto start = std::chrono::steady_clock::now();

            std::string algorithm;

            if (!(stream >> algorithm))
            {
                std::cerr << "Usage: stack <algorithm> <directory path>\n";
                continue;
            }

            std::string dirPath;
            std::getline(stream >> std::ws, dirPath);

            if (dirPath.empty())
            {
                std::cerr << "Usage: stack <algorithm> <directory path>\n";
                continue;
            }

            if (dirPath.size() >= 2 && dirPath.front() == '"' && dirPath.back() == '"')
            {
                dirPath = dirPath.substr(1, dirPath.size() - 2);
            }

            std::vector<fs::path> paths = GetFitsInDir(dirPath);

            std::vector<Fits> frames = {};
            frames.resize(paths.size());

            int threadCount = std::thread::hardware_concurrency();

            threadCount = std::min<int>(threadCount, paths.size());

            std::cout << "Using " << threadCount << " threads\n";

            std::vector<std::thread> threads;
            threads.reserve(threadCount);

            std::size_t filesPerThread = (paths.size() + threadCount - 1) / threadCount;

            auto loadRange = [&](std::size_t begin, std::size_t end)
            {
                for (std::size_t i = begin; i < end; i++)
                {
                    frames[i] = LoadFits(paths[i]);
                }
            };

            for (unsigned int threadIndex = 0; threadIndex < threadCount; threadIndex++)
            {
                std::size_t begin = threadIndex * filesPerThread;
                std::size_t end = std::min(begin + filesPerThread, paths.size());

                threads.emplace_back(loadRange, begin, end);
            }

            for (std::thread& thread : threads)
            {
                thread.join();
            }

            auto end = std::chrono::steady_clock::now();

            double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000);

            std::cout << "Parsed " << frames.size() << " FITS files in " << elapsed << "ms\n";

            start = std::chrono::steady_clock::now();

            if (algorithm == "average")
            {
                Fits result = IntegrateAverage(frames);
                WriteToDir(result, dirPath);
            }
            else if (algorithm == "median")
            {
                Fits result = IntegrateMedian(frames);
                WriteToDir(result, dirPath);
            }
            else if (algorithm == "maximum")
            {
                Fits result = IntegrateMaximum(frames);
                WriteToDir(result, dirPath);
            }
            else if (algorithm == "minimum")
            {
                Fits result = IntegrateMinimum(frames);
                WriteToDir(result, dirPath);
            }
            else
            {
                std::cerr << "The selected integration algorithm is not available\n";
                continue;
            }

            end = std::chrono::steady_clock::now();

            elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000);

            std::cout << "Stacked " << frames.size() << " FITS files in " << elapsed << "ms\n";
        }
        else if (command == "quit" ||
                 command == "exit" ||
                 command == "q")
        {
            break;
        }
    }
}
