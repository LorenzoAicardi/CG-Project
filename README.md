# Toy Rocket

Computer graphics project developed for the Computer Graphics course 2023/2024.

**Contributors**: \
Lorenzo Aicardi _lorenzo.aicardi@mail.polimi.it_ \
Isaia Belardinelli _isaia.belardinelli@mail.polimi.it_ \
Nicolas Benatti _nicolas.benatti@mail.polimi.it_

## The project

Our project is a middle-ground between a showcase of rudimentary Physically-Based
Rendering (PBR) techniques and a minimal game: you will be driving a little toy rocket inside
a bedroom with some floating coins which you have to collect.

Following this minimal approach, there is no score, making it kind of a "zen" experience 😉

<img src="res/showreel.gif">

## Technical features

Here are some of the techniques used

* *Cook-Torrance* BRDF
* Full BSDF for lateral window and roof lamp
* Movable direct lighting through Z, X keys
* Point and Spot lights
* "Smooth" rocket steering
* Static elements of the scene can be placed via JSON file

## Building & Running

This project is built on Vulkan, so please refer to tutorials
like [this](https://vulkan-tutorial.com/Development_environment)
in order to properly setup your environment.

As of build systems, we opted for Cmake: your Vulkan installation should be detected by default
while GLFW include directories and library location have to be specified via `GLFW_INCLUDE_DIR`
and `GLFW_LIB`, respectively.

[Ninja](https://ninja-build.org/) is also recommended for better build performance.

```bash
$ mkdir -p && cd build

$ cmake \
  -DGLFW_INCLUDE_DIR="/usr/include/GLFW" \
  -DGLFW_LIB="/usr/lib64/libglfw.so" \
  -DCMAKE_BUILD_TYPE=[Debug | Release] \
  -G Ninja
  ..

$ cmake --build .
```

### Integration with IDEs

#### CLion

Integrating with CLion is pretty easy thanks to CMake, just clone the repo and setup build parameters
in `"Settings" > "Build, Execution, Deployment" > "CMake"`
