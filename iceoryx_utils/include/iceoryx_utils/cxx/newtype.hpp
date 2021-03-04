// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_UTILS_CXX_NEWTYPE_HPP
#define IOX_UTILS_CXX_NEWTYPE_HPP

#include "iceoryx_utils/cxx/algorithm.hpp"
#include "iceoryx_utils/internal/cxx/newtype/assignment.hpp"
#include "iceoryx_utils/internal/cxx/newtype/comparable.hpp"
#include "iceoryx_utils/internal/cxx/newtype/constructor.hpp"
#include "iceoryx_utils/internal/cxx/newtype/convertable.hpp"
#include "iceoryx_utils/internal/cxx/newtype/internal.hpp"
#include "iceoryx_utils/internal/cxx/newtype/protected_constructor.hpp"
#include "iceoryx_utils/internal/cxx/newtype/sortable.hpp"

#include <type_traits>

namespace iox
{
namespace cxx
{
/// @brief Implementation of the haskell NewType pattern:
///         https://wiki.haskell.org/Newtype
/// Lets say you would like to have an index which is in the end an integer but
/// with certain restraints. The users should be forced to set it when they are
/// creating it but afterwards it should be immutable.
/// You would like to be able to compare the type as well as to sort it so that
/// it can be stored in a map for instance.
/// An example could be that you would like to have an index class
/// with those properties and some additional methods. Then you can
/// inherit from NewType and add your methods.
/// @code
///     class Index : public NewType<int,
///                                     newtype::ConstructByValueCopy,
///                                     newtype::Comparable,
///                                     newtype::Sortable,
///                                     newtype::AssignByValueCopy>
///     {
///         public:
///         // VERY IMPORTANT: we have to put the constructors and operator= in scope
///         //                 otherwise the code will not compile
///             using ThisType::ThisType;   // this makes all constructors of NewType available for Index
///             using ThisType::operator=;  // put the assignment operators in scope
///     };
///
///     Index a(123), c(456);   // allowed since we are using the policy ConstructByValueCopy
///     // Index b;             // not allowed since we are not using the policy DefaultConstructable
///     if ( a < c ) {}         // allowed since we are Sortable
///     a = 567;                // allowed since we are assignable
/// @endcode
template <typename T, template <typename> class... Policies>
class NewType : public Policies<NewType<T, Policies...>>...
{
  protected:
    NewType(newtype::internal::ProtectedConstructor_t, const T& rhs) noexcept;

  public:
    /// @brief the type of *this
    using ThisType = NewType<T, Policies...>;
    /// @brief the type of the underlying value
    using value_type = T;

    /// @brief default constructor
    NewType() noexcept;

    /// @brief construct with value copy
    explicit NewType(const T& rhs) noexcept;

    /// @brief copy constructor
    NewType(const NewType& rhs) noexcept;

    /// @brief move constructor
    NewType(NewType&& rhs) noexcept;

    /// @brief copy assignment
    NewType& operator=(const NewType& rhs) noexcept;

    /// @brief move assignment
    NewType& operator=(NewType&& rhs) noexcept;

    /// @brief copy by value assignment
    NewType& operator=(const T& rhs) noexcept;

    /// @brief copy by value assignment
    NewType& operator=(T&& rhs) noexcept;

    /// @brief conversion operator
    explicit operator T() const noexcept;

    template <typename Type>
    friend typename Type::value_type newtype::internal::newTypeAccessor(const Type&) noexcept;

  private:
    T m_value;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/newtype.inl"

#endif
