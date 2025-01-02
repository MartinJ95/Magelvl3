cmake_minimum_required(VERSION 3.10)

project(Mage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C++_STANDARD_REQUIRED True)

add_subdirectory(Utils)
add_subdirectory(Graphics)
add_subdirectory(Physics)
add_subdirectory(Network)
add_subdirectory(Core)
add_subdirectory(Sandbox)