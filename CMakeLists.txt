cmake_minimum_required(VERSION 3.16)
project(CompilerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All include headers
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

add_executable(compiler_project ${SOURCES})
