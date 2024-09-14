cmake_minimum_required(VERSION 3.5)
project(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY "https://github.com/doctest/doctest"
  GIT_TAG "v2.4.11")
FetchContent_MakeAvailable(doctest)

include("../cmake/pedantic.cmake")
include("../cmake/sanitizer.cmake")

add_executable(test main.test.cpp)
target_compile_features(test PRIVATE cxx_std_20)
target_link_libraries(test PRIVATE sanitizer::address sanitizer::undefined
                                   pedantic::pedantic doctest::doctest)

add_subdirectory("../sso" "${CMAKE_BINARY_DIR}/sso")
target_link_libraries(test PRIVATE sso::sso)
