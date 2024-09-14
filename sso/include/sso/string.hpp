#pragma once

#include <cstddef>

namespace sso
{

struct string
{
public:
    [[nodiscard]] constexpr bool
    empty() const noexcept
    {
        return true;
    }

    [[nodiscard]] constexpr size_t
    capacity() const noexcept
    {
        return 0;
    }

    [[nodiscard]] constexpr size_t
    size() const noexcept
    {
        return 0;
    }

    [[nodiscard]] constexpr size_t
    length() const noexcept
    {
        return size();
    }
};

} // namespace sso
