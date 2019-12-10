// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace iox
{
namespace cxx
{
/// This is a proxy which must be used for the non default PoorMansHeap ctor
template <typename Type>
class PoorMansHeapType
{
};

/// @brief Reserves space on stack for placement new instatiation
/// @param Interface base type of all classes which should be stored in here
/// @param TypeSize maximum size of a child of Interface
/// @param TypeAlignment alignment which is required for the types
///
/// @code
/// #include "iceoryx_utils/cxx/poor_mans_heap.hpp"
///
/// #include "iceoryx_utils/cxx/helplets.hpp"
///
/// #include <iostream>
///
/// class Base
/// {
///   public:
///     virtual ~Base() = default;
///     virtual void doStuff() = 0;
/// };
///
/// class Foo : public Base
/// {
///   public:
///     Foo(int stuff)
///         : m_stuff(stuff)
///     {
///     }
///
///     void doStuff() override
///     {
///         std::cout << __PRETTY_FUNCTION__ << ": " << m_stuff << std::endl;
///     }
///
///   private:
///     int m_stuff;
/// };
///
/// class Bar : public Base
/// {
///   public:
///     void doStuff() override
///     {
///         std::cout << __PRETTY_FUNCTION__ << std::endl;
///     }
/// };
///
/// int main()
/// {
///     constexpr auto MaxSize = cxx::maxSize<Foo, Bar>();
///     constexpr auto MaxAlignment = cxx::maxAlignment<Foo, Bar>();
///
///     using FooBar = cxx::PoorMansHeap<Base, MaxSize, MaxAlignment>;
///
///     FooBar fooBar1{cxx::PoorMansHeapType<Foo>(), 42};
///     fooBar1->doStuff();
///
///     fooBar1.newInstance<Bar>();
///     fooBar1->doStuff();
///
///     fooBar1.newInstance<Foo>(13);
///     fooBar1->doStuff();
///
///     FooBar fooBar2;
///     if (!fooBar2.hasInstance())
///     {
///         std::cout << "There is no instance!" << std::endl;
///     }
///
///     fooBar2.newInstance<Bar>();
///     fooBar2->doStuff();
///
///     fooBar2.deleteInstance();
///     if (!fooBar2.hasInstance())
///     {
///         std::cout << "There is again no instance!" << std::endl;
///     }
///
///     return 0;
/// }
/// @endcode
template <typename Interface, size_t TypeSize, size_t TypeAlignment = 8>
class PoorMansHeap
{
  public:
    PoorMansHeap() = default;
    ~PoorMansHeap() noexcept;

    /// Constructor for immediate construction of an instance
    /// @param [in] Type the type to instantiate, wrapped in PoorMansHeapType
    /// @param [in] ctorArgs ctor arguments for the type to instantiate
    template <typename Type, typename... CTorArgs>
    PoorMansHeap(PoorMansHeapType<Type>, CTorArgs&&... ctorArgs) noexcept;

    PoorMansHeap(PoorMansHeap&& other) = delete;
    PoorMansHeap& operator=(PoorMansHeap&& rhs) = delete;

    PoorMansHeap(const PoorMansHeap&) = delete;
    PoorMansHeap& operator=(const PoorMansHeap&) = delete;

    /// Create a new instance of the Type
    /// @param [in] Type the type to instantiate, wrapped in PoorMansHeapType
    /// @param [in] ctorArgs ctor arguments for the type to instantiate
    template <typename Type, typename... CTorArgs>
    void newInstance(CTorArgs&&... ctorArgs) noexcept;

    /// Calls the destructor if there is a valid instance, otherwise nothing happens
    void deleteInstance() noexcept;

    /// Checks is there is a valid instance
    /// @return true if there is a valid instance
    bool hasInstance() const noexcept;

    /// Returns a pointer to the underlying instance
    /// @return pointer to the underlying instance or nullptr if there is no valid instance
    Interface* operator->() const noexcept;

    /// Returns a reference to the underlying instance. If there is no valid instance, the behaviour is undefined
    /// @return reference to the underlying instance
    Interface& operator*() const noexcept;

  private:
    Interface* m_instance{nullptr};
    alignas(TypeAlignment) uint8_t m_heap[TypeSize];
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/poor_mans_heap.inl"
