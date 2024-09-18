cmake_minimum_required(VERSION 3.5)
project(
  sso
  DESCRIPTION "sso - small string optimization"
  LANGUAGES CXX)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/")
add_library(sso INTERFACE)
target_sources(
  sso INTERFACE "${INCLUDE_DIR}/sso/string.hpp"
                "${INCLUDE_DIR}/sso/detail/basic_string_buffer.hpp")
target_compile_features(sso INTERFACE cxx_std_20)
target_include_directories(sso INTERFACE "${INCLUDE_DIR}")

add_library(sso::sso ALIAS sso)
