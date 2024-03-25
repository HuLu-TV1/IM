#需要在根目录下添加，表示开启项目的测试功能
enable_testing()

include_directories(../src/include ../../src/libotutil/include/common)
add_executable(server_unittest server_unittest.cpp)
target_link_libraries(server_unittest net_demo )
#################


add_executable(thread_pool_unittest thread_pool_unittest.cpp)
target_link_libraries(thread_pool_unittest net_demo )

#################

add_executable(server_echo_unittest server_echo_unittest.cpp)
target_link_libraries(server_echo_unittest net_demo )
#################

# add_executable(client_unittest client_unittest.cpp)
# target_link_libraries(client_unittest net_demo )

if(BOOSTTEST_LIBRARY)
add_executable(buffer_unittest buffer_unittest.cpp)
# target_link_libraries(boost_unit_test_framework)
target_link_libraries(buffer_unittest  PUBLIC net_demo  ${CMAKE_SOURCE_DIR}/thirdfile/lib/libboost_unit_test_framework.so)
add_test(NAME buffer_unittest COMMAND buffer_unittest)
endif()
#################

add_executable(coroutine_test coroutine_test.cpp)
target_link_libraries(coroutine_test PUBLIC ${CMAKE_SOURCE_DIR}/third_part/bin/libotutil.so pthread)

add_executable(connect_pool_test connect_pool_unittest.cpp)
target_link_libraries(connect_pool_test PUBLIC ${CMAKE_SOURCE_DIR}/third_part/bin/libotutil.so pthread)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(otutil PUBLIC NLOG)
endif()