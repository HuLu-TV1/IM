cmake_minimum_required(VERSION 3.11)
project(IM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UTILS "${PROJECT_SOURCE_DIR}/src/libotutil")
set(NETS "${PROJECT_SOURCE_DIR}/net_server")

add_subdirectory(${UTILS})
add_subdirectory(${NETS})