# fits-integration

CLI tool for integrating FITS sub-exposures.

Note: this tool exclusively integrates frames, it does NOT register/align them.

## Available algorithms

- Average
- Median
- Minimum/Maximum

## Planned rejections

- Min/Max
- (Winsorized) Sigma clipping

## Performance

I'll try to keep track of performance improvements between different algorithm versions here.

## Build

With compiler optimizations:

```bash
g++ src/main.cpp src/fits/fits.cpp src/fits/dir.cpp src/algorithms/average.cpp src/algorithms/median.cpp src/algorithms/min-max.cpp -O3 -march=native -std=c++20 -o stacker.exe
```