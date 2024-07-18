cmake_minimum_required(VERSION 3.22.3)
project(CG-Project LANGUAGES CXX)

#########################################################
# Variables                                             #
# Set them to match your SDK and libraries installation #
#########################################################
list(APPEND LINK_LIBS "${GLFW_LIB}")
list(APPEND INCLUDE_DIRS "${GLFW_INCLUDE_DIR}" headers)

#########################################################
# CMake configuration                                   #
# You shouldn't need to modify anything past this point #
#########################################################
set(CMAKE_CXX_STANDARD 17)

add_executable(CG-Project main.cpp modules/Starter.hpp modules/TextMaker.hpp
        modules/SceneManager.hpp)

find_package(Vulkan REQUIRED)

foreach(dir IN LISTS Vulkan_INCLUDE_DIR INCLUDE_DIRS)
    target_include_directories(CG-Project PUBLIC ${dir})
endforeach()

foreach(lib IN LISTS Vulkan_LIBRARIES LINK_LIBS)
    target_link_libraries(CG-Project ${lib})
endforeach()