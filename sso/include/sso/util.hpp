#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>
#include <type_traits>

namespace sso::detail
{

[[noreturn]] inline void
unimplemented(std::source_location location = std::source_location::current())
{
    std::cout << std::format("{}:{}:{}: in {}\n\tunimplemented...", location.file_name(),
                             location.line(), location.column(), location.function_name())
              << std::endl;
    std::abort();
}

[[noreturn]] inline void
unreachable()
{
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

} // namespace sso::detail
