#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>

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
    using iterator = pointer; //< TODO: use thin wrapper to prevent implicit conversion to/from pointer
    using const_iterator = const_pointer;

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
        : basic_string_buffer{ other.get_allocator() }
    {
        if (other.is_long())
        {
            *get_long() = *other.get_long();
        } else
        {
            *get_short() = *other.get_short();
        }
    }

    constexpr basic_string_buffer(size_type size, value_type value,
                                  allocator_type const& allocator = allocator_type())
        : basic_string_buffer{ allocator }
    {
        reserve(size);

        std::fill_n(this->begin(), size, value);
        if (is_long()) get_long()->size_ = size;

        *(data() + length()) = value_type{};
    }

    explicit constexpr basic_string_buffer(string_view other)
        : basic_string_buffer{}
    {
        if (other.size() >= short_buf::capacity)
        {
            set_long();
            auto* const buf{ get_long() };
            reserve(other.size() + 1);

            std::ranges::copy(other, this->begin());
            buf->size_ = other.size();
            *(buf->data_ + buf->size_) = value_type{};
        } else
        {
            set_short();

            std::ranges::copy(other, this->begin());

            auto* const buf{ get_short() };
            *(buf->data_.data() + buf->length()) = value_type{};
        }
    }

    constexpr basic_string_buffer&
    operator=(basic_string_buffer other)
    {
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

        swap(l.data_, r.data_);
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
        return std::max(long_buf::max_size(), short_buf::max_size());
    }

    //! @throws `std::length_error` if `count > max_size()`
    constexpr void
    reserve(size_type count)
    {
        if (count < capacity()) return;
        if (count > max_size())
            throw std::length_error("`count` must not be greater than `max_size()`");

        basic_string_buffer buf{ get_allocator() };
        buf.set_long();
        auto* long_buf = buf.get_long();
        long_buf->capacity_ = count + 1;
        long_buf->data_ = allocator_traits::allocate(buf.allocator(), buf.capacity());

        std::ranges::copy(*this, buf.begin());

        long_buf->size_ = length();
        *(long_buf->data_ + long_buf->size_) = value_type{};

        swap(*this, buf);
    }

    [[nodiscard]] constexpr iterator
    begin()
    {
        return is_long() ? get_long()->begin() : get_short()->begin();
    }

    [[nodiscard]] constexpr iterator
    end()
    {
        return is_long() ? get_long()->end() : get_short()->end();
    }

    [[nodiscard]] constexpr const_iterator
    begin() const
    {
        return is_long() ? get_long()->begin() : get_short()->begin();
    }

    [[nodiscard]] constexpr const_iterator
    end() const
    {
        return is_long() ? get_long()->end() : get_short()->end();
    }

private:
    struct long_buf;
    struct short_buf;

    [[nodiscard]] constexpr bool
    is_long() const
    {
        return data_.back() != std::byte{};
    }

    //! @post `is_long()`
    constexpr void
    set_long()
    {
        if (!is_long())
        {
            std::destroy_at(get_short());
            std::ignore = std::construct_at(reinterpret_cast<long_buf*>(data_.data()));

            data_.back() = std::byte{ 1 };
        }

        assert(is_long());
    }

    //! @post `!is_long()`
    constexpr void
    set_short()
    {
        if (is_long())
        {
            std::destroy_at(get_long());
            std::ignore = std::construct_at(reinterpret_cast<short_buf*>(data_.data()));

            data_.back() = std::byte{};
        }

        assert(!is_long());
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

    std::array<std::byte, std::max(sizeof(long_buf), sizeof(short_buf))> data_{};

    [[no_unique_address]] Allocator allocator_;
};

template <typename Char, typename Allocator>
struct basic_string_buffer<Char, Allocator>::long_buf
{
    // TODO: use `iterator` instead of `pointer`
    using iterator = basic_string_buffer<Char, Allocator>::pointer;
    using const_iterator = basic_string_buffer<Char, Allocator>::const_pointer;

    [[nodiscard]] constexpr iterator
    begin()
    {
        return data_;
    }

    [[nodiscard]] constexpr iterator
    end()
    {
        return begin() + length();
    }

    [[nodiscard]] constexpr const_iterator
    begin() const
    {
        return data_;
    }

    [[nodiscard]] constexpr const_iterator
    end() const
    {
        return begin() + length();
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        return size_;
    }

    [[nodiscard]] static constexpr size_type
    max_size()
    {
        return size_type{ 1 } << ((sizeof(size_type) - 1) * CHAR_BIT);
    }

    pointer data_{ nullptr };
    size_type size_{ 0 };
    size_type capacity_ : (sizeof(size_type) - 1) * CHAR_BIT{ 0 };
};

template <typename Char, typename Allocator>
struct basic_string_buffer<Char, Allocator>::short_buf
{
    static constexpr size_type capacity{ sizeof(long_buf) / sizeof(value_type) };
    static_assert(capacity > 1);

private:
    using container_type = std::array<value_type, capacity>;

public:
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    [[nodiscard]] constexpr iterator
    begin()
    {
        return data_.begin();
    }

    [[nodiscard]] constexpr iterator
    end()
    {
        return begin() + length();
    }

    [[nodiscard]] constexpr const_iterator
    begin() const
    {
        return data_.begin();
    }

    [[nodiscard]] constexpr const_iterator
    end() const
    {
        return begin() + length();
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        return std::ranges::find(data_, value_type{}) - begin();
    }

    [[nodiscard]] static constexpr size_type
    max_size()
    {
        return capacity - 1;
    }

    std::array<value_type, capacity - 1> data_{};
};

} // namespace sso::detail
