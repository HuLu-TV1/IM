set(COMMON_INC "include/common")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
"src/common/cfile.cpp"
"src/common/fileManager.cpp" 
"src/common/log.cpp"
"src/common/ThreadPool.cpp"
"src/common/arena.cpp"
"src/common/coroutine_iml.cpp"
"src/common/Connect_Pool.cpp"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/third_part/bin)

add_library(otutil SHARED ${SRC})
target_include_directories(otutil PUBLIC ${COMMON_INC})
target_link_options(otutil PRIVATE -w -g -fstack-protector -fPIC)
target_link_libraries(otutil PRIVATE pthread mysqlclient)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(otutil PUBLIC NLOG)
endif()