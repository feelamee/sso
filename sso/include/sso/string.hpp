#pragma once

#include <sso/util.hpp>

#include <algorithm>
#include <cassert>
#include <format>
#include <memory>
#include <ranges>
#include <string_view>

#include "sso/detail/basic_string_buffer.hpp"

namespace sso
{

template <typename Char, typename Allocator = std::allocator<Char>>
struct basic_string
{
private:
    using basic_string_buffer = detail::basic_string_buffer<Char, Allocator>;
    using allocator_traits = std::allocator_traits<typename basic_string_buffer::allocator_type>;

public:
    using size_type = allocator_traits::size_type;
    using value_type = allocator_traits::value_type;
    using const_reference = basic_string_buffer::const_reference;
    using reference = basic_string_buffer::reference;
    using const_pointer = allocator_traits::const_pointer;
    using pointer = allocator_traits::pointer;
    using allocator_type = allocator_traits::allocator_type;

    using string_view = std::basic_string_view<Char>;

    constexpr basic_string(basic_string const& other)
        : basic_string(static_cast<string_view>(other))
    {
    }

    constexpr basic_string(basic_string&& other) noexcept { swap(*this, other); }

    explicit constexpr basic_string(allocator_type const& allocator = allocator_type())
        : buffer(allocator)
    {
    }

    constexpr basic_string(size_type size, value_type value, allocator_type const& allocator = allocator_type())
        : buffer(size, value, allocator)
    {
    }

    explicit constexpr basic_string(string_view other)
        : buffer(std::move(other))
    {
    }

    explicit constexpr basic_string(value_type const* c_str)
        : basic_string(string_view(c_str))
    {
    }

    constexpr basic_string&
    operator=(basic_string other)
    {
        using std::swap;

        swap(*this, other);

        return *this;
    }

    constexpr basic_string& operator=(basic_string&&) = delete;

    [[nodiscard]] constexpr
    operator string_view() const noexcept
    {
        return std::basic_string_view<value_type>(data(), size());
    }

    constexpr ~basic_string() = default;

    [[nodiscard]] constexpr bool
    empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] constexpr size_type
    capacity() const noexcept
    {
        return buffer.capacity();
    }

    [[nodiscard]] constexpr size_type
    size() const noexcept
    {
        return buffer.length();
    }

    [[nodiscard]] constexpr size_type
    length() const noexcept
    {
        return size();
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr const_pointer
    data() const noexcept
    {
        return buffer.data();
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr pointer
    data() noexcept
    {
        return buffer.data();
    }

    //! @pre `position < size()`
    [[nodiscard]] constexpr const_reference
    operator[](size_type position) const noexcept
    {
        assert(position < size());

        return *(data() + position);
    }

    //! @pre `position < size()`
    [[nodiscard]] constexpr reference
    operator[](size_type position) noexcept
    {
        assert(position < size());

        return *(data() + position);
    }

    [[nodiscard]] constexpr allocator_type
    get_allocator() const
    {
        return buffer.get_allocator();
    }

    [[nodiscard]] friend constexpr bool
    operator==(basic_string const& l, basic_string const& r) noexcept
    {
        if (l.size() != r.size()) return false;

        // TODO: replace with `std::equal` after implementing iterator
        bool res = true;
        for (auto const i : std::views::iota(size_type{ 0 }, l.size()))
        {
            if (l[i] != r[i])
            {
                res = false;
                break;
            }
        }

        return res;
    }

    [[nodiscard]] friend constexpr bool
    operator==(basic_string const& l, string_view const& r) noexcept
    {
        return static_cast<string_view>(l) == r;
    }

    friend constexpr void
    swap(basic_string& l, basic_string& r) noexcept
    {
        using std::swap;

        swap(l.buffer, r.buffer);
    }

    //! @pre `!empty()`
    [[nodiscard]] constexpr const_reference
    front() const noexcept
    {
        assert(!empty());

        return (*this)[0];
    }

    //! @pre `!empty()`
    [[nodiscard]] constexpr const_reference
    back() const noexcept
    {
        assert(!empty());

        return (*this)[size() - 1];
    }

    //! @pre `!empty()`
    [[nodiscard]] constexpr reference
    front() noexcept
    {
        assert(!empty());

        return (*this)[0];
    }

    //! @pre `!empty()`
    [[nodiscard]] constexpr reference
    back() noexcept
    {
        assert(!empty());

        return (*this)[size() - 1];
    }

    //! @return null-terminated array [ `data()`, `data() + size()` ]
    [[nodiscard]] constexpr const_pointer
    c_str() const noexcept
    {
        // TODO: for now, while SSO isn't implemented
        return empty() ? "" : data();
    }

    constexpr void
    clear() noexcept
    {
        buffer.clear();
    }

    //! @throw `std::out_of_range` if `position >= size()`
    [[nodiscard]] constexpr reference
    at(size_type position)
    {
        if (auto const size = length(); position >= size)
        {
            throw std::out_of_range(std::format("`position >= size()` is `{} >= {}`", position, size));
        }

        return (*this)[position];
    }

private:
    detail::basic_string_buffer<value_type, allocator_type> buffer;
};

using string = basic_string<char>;

} // namespace sso
