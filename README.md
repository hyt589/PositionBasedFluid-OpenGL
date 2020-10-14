# Position Based Fluid

***

## Overview

This is an OpenGL project built to simulate fluid with position based dynamics.

## Usage

Clone this repo recursively with submodules as this project includes dependencies from other repositories.

```bash
git clone --recursive https://github.com/hyt589/PositionBasedFluid-OpenGL.git
```

This project is built with CMake:

```sh
cd PositionBasedFluid-OpenGL
mkdir build
cd build
cmake ../.
make -j8 #make with 8 parallel jobs
```

Run with `config.json`:

```sh
cd PositionBasedFluid-OpenGL
./build/pbf
```

Or run with your own configuration and assets:
```sh
./pbf <path to config json file>
```

## Notes

This project is still a work in progress.

### TODOS

- Implement PBR shaders for rendering
- Implement position based fluid
- (stretch goal) Implement global illumination using Voxel Cone Tracing