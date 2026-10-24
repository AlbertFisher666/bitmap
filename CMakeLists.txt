cmake_minimum_required(VERSION 3.20)
project(bitmap VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(bitmap_lib STATIC
    src/bitmap.cpp
)
target_include_directories(bitmap_lib PUBLIC include)
target_compile_options(bitmap_lib PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
)

add_executable(bitmap_test src/bitmap_test.cpp)
target_link_libraries(bitmap_test PRIVATE bitmap_lib)
target_compile_options(bitmap_test PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
    -UNDEBUG
)

enable_testing()
add_test(NAME bitmap_test COMMAND bitmap_test)

add_executable(bitmap_demo src/bitmap_demo.cpp)
target_link_libraries(bitmap_demo PRIVATE bitmap_lib)
target_compile_options(bitmap_demo PRIVATE
	-Wall -Wextra -Wpedantic -Wshadow
)
