// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_DESIGN_NEWTYPE_HPP
#define IOX_HOOFS_DESIGN_NEWTYPE_HPP

#include "iox/algorithm.hpp"
#include "iox/detail/newtype/arithmetic.hpp"
#include "iox/detail/newtype/assignment.hpp"
#include "iox/detail/newtype/comparable.hpp"
#include "iox/detail/newtype/constructor.hpp"
#include "iox/detail/newtype/convertable.hpp"
#include "iox/detail/newtype/decrementable.hpp"
#include "iox/detail/newtype/incrementable.hpp"
#include "iox/detail/newtype/internal.hpp"
#include "iox/detail/newtype/protected_constructor.hpp"
#include "iox/detail/newtype/sortable.hpp"

namespace iox
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
/// For most generic usage, the IOX_NEW_TYPE macro can be used.
/// @code
///     #include "iox/newtype.hpp"
///
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
///
///             // implement ctors and assignment operators when they are implemented by the base class
///             // this is necessary to prevent warnings from some compilers
///             Index() noexcept = default;
///             Index(const Index&) noexcept = default;
///             Index(Index&&) noexcept = default;
///             Index& operator=(const Index&) noexcept = default;
///             Index& operator=(Index&&) noexcept = default;
///     };
///
///     Index a(123), c(456);   // allowed since we are using the policy ConstructByValueCopy
///     // Index b;             // not allowed since we are not using the policy DefaultConstructable
///     if ( a < c ) {}         // allowed since we are Sortable
///     a = 567;                // allowed since we are assignable
/// @endcode
// AXIVION Next Construct AutosarC++19_03-A10.1.1 : Multiple inheritance needed to add several policies to NewType. The
// Diamond-Problem cannot occur since the policy structs do not inherit.
template <typename Derived, typename T, template <typename, typename> class... Policies>
class NewType : public Policies<Derived, NewType<Derived, T, Policies...>>...
{
  protected:
    /// 'ProtectedConstructor_t' is a compile time variable to select the correct constructors
    /// @NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    NewType(newtype::internal::ProtectedConstructor_t, const T& rhs) noexcept;

    /// @brief copy constructor
    NewType(const NewType& rhs) noexcept;

    /// @brief move constructor
    NewType(NewType&& rhs) noexcept;

    /// @brief copy assignment
    NewType& operator=(const NewType& rhs) noexcept;

    /// @brief move assignment
    NewType& operator=(NewType&& rhs) noexcept;

    /// @note Since 'using Foo = NewType<int>' and 'using Bar = NewType<int>' result
    /// in 'Foo' and 'Bar' being the same type, this enforces the creation of the
    /// new type by inheritance
    ~NewType() = default;

  public:
    /// @brief the type of *this
    using ThisType = NewType<Derived, T, Policies...>;
    /// @brief the type of the underlying value
    using value_type = T;

    /// @brief default constructor
    NewType() noexcept;

    /// @brief construct with value copy
    explicit NewType(const T& rhs) noexcept;

    /// @brief copy by value assignment
    NewType& operator=(const T& rhs) noexcept;

    /// @brief copy by value assignment
    NewType& operator=(T&& rhs) noexcept;

    /// @brief conversion operator
    // AXIVION Next Construct AutosarC++19_03-A13.5.3 : needed to provide convertable policy so that the derived type
    // can be convertable
    explicit operator T() const noexcept;

    template <typename Type>
    friend typename Type::value_type newtype::internal::newTypeAccessor(const Type&) noexcept;

    template <typename Type>
    friend typename Type::value_type& newtype::internal::newTypeRefAccessor(Type&) noexcept;

  private:
    T m_value;
};

} // namespace iox

/// @brief This macro helps to create types with the NewType class.
/// In case the functionality of the underlying type is sufficient and one just
/// wants to prevent to mix types, this macro removes the burden to write
/// boilerplate code.
///
/// @param[in] TypeName is the name of the new type to create
/// @param[in] Type is the underlying type of the new type
/// @param[in] ... is a variadic list of the policies applied to the new type,
/// e.g. iox::newtype::ConstructByValueCopy
///
/// @code
///     #include "iox/newtype.hpp"
///
///     IOX_NEW_TYPE(MyType,
///                  uint64_t,
///                  newtype::ConstructByValueCopy,
///                  newtype::Comparable,
///                  newtype::Sortable,
///                  newtype::AssignByValueCopy);
/// @endcode
// AXIVION Next Construct AutosarC++19_03-M16.0.6 : brackets around macro parameter would lead in this case to compile
// time failures
// AXIVION Next Construct AutosarC++19_03-A16.0.1 : macro is used to reduce boilerplate code for 'NewType'
/// @NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define IOX_NEW_TYPE(TypeName, Type, ...)                                                                              \
    struct TypeName : public iox::NewType<TypeName, Type, __VA_ARGS__>                                                 \
    {                                                                                                                  \
        using ThisType::ThisType;                                                                                      \
        using ThisType::operator=;                                                                                     \
                                                                                                                       \
        TypeName() noexcept = default;                                                                                 \
        TypeName(const TypeName&) noexcept = default;                                                                  \
        TypeName(TypeName&&) noexcept = default;                                                                       \
        TypeName& operator=(const TypeName&) noexcept = default;                                                       \
        TypeName& operator=(TypeName&&) noexcept = default;                                                            \
    }

#include "iox/detail/newtype.inl"

#endif
