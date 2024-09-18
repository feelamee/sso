#pragma once

#include <sso/util.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <ranges>
#include <string_view>
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
    using reference = value_type&;
    using const_pointer = value_type const*;
    using pointer = value_type*;
    using allocator_type = Allocator;
    using string_view = std::basic_string_view<Char>;

    constexpr basic_string() noexcept(noexcept(allocator_type()))
        : allocator_(allocator_type())
    {
    }

    constexpr basic_string(basic_string const& other)
        : basic_string(static_cast<string_view>(other))
    {
    }

    constexpr basic_string(basic_string&& other) noexcept
        : data_(other.data())
        , size_(other.size())
        , capacity_(other.capacity())
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

    explicit constexpr basic_string(allocator_type const& allocator) noexcept(std::is_nothrow_constructible_v<allocator_type>)
        : allocator_(allocator)
    {
    }

    constexpr basic_string(size_type size, value_type value, allocator_type const& allocator = allocator_type())
        : size_(size)
        , capacity_(length() + 1)
        , allocator_(allocator)
    {
        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity());
        std::fill_n(data(), length(), value);
        *(data() + length()) = '\0';
    }

    explicit constexpr basic_string(string_view other)
    {
        if (other.empty()) return;

        size_ = other.size();
        capacity_ = size() + 1;

        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity());
        // TODO: use iterators
        std::copy(other.data(), other.data() + other.size(), data());
        *(data() + size()) = '\0';
    }

    explicit constexpr basic_string(value_type const* c_str)
        : basic_string(string_view(c_str))
    {
    }

    constexpr
    operator string_view() const noexcept
    {
        return std::basic_string_view<value_type>(data(), size());
    }

    constexpr ~basic_string()
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

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr const_pointer
    data() const noexcept
    {
        return data_;
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr pointer
    data() noexcept
    {
        return data_;
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
        return allocator_;
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

        swap(l.data_, r.data_);
        swap(l.capacity_, r.capacity_);
        swap(l.size_, r.size_);
    }

    //! @pre `!empty()`
    const_reference
    front() const noexcept
    {
        assert(!empty());

        return (*this)[0];
    }

    //! @pre `!empty()`
    const_reference
    back() const noexcept
    {
        assert(!empty());

        return (*this)[size() - 1];
    }

    //! @pre `!empty()`
    reference
    front() noexcept
    {
        assert(!empty());

        return (*this)[0];
    }

    //! @pre `!empty()`
    reference
    back() noexcept
    {
        assert(!empty());

        return (*this)[size() - 1];
    }

    //! @return null-terminated array [ `data()`, `data() + size()` ]
    const_pointer
    c_str() const noexcept
    {
        // TODO: for now, while SSO isn't implemented
        return empty() ? "" : data();
    }

    void
    clear() noexcept
    {
        if (size() > 0) front() = value_type{};
        size_ = 0;
    }

private:
    value_type* data_{ nullptr };
    size_type size_{ 0 };
    size_type capacity_{ 0 };

    // TODO: move to base class to eliminate size for stateless allocator
    Allocator allocator_;
};

using string = basic_string<char>;

} // namespace sso
