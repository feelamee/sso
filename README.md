### sso - small string optimization

I wrote this just for fun when saw [article about sso](https://tunglevo.com/note/an-optimization-thats-impossible-in-rust/) in Rust.
My string can store up to 23 (excluding null-terminator) 8-bit chars without calling allocator.

There are few ideas, which I not implemented:
  - way to configure usage of exceptions.
    This can be achived with macros, but I want to try use something like traits/policies.
    E.g. `std::string` use `std::char_traits`, what about creating `exception_traits`/`throw_policy`/`throw_strategy`?
    
  - strategy for preallocating more memory then need
    now I allocator exactly as much as I need.
    I want to do this behaviour configurable and see few ways:
      - using `std::allocator_traits::allocate_at_least`. Such way user can configure allocation strategy by providing custom allocator.
      - policy/traits as well as with exceptions
        this less idiomatic for C++, but more consistent if using same way for exceptions too

And there is also one problem(mistake?).
When string is small (placed on stack) I use simple O(n) algo for calculate length.
That's why it can return wrong length if string contain null-terminator inside.
This can be fixed by occupying one byte for store length.
  
#### BUILD
    cmake -S [test | sso] -B build/
    cmake --build build/ -j
