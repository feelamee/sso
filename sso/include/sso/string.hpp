#pragma once

#include <cstddef>

namespace sso
{

struct string
{
public:
    using size_type = std::size_t;

    [[nodiscard]] constexpr bool
    empty() const noexcept
    {
        return true;
    }

    [[nodiscard]] constexpr size_type
    capacity() const noexcept
    {
        return 0;
    }

    [[nodiscard]] constexpr size_type
    size() const noexcept
    {
        return 0;
    }

    [[nodiscard]] constexpr size_type
    length() const noexcept
    {
        return size();
    }
};

} // namespace sso
