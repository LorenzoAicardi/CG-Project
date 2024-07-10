# CG-Project

Computer graphics project developed for the Computer Graphics course 2023/2024. 

**Contributors**: \
Lorenzo Aicardi _lorenzo.aicardi@mail.polimi.it_ \
Isaia Belardinelli _isaia.belardinelli@mail.polimi.it_ \
Nicolas Benatti _nicolas.benatti@mail.polimi.it_

## Building & Running

The project uses the CMake build system, Vulkan installation is detected by default while GLFW include directories and library location has to be specified via `GLFW_DIR` and `GLFW_LIB`, respectively. 
Also, [Ninja](https://ninja-build.org/) is recommended for better build performance.

```bash
$ mkdir -p && cd build

$ cmake \
  -DGLFW_DIR="/usr/include/GLFW" \
  -DGLFW_LIB="/usr/lib64/libglfw.so" \
  -DCMAKE_BUILD_TYPE=[Debug | Release] \
  -G Ninja
  ..

$ cmake --build .
```

### Integration with IDEs

#### CLion

Integrating with CLion is pretty easy thanks to CMake, just clone the repo and setup build parameters in `"Settings" > "Build, Execution, Deployment" > "CMake"`