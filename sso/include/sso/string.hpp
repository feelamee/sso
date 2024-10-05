#pragma once

#include <sso/detail/basic_string_buffer.hpp>
#include <sso/util.hpp>

#include <algorithm>
#include <cassert>
#include <format>
#include <memory>
#include <ranges>
#include <string_view>

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
    using iterator = basic_string_buffer::iterator;
    using const_iterator = basic_string_buffer::const_iterator;

    using string_view = std::basic_string_view<Char>;

    constexpr basic_string(basic_string const& other)
        : basic_string{ static_cast<string_view>(other) }
    {
    }

    constexpr basic_string(basic_string&& other) noexcept
        : buffer{ static_cast<basic_string_buffer&&>(other.buffer) }
    {
    }

    explicit constexpr basic_string(allocator_type const& allocator = allocator_type())
        : buffer{ allocator }
    {
    }

    constexpr basic_string(size_type size, value_type value, allocator_type const& allocator = allocator_type())
        : buffer{ size, value, allocator }
    {
    }

    explicit constexpr basic_string(string_view other)
        : buffer{ other }
    {
    }

    explicit constexpr basic_string(value_type const* c_str)
        : basic_string{ string_view(c_str) }
    {
    }

    constexpr basic_string&
    operator=(basic_string other)
    {
        using std::swap;

        swap(*this, other);

        return *this;
    }

    constexpr basic_string&
    operator=(string_view other)
    {
        replace(0, length(), other);
        return *this;
    }

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
        return std::ranges::equal(l, r);
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
        return data();
    }

    constexpr void
    clear() noexcept
    {
        buffer.clear();
    }

    //! @throws `std::out_of_range` if `position >= size()`
    [[nodiscard]] constexpr reference
    at(size_type position)
    {
        if (auto const size = length(); position >= size)
        {
            throw std::out_of_range(std::format("`position >= size()` is `{} >= {}`", position, size));
        }

        return (*this)[position];
    }

    [[nodiscard]] constexpr iterator
    begin()
    {
        return buffer.begin();
    }

    [[nodiscard]] constexpr iterator
    end()
    {
        return buffer.end();
    }

    [[nodiscard]] constexpr const_iterator
    begin() const
    {
        return buffer.begin();
    }

    [[nodiscard]] constexpr const_iterator
    end() const
    {
        return buffer.end();
    }

    //! @throws `std::length_error` if `count > max_size()`
    constexpr void
    reserve(size_type size)
    {
        buffer.reserve(size);
    }

    constexpr void
    replace(size_type pos, size_type count, string_view src)
    {
        buffer.replace(pos, count, src);
    }

private:
    detail::basic_string_buffer<value_type, allocator_type> buffer;
};

using string = basic_string<char>;

} // namespace sso
