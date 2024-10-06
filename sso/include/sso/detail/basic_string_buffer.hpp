#pragma once

#include <sso/util.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <ranges>

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
        set_short();
        construct_short();

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
            set_long();
            *construct_long() = *other.get_long();
        } else
        {
            set_short();
            *construct_short() = *other.get_short();
        }
    }

    constexpr basic_string_buffer(size_type size, value_type value,
                                  allocator_type const& allocator = allocator_type())
        : basic_string_buffer{ allocator }
    {
        reserve(size);

        // TODO: replace with assign from range `std::views::repeat`
        std::fill_n(begin(), size, value);
        set_length(size);
    }

    explicit constexpr basic_string_buffer(string_view other)
        : basic_string_buffer{}
    {
        if (other.size() >= short_buf::capacity)
        {
            set_long();
            construct_long();
        } else
        {
            set_short();
            construct_short();
        }

        reserve(other.size());
        std::ranges::copy(other, begin());
        set_length(other.length());
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
        destroy();
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        if (is_long()) return get_long()->length();

        return get_short()->length();
    }

    [[nodiscard]] constexpr size_type
    capacity() const
    {
        if (is_long()) return get_long()->capacity_ - 1;

        return short_buf::capacity - 1;
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr const_pointer
    data() const noexcept
    {
        if (is_long()) return get_long()->data();

        return get_short()->data();
    }

    //! @return [ `data()`, `data() + size()` ).
    [[nodiscard]] constexpr pointer
    data() noexcept
    {
        if (is_long()) return get_long()->data();

        return get_short()->data();
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
        set_length(0);
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
        if (count <= capacity()) return;
        if (count > max_size())
            throw std::length_error("`count` must not be greater than `max_size()`");

        auto const capacity{ count + 1 };
        auto* const data{ allocator_traits::allocate(allocator(), capacity) };
        auto const size{ length() };

        std::ranges::copy(*this, data);

        destroy();
        set_long();
        auto* const buf{ construct_long() };
        buf->capacity_ = capacity;
        buf->data_ = data;
        set_length(size);
    }

    [[nodiscard]] constexpr iterator
    begin()
    {
        return data();
    }

    [[nodiscard]] constexpr iterator
    end()
    {
        return begin() + length();
    }

    [[nodiscard]] constexpr const_iterator
    begin() const
    {
        return data();
    }

    [[nodiscard]] constexpr const_iterator
    end() const
    {
        return begin() + length();
    }

    //! @pre `pos + count <= lenght()`
    constexpr void
    replace(size_type pos, size_type count, string_view src)
    {
        // TODO: optimize by removing `reserve`
        assert(pos + count <= length());

        auto const src_size{ std::ranges::size(src) };
        auto const new_size{ length() + src_size - count };
        reserve(new_size);

        auto const rest{ *this | std::views::drop(pos + count) };

        if (count > src_size)
        {
            std::ranges::move(rest, begin() + pos + src_size);
        } else if (count < src_size)
        {
            std::ranges::move_backward(rest, end() + src_size - count);
        }

        std::ranges::copy(src, begin() + pos);
        set_length(new_size);
    }

private:
    struct long_buf;
    struct short_buf;

    constexpr void
    destroy()
    {
        if (is_long())
        {
            allocator_traits::deallocate(allocator(), data(), capacity() + 1);
            std::destroy_at(get_long());
        } else
        {
            std::destroy_at(get_short());
        }
    }

    constexpr long_buf*
    construct_long()
    {
        return std::construct_at(reinterpret_cast<long_buf*>(data_.data()));
    }

    constexpr short_buf*
    construct_short()
    {
        return std::construct_at(reinterpret_cast<short_buf*>(data_.data()));
    }

    [[nodiscard]] constexpr bool
    is_long() const
    {
        return data_.back() != std::byte{};
    }

    //! @post `is_long()`
    constexpr void
    set_long()
    {
        data_.back() = std::byte{ 1 };

        assert(is_long());
    }

    //! @post `!is_long()`
    constexpr void
    set_short()
    {
        data_.back() = std::byte{};

        assert(!is_long());
    }

    //! @pre `capacity() >= size`
    //! @post `length() <= length`, due to possibility of '\0' inside string before `data() + size`
    constexpr void
    set_length(size_type size)
    {
        assert(capacity() >= size);

        if (is_long()) get_long()->size_ = size;
        *(data() + size) = value_type{};

        assert(length() <= size);
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
    using pointer = basic_string_buffer<Char, Allocator>::pointer;
    using const_pointer = basic_string_buffer<Char, Allocator>::const_pointer;

    [[nodiscard]] constexpr pointer
    data()
    {
        return data_;
    }

    [[nodiscard]] constexpr const_pointer
    data() const
    {
        return data_;
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
    using pointer = container_type::iterator;
    using const_pointer = container_type::const_iterator;

    [[nodiscard]] constexpr pointer
    data()
    {
        return data_.data();
    }

    [[nodiscard]] constexpr const_pointer
    data() const
    {
        return data_.data();
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        return std::ranges::find(data_, value_type{}) - std::ranges::begin(data_);
    }

    [[nodiscard]] static constexpr size_type
    max_size()
    {
        return capacity - 1;
    }

    std::array<value_type, capacity - 1> data_{};
};

} // namespace sso::detail
