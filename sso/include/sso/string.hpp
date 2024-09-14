#pragma once

#include <sso/util.hpp>

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace sso
{

template <typename Char, typename Allocator = std::allocator<Char>>
struct basic_string
{
public:
    using size_type = std::size_t;
    using value_type = Char;
    using const_reference = value_type const&;
    using const_pointer = value_type const*;
    using pointer = value_type*;
    using allocator_type = Allocator;

    constexpr basic_string() noexcept(noexcept(allocator_type()))
        : allocator_(allocator_type())
    {
    }

    constexpr basic_string(allocator_type const& allocator) noexcept(std::is_nothrow_constructible_v<allocator_type>)
        : allocator_(allocator)
    {
    }

    basic_string(size_type size, value_type value, allocator_type const& allocator = allocator_type())
        : size_(size)
        , capacity_(length() + 1)
        , allocator_(allocator)
    {

        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity());
        std::fill_n(data_, length(), value);
        *(data() + length()) = '\0';
    }

    ~basic_string()
    {
        std::allocator_traits<Allocator>::deallocate(allocator_, data(), capacity());
    }

    [[nodiscard]] constexpr bool
    empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] constexpr size_type
    capacity() const noexcept
    {
        return capacity_;
    }

    [[nodiscard]] constexpr size_type
    size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] constexpr size_type
    length() const noexcept
    {
        return size();
    }

    [[nodiscard]] constexpr const_pointer
    data() const noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr pointer
    data() noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr const_reference
    operator[](size_type position) const
    {
        assert(position < size());

        return *(data() + position);
    }

    [[nodiscard]] constexpr allocator_type
    get_allocator() const
    {
        return allocator_;
    }

private:
    value_type* data_{ nullptr };
    size_type size_{ 0 };
    size_type capacity_{ 0 };

    // TODO: move to base class to eliminate size for stateless allocator
    Allocator allocator_;
};

using string = basic_string<char8_t>;

} // namespace sso
