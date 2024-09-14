#pragma once

#include <sso/util.hpp>

#include <cassert>
#include <cstddef>
#include <memory>
#include <ranges>
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

    basic_string(basic_string const& other)
        : size_(other.size())
        , capacity_(size() + 1)
        , allocator_(other.get_allocator())
    {
        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity());
        std::copy(other.data(), other.data() + other.size(), data());
        *(data() + length()) = '\0';
    }

    basic_string(basic_string&& other)
        : data_(other.data())
        , size_(other.size())
        , capacity_(other.capacity())
    {
    }

    basic_string&
    operator=(basic_string other)
    {
        using std::swap;

        swap(*this, other);

        return *this;
    }

    basic_string& operator=(basic_string&&) = delete;

    explicit constexpr basic_string(allocator_type const& allocator) noexcept(std::is_nothrow_constructible_v<allocator_type>)
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
        if (data())
        {
            std::allocator_traits<Allocator>::deallocate(allocator_, data(), capacity());
        }
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

    [[nodiscard]] friend constexpr bool
    operator==(basic_string const& l, basic_string const& r)
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

    friend constexpr void
    swap(basic_string& l, basic_string& r)
    {
        using std::swap;

        swap(l.data_, r.data_);
        swap(l.capacity_, r.capacity_);
        swap(l.size_, r.size_);
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
