# fits-registration

CLI tool for registering FITS sub-exposures (WIP)

## Performance

I'll try to keep track of performance improvements between different algorithm versions here.

## Build

With compiler optimizations:

```bash
g++ src/main.cpp src/fits/fits.cpp src/registration/registration.cpp -O3 -march=native -std=c++20 -o stacker.exe
```