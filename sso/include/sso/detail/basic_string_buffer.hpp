#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <ranges>
#include <span>

#include "sso/util.hpp"

namespace sso::detail
{

template <typename Char, typename Allocator>
struct basic_string_buffer
{
private:
    using allocator_traits = std::allocator_traits<Allocator>;

public:
    using size_type = allocator_traits::size_type;
    using value_type = allocator_traits::value_type;
    using const_reference = value_type const&;
    using reference = value_type&;
    using const_pointer = allocator_traits::const_pointer;
    using pointer = allocator_traits::pointer;
    using allocator_type = allocator_traits::allocator_type;

    using string_view = std::basic_string_view<Char>;

    constexpr basic_string_buffer(basic_string_buffer const& other)
        : basic_string_buffer(static_cast<string_view>(other))
    {
    }

    explicit constexpr basic_string_buffer(allocator_type const& allocator) noexcept(std::is_nothrow_constructible_v<allocator_type>)
        : allocator_(allocator)
    {
        std::ignore = std::construct_at(reinterpret_cast<short_buf*>(data_.data()));

        data_.back() = std::byte{};

        assert(!is_long());
    }

    constexpr basic_string_buffer() noexcept(noexcept(allocator_type()))
        : basic_string_buffer(allocator_type())
    {
    }

    constexpr basic_string_buffer(basic_string_buffer&& other) noexcept
    {
        unimplemented();
        // other.data_ = nullptr;
        // other.size_ = 0;
        // other.capacity_ = 0;
    }

    constexpr basic_string_buffer(size_type size, value_type value,
                                  allocator_type const& allocator = allocator_type())
    {
        reserve(size);
        std::fill_n(data(), length(), value);
        *(data() + length()) = value_type{};
    }

    explicit constexpr basic_string_buffer(string_view other)
    {
        unimplemented();
        if (other.empty()) return;

        // size_ = other.size();
        // capacity_ = length() + 1;

        // data_ = allocator_traits::allocate(allocator_, capacity());
        // TODO: use iterators
        std::copy(other.data(), other.data() + other.size(), data());
        *(data() + length()) = '\0';
    }

    constexpr basic_string_buffer&
    operator=(basic_string_buffer other)
    {
        using std::swap;

        swap(*this, other);

        return *this;
    }

    constexpr basic_string_buffer& operator=(basic_string_buffer&&) = delete;

    ~basic_string_buffer()
    {
        if (is_long())
        {
            allocator_traits::deallocate(allocator(), data(), capacity());
        }
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        if (is_long()) return get_long()->size_;

        return get_short()->length();
    }

    [[nodiscard]] constexpr size_type
    capacity() const
    {
        if (is_long()) return get_long()->capacity_;

        return short_buf::capacity;
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr const_pointer
    data() const noexcept
    {
        if (is_long()) return get_long()->data_;

        return get_short()->data_.data();
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr pointer
    data() noexcept
    {
        if (is_long()) return get_long()->data_;

        return get_short()->data_.data();
    }

    [[nodiscard]] constexpr allocator_type
    get_allocator() const
    {
        return allocator();
    }

    friend constexpr void
    swap(basic_string_buffer& l, basic_string_buffer& r) noexcept
    {
        using std::swap;

        l.data_.swap(r.data_);
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
        if (is_long())
        {
            auto* const buf{ get_long() };
            *(buf->data_) = value_type{};
            buf->size_ = 0;
        } else
        {
            auto* const buf{ get_short() };
            buf->data_.front() = value_type{};
        }
    }

    [[nodiscard]] constexpr size_type
    max_size() const
    {
        unimplemented();
    }

    constexpr void
    reserve(size_type count)
    {
        if (count <= capacity()) return;
        if (count > max_size())
            throw std::length_error("`count` must not be greater than `max_size()`");

        short_buf old_buf{ is_long() ? short_buf{} : set_long() };

        if (data() != nullptr) allocator_traits::deallocate(allocator(), data(), capacity());

        auto* const buf{ get_long() };
        buf->capacity_ = count + 1;
        buf->data_ = allocator_traits::allocate(allocator(), buf->capacity_);
        std::ranges::copy(std::span{ old_buf.data_.data(), old_buf.length() }, buf->data_);
        buf->size_ = old_buf.length();
        *(buf->data_ + buf->size_) = value_type{};
    }

private:
    struct long_buf
    {
        pointer data_{ nullptr };
        size_type size_{ 0 };
        size_type capacity_ : (sizeof(size_type) - 1) * CHAR_BIT{ 0 };
    };

    struct short_buf
    {
        static constexpr size_type capacity{ sizeof(long_buf) / sizeof(value_type) };
        static_assert(capacity > 1);

        size_type
        length() const
        {
            return std::ranges::find(data_, value_type{}) - begin(data_);
        }

        std::array<value_type, capacity> data_{};
    };

    [[nodiscard]] constexpr bool
    is_long() const
    {
        return data_.back() != std::byte{};
    }

    //! @pre `!is_long()`
    //! @post `is_long()`
    constexpr short_buf
    set_long()
    {
        assert(!is_long());

        short_buf const old_buf = *get_short();

        std::destroy_at(get_short());
        std::ignore = std::construct_at(reinterpret_cast<long_buf*>(data_.data()));

        data_.back() = std::byte{ 1 };

        assert(is_long());

        return old_buf;
    }

    //! @pre `is_long()`
    //! @post `!is_long()`
    constexpr long_buf
    set_short()
    {
        assert(is_long());

        long_buf const old_buf = *get_long();

        std::destroy_at(get_long());
        std::ignore = std::construct_at(reinterpret_cast<short_buf*>(data_.data()));

        data_.back() = std::byte{};

        assert(!is_long());

        return old_buf;
    }

    [[nodiscard]] constexpr allocator_type&
    allocator()
    {
        return allocator_;
    }

    [[nodiscard]] constexpr allocator_type const&
    allocator() const
    {
        return allocator_;
    }

    //! @pre `is_long()`
    [[nodiscard]] constexpr long_buf const*
    get_long() const
    {
        assert(is_long());

        return reinterpret_cast<long_buf const*>(data_.data());
    }

    //! @pre `!is_long()`
    [[nodiscard]] constexpr short_buf const*
    get_short() const
    {
        assert(!is_long());

        return reinterpret_cast<short_buf const*>(data_.data());
    }

    [[nodiscard]] constexpr long_buf*
    get_long()
    {
        assert(is_long());

        return reinterpret_cast<long_buf*>(data_.data());
    }

    //! @pre `!is_long()`
    [[nodiscard]] constexpr short_buf*
    get_short()
    {
        assert(!is_long());

        return reinterpret_cast<short_buf*>(data_.data());
    }

    static_assert(sizeof(long_buf) == sizeof(short_buf),
                  "size of long and short buffers must be equal");

    std::array<std::byte, sizeof(long_buf)> data_{};

    [[no_unique_address]] Allocator allocator_;
};

template <typename Char>
struct iterator
{
};

} // namespace sso::detail
