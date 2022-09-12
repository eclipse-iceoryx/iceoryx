// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_FUNCTIONAL_POLICY_HPP
#define IOX_HOOFS_CXX_FUNCTIONAL_POLICY_HPP

#include "iceoryx_hoofs/cxx/attributes.hpp"
#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_platform/unistd.hpp"

#include <utility>

namespace iox
{
namespace cxx
{
template <uint64_t Capacity>
class string;

namespace internal
{
template <typename Derived, class = void>
struct HasValueMethod : std::false_type
{
};

template <typename Derived>
struct HasValueMethod<Derived, cxx::void_t<decltype(std::declval<Derived>().value())>> : std::true_type
{
};

template <typename Derived, class = void>
struct HasGetErrorMethod : std::false_type
{
};

template <typename Derived>
struct HasGetErrorMethod<Derived, cxx::void_t<decltype(std::declval<Derived>().get_error())>> : std::true_type
{
};

void print_expect_message(const char* message) noexcept;

template <typename Derived>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct Expect
{
    /// @brief Expects that the object is valid, otherwise the method prints the
    ///        provided message and induces a fatal error
    /// @tparam StringType the string type of the message. Allowed types are a char array and a cxx::string
    /// @param[in] msg Message which will be printed when the object is invalid
    template <typename StringType>
    void expect(const StringType& msg) const noexcept;

  protected:
    ~Expect() = default;
};

template <typename Derived, typename ValueType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct ExpectWithValue
{
    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @tparam StringType the string type of the message. Allowed types are a char array and a cxx::string
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return a reference to the contained value
    template <typename StringType>
    ValueType& expect(const StringType& msg) & noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @tparam StringType the string type of the message. Allowed types are a char array and a cxx::string
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return a const reference the contained value
    template <typename StringType>
    const ValueType& expect(const StringType& msg) const& noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @tparam StringType the string type of the message. Allowed types are a char array and a cxx::string
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return rvalue reference to the contained value
    template <typename StringType>
    ValueType&& expect(const StringType& msg) && noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @tparam StringType the string type of the message. Allowed types are a char array and a cxx::string
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return const rvalue reference to the contained value
    template <typename StringType>
    const ValueType&& expect(const StringType& msg) const&& noexcept;

  protected:
    ~ExpectWithValue() = default;
};

template <typename Derived, typename ValueType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct ValueOr
{
    /// @brief When the object contains a value a copy will be returned otherwise
    ///        alternative is perfectly forwarded to the ValueType constructor
    /// @param[in]  alternative the return value which will be used when the object does
    ///             not contain a value
    /// @return A copy of the contained value when present otherwise return new ValueType
    ///         with alternative as constructor argument
    template <typename U>
    ValueType value_or(U&& alternative) const& noexcept;

    /// @brief When the object contains a value the value will be returned via move otherwise
    ///        alternative is perfectly forwarded to the ValueType constructor
    /// @param[in]  alternative the return value which will be used when the object does
    ///             not contain a value
    /// @return The contained value is returned via move when present otherwise return new ValueType
    ///         with alternative as constructor argument
    template <typename U>
    ValueType value_or(U&& alternative) && noexcept;

  protected:
    ~ValueOr() = default;
};

template <typename Derived, typename ValueType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct AndThenWithValue
{
    using and_then_callback_t = cxx::function_ref<void(ValueType&)>;
    using const_and_then_callback_t = cxx::function_ref<void(const ValueType&)>;

    /// @note and_then has a template argument since otherwise we encounter issue
    ///       with the type deduction and constness of auto arguments. A detailed
    ///       discussion can be found here:
    ///       https://stackoverflow.com/questions/71797023/type-deduction-for-stdfunction-argument-types-with-auto-adds-const

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable will be called when valid
    /// @return reference to *this
    template <typename Functor>
    Derived& and_then(const Functor& callable) & noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value const reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable will be called when valid
    /// @return const reference to *this
    template <typename Functor>
    const Derived& and_then(const Functor& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable will be called when valid
    /// @return rvalue reference to *this
    template <typename Functor>
    Derived&& and_then(const Functor& callable) && noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value const reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable will be called when valid
    /// @return const rvalue reference to *this
    template <typename Functor>
    const Derived&& and_then(const Functor& callable) const&& noexcept;

  protected:
    ~AndThenWithValue() = default;
};

template <typename Derived>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct AndThen
{
    using and_then_callback_t = cxx::function_ref<void()>;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable will be called when valid
    /// @return reference to *this
    Derived& and_then(const and_then_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable will be called when valid
    /// @return const reference to *this
    const Derived& and_then(const and_then_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable will be called when valid
    /// @return rvalue reference to *this
    Derived&& and_then(const and_then_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable will be called when valid
    /// @return const rvalue reference to *this
    const Derived&& and_then(const and_then_callback_t& callable) const&& noexcept;

  protected:
    ~AndThen() = default;
};

template <typename Derived, typename ErrorType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct OrElseWithValue
{
    using or_else_callback_t = cxx::function_ref<void(ErrorType&)>;
    using const_or_else_callback_t = cxx::function_ref<void(const ErrorType&)>;

    /// @note or_else has a template argument since otherwise we encounter issue
    ///       with the type deduction and constness of auto arguments. A detailed
    ///       discussion can be found here:
    ///       https://stackoverflow.com/questions/71797023/type-deduction-for-stdfunction-argument-types-with-auto-adds-const

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable will be called when invalid
    /// @return reference to *this
    template <typename Functor>
    Derived& or_else(const Functor& callable) & noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error const reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable will be called when invalid
    /// @return const reference to *this
    template <typename Functor>
    const Derived& or_else(const Functor& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable will be called when invalid
    /// @return rvalue reference to *this
    template <typename Functor>
    Derived&& or_else(const Functor& callable) && noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error const reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable will be called when invalid
    /// @return const rvalue reference to *this
    template <typename Functor>
    const Derived&& or_else(const Functor& callable) const&& noexcept;

  protected:
    ~OrElseWithValue() = default;
};

template <typename Derived>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct OrElse
{
    using or_else_callback_t = cxx::function_ref<void()>;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable will be called when invalid
    /// @return reference to *this
    Derived& or_else(const or_else_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable will be called when invalid
    /// @return const reference to *this
    const Derived& or_else(const or_else_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable will be called when invalid
    /// @return rvalue reference to *this
    Derived&& or_else(const or_else_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable will be called when invalid
    /// @return const rvalue reference to *this
    const Derived&& or_else(const or_else_callback_t& callable) const&& noexcept;

  protected:
    ~OrElse() = default;
};

template <typename Derived, typename ValueType, typename ErrorType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct FunctionalInterfaceImpl : public ExpectWithValue<Derived, ValueType>,
                                 public ValueOr<Derived, ValueType>,
                                 public AndThenWithValue<Derived, ValueType>,
                                 public OrElseWithValue<Derived, ErrorType>
{
  protected:
    ~FunctionalInterfaceImpl() = default;
};

template <typename Derived>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct FunctionalInterfaceImpl<Derived, void, void>
    : public Expect<Derived>, public AndThen<Derived>, public OrElse<Derived>
{
  protected:
    ~FunctionalInterfaceImpl() = default;
};

template <typename Derived, typename ValueType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct FunctionalInterfaceImpl<Derived, ValueType, void> : public ExpectWithValue<Derived, ValueType>,
                                                           public ValueOr<Derived, ValueType>,
                                                           public AndThenWithValue<Derived, ValueType>,
                                                           public OrElse<Derived>
{
  protected:
    ~FunctionalInterfaceImpl() = default;
};

template <typename Derived, typename ErrorType>
// not required since a default'ed destructor does not define a destructor, hence the move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct FunctionalInterfaceImpl<Derived, void, ErrorType>
    : public Expect<Derived>, public AndThen<Derived>, public OrElseWithValue<Derived, ErrorType>
{
  protected:
    ~FunctionalInterfaceImpl() = default;
};
} // namespace internal

/// @brief Provides a functional interface for types which have a bool conversion
///        operator. This provides the methods
///          * and_then
///          * or_else
///          * expect
///        When the class has a value method the method
///          * value_or
///        is added and and_then provides a reference in the callback to the underlying value.
///        When the class has a get_error method the or_else method has a parameter to access
///        a reference to the underlying error.
///
/// @note When inheriting from this type one does not have to write additional unit tests.
///       Instead add a factory for your class to `test_cxx_functional_interface_types.hpp`,
///       add the type to the FunctionalInterfaceImplementations and all typed tests will be
///       generated.
template <typename Derived, typename ValueType, typename ErrorType>
using FunctionalInterface = internal::FunctionalInterfaceImpl<Derived, ValueType, ErrorType>;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/functional_interface.inl"

#endif
