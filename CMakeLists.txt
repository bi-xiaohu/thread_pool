cmake_minimum_required(VERSION 3.25)

project(my_thread_pool
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Header-only library
add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Example
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE thread_pool)

# Tests
add_executable(test_thread_pool tests/test_thread_pool.cpp)
target_link_libraries(test_thread_pool PRIVATE thread_pool)

# Platform-specific threading
find_package(Threads REQUIRED)
target_link_libraries(basic_usage PRIVATE Threads::Threads)
target_link_libraries(test_thread_pool PRIVATE Threads::Threads)