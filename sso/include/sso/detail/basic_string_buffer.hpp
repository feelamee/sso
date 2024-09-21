#pragma once

#include <memory>

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

    constexpr basic_string_buffer() noexcept(noexcept(allocator_type()))
        : allocator_(allocator_type())
    {
    }

    constexpr basic_string_buffer(basic_string_buffer const& other)
        : basic_string_buffer(static_cast<string_view>(other))
    {
    }

    explicit constexpr basic_string_buffer(allocator_type const& allocator) noexcept(std::is_nothrow_constructible_v<allocator_type>)
        : allocator_(allocator)
    {
    }

    constexpr basic_string_buffer(basic_string_buffer&& other) noexcept
        : data_(other.data())
        , size_(other.size())
        , capacity_(other.capacity())
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    constexpr basic_string_buffer(size_type size, value_type value,
                                  allocator_type const& allocator = allocator_type())
        : size_(size)
        , capacity_(length() + 1)
        , allocator_(allocator)
    {
        data_ = allocator_traits::allocate(allocator_, capacity());
        std::fill_n(data(), length(), value);
        *(data() + length()) = '\0';
    }

    explicit constexpr basic_string_buffer(string_view other)
    {
        if (other.empty()) return;

        size_ = other.size();
        capacity_ = length() + 1;

        data_ = allocator_traits::allocate(allocator_, capacity());
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
        if (data())
        {
            allocator_traits::deallocate(allocator_, data(), capacity());
        }
    }

    [[nodiscard]] constexpr size_type
    length() const
    {
        return size_;
    }

    [[nodiscard]] constexpr size_type
    capacity() const
    {
        return capacity_;
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

    [[nodiscard]] constexpr allocator_type
    get_allocator() const
    {
        return allocator_;
    }

    friend constexpr void
    swap(basic_string_buffer& l, basic_string_buffer& r) noexcept
    {
        using std::swap;

        swap(l.data_, r.data_);
        swap(l.capacity_, r.capacity_);
        swap(l.size_, r.size_);
    }

    //! @return null-terminated array [ `data()`, `data() + size()` ]
    [[nodiscard]] constexpr const_pointer
    c_str() const noexcept
    {
        // TODO: for now, while SSO isn't implemented
        return length() == 0 ? "" : data();
    }

    constexpr void
    clear() noexcept
    {
        if (length() > 0) *data_ = value_type{};
        size_ = 0;
    }

private:
    pointer data_{ nullptr };
    size_type size_{ 0 };
    size_type capacity_{ 0 };

    // TODO: move to base class to eliminate size for stateless allocator
    Allocator allocator_;
};

template <typename Char>
struct iterator
{
};

} // namespace sso::detail
