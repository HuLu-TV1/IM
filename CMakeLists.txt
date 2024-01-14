cmake_minimum_required(VERSION 3.11)
project(IM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UTILS "${PROJECT_SOURCE_DIR}/src/libotutil")

add_subdirectory(${UTILS})