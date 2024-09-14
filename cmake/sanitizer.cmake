include_guard(GLOBAL)

add_library(undefined INTERFACE)
target_compile_options(undefined INTERFACE "-fsanitize=undefined")
target_link_options(undefined INTERFACE "-fsanitize=undefined")

add_library(sanitizer::undefined ALIAS undefined)

add_library(address INTERFACE)
target_compile_options(address INTERFACE "-fsanitize=address")
target_link_options(address INTERFACE "-fsanitize=address")

add_library(sanitizer::address ALIAS address)
