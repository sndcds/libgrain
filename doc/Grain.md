# GrainLib

Last edit: 19.07.2025

## Build Library

```
rm -rf build && mkdir build

cmake -S . -B build
cmake --build build
sudo cmake --install build --prefix /usr/local
```


## Build and Run Demos

Configure, Compile and Execute:
```sh
cmake -S . -B build
cmake --build build
./build/grain_demo  
```


## Count Code Lines

cloc counts code, comments, and blank lines separately.

```bash
cloc .
```


## External libraries

- nlohmann/json
- ssh
- webp
- png
- ${JPEG_LIBRARY}
- raw
- tiff
- proj
- sndfile


## Includes Standards

- JSON via nlohmann/json
- TOML
- XML via tinyxml2
