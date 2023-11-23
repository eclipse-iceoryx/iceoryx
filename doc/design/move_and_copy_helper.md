# Move And Copy Helper

## Summary and problem description

Currently, some classes in our codebase use assignment in the constructor (ctor) to avoid code duplication, as seen in issue [#1517](https://github.com/eclipse-iceoryx/iceoryx/issues/1517) . However, it's noted in issue [#1686](https://github.com/eclipse-iceoryx/iceoryx/issues/1686) that self-assignment cannot happen in the ctor. Therefore, an alternative approach should be adopted to replace the current implementation.

One method involves placing the shared logical code within a separate function, hereafter referred to as `copy_and_move_impl`. This function is then called within the ctor and assignment operations, using template parameters to specify the method of member initialization.

## Premise

When a class performs highly consistent tasks during construction (ctor) and assignment, implementers often extract the repetitive parts into a separate `copy_and_move_impl`. However, there are inevitably minor differences between move, copy, and ctor, assignment (for example, during ctor, specific variables need to be initialized to avoid garbage values affecting subsequent behaviors, etc...). In these cases, the `MoveAndCopyHelper` can be used to simplify the code.

## Terminology

- `MoveAndCopyOperations`: an enum class, including

    - `CopyConstructor`
    - `MoveConstructor`
    - `CopyAssignment`
    - `MoveAssignment`

## Goals

The purpose of the `MoveAndCopyHelper` is:
- To allow special constructors to indicate the source of the call (e.g., CopyConstructor) when calling `copy_and_move_impl`.
- To provide a consistent way of expressing data exchange in `copy_and_move_impl`.
- To offer a way to ignore `MoveAndCopyOperations` and forcibly use ctor or assignment, facilitating users' needs in special circumstances.

## Design

- Provide the enum class `MoveAndCopyOperations` to describe the expected behavior of member variables during initialization (e.g., using copy or move). Passing this enum as a template parameter to `copy_and_move_impl` ensures whether special constructors are called as expected.
- Provide functions like `transfer` to ensure that the exchange process of member variables conforms to `MoveAndCopyOperations`.

### Code example

```cpp
// header

#include "iox/move_and_copy_helper.hpp"

#define TMP 1024

class Test
{
  public:
    Test();
    Test(const Test&);
    Test(Test&&);
    Test& operator=(const Test&);
    Test& operator=(Test&&);

    uint64_t size();

  private:
    int m_data[TMP];
    uint64_t m_size{0};
}

// implementation
Test::Test(const Test& rhs)
{
    copy_and_move_impl<MoveAndCopyOperations::CopyConstructor>(rhs);
}

Test::Test(Test&& rhs)
{
     copy_and_move_impl<MoveAndCopyOperations::MoveConstructor>(rhs);
}

Test& Test::operator=(const Test& rhs)
{
    if (this != &rhs)
    {
        copy_and_move_impl<MoveAndCopyOperations::CopyAssignment>(rhs);
    }
    return *this;
}

Test& Test::operator=(Test&& rhs)
{
    if (this != &rhs)
    {
        copy_and_move_impl<MoveAndCopyOperations::MoveAssignment>(std::move(rhs));
    }
    return *this;
}

void Test::size()
{
    return m_size;
}

template <MoveAndCopyOperations Opt, typename RhsType>
void Test::copy_and_move_impl(RhsType&& rhs)
{
    // you can set alias to simplify code
    using Helper = MoveAndCopyHelper<Opt>;

    // you can determine the current `Opt` at compile time for compile-time branching decisions.
    constexpr bool is_ctor = Helper::is_ctor();
    constexpr bool is_move = Helper::is_move();

    // for ctor operation
    if constexpr (is_ctor)
    {
        // reset something if needed
        reset_something();
    }

    // transfer data example
    for (uint64_t i = 0; i < rhs.size(); ++i)
    {
        if constexpr (is_move)
        {
            Helper::transfer(m_data[i], std::move(rhs.data[i]));
        }
        else
        {
            Helper::transfer(m_data[i], rhs.data[i]);
        }
    }
}
```

For more examples, see

- `iceoryx_dust/container/detail/fixed_position_container.inl`

## Open issues

[#2113 Move MoveAndCopyHelper to iceoryx_hoofs/design](https://github.com/eclipse-iceoryx/iceoryx/issues/2113)
