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

#include "iceoryx_utils/cxx/variant.hpp"

#include <functional>
#include <utility>

namespace iox
{
namespace cxx
{
/// @brief helper struct to create an expected which is signalling success more easily
/// @param T type which the success helper class should contain
/// @code
///     cxx::expected<int, float> callMe() {
///         //...
///         return cxx::success<int>(55);
///     }
/// @endcode
template <typename T = void>
struct success
{
    /// @brief constructor which creates a success helper class by copying
    ///         the value of t
    /// @param[in] t value which should be later stored in an expected
    success(const T& t) noexcept;

    /// @brief constructor which creates a success helper class by moving
    ///         the value of t
    /// @param[in] t value which should be later moved into an expected
    success(T&& t) noexcept;
    template <typename... Targs>

    /// @brief constructor which creates a success helper class by forwarding
    ///         arguments to the constructor of T
    /// @param[in] args... arguments which will be perfectly forwarded to the
    ///                     constructor
    success(Targs&&... args) noexcept;

    T value;
};

/// @brief helper struct to create an error only expected which is signalling success more easily
/// @code
///     cxx::expected<float> callMe() {
///         //...
///         return cxx::success<>();
///     }
/// @endcode
template <>
struct success<void>
{
};

/// @brief helper struct to create an expected which is signalling an error more easily
/// @param T type which the success helper class should contain
/// @code
///     cxx::expected<float> callMe() {
///         //...
///         return cxx::error<float>(12.34f);
///     }
/// @endcode
template <typename T>
struct error
{
    /// @brief constructor which creates a error helper class by copying
    ///         the value of t
    /// @param[in] t value which should be later stored in an expected
    error(const T& t) noexcept;

    /// @brief constructor which creates a error helper class by moving
    ///         the value of t
    /// @param[in] t value which should be later moved into an expected
    error(T&& t) noexcept;

    /// @brief constructor which creates a error helper class by forwarding
    ///         arguments to the constructor of T
    /// @param[in] args... arguments which will be perfectly forwarded to the
    ///                     constructor
    template <typename... Targs>
    error(Targs&&... args) noexcept;

    T value;
};

template <typename... T>
class expected;

/// @brief expected implementation from the C++20 proposal with C++11. The interface
///         is inspired by the proposal but it has changes since we are not allowed to
///         throw an exception.
/// @param ErrorType type of the error which can be stored in the expected
///
/// @code
///     cxx::expected<int, float> callMe() {
///         bool l_errorOccured;
///         // ... do stuff
///         if ( l_errorOccured ) {
///             return cxx::error<float>(55.1f);
///         } else if ( !l_errorOccured ) {
///             return cxx::success<int>(123);
///         }
///     }
///
///     cxx::expected<float> errorOnlyMethod() {
///         return callMe().on_error([]{
///             std::cerr << "Error Occured\n";
///             /// perform some action
///         }).on_success([](cxx::expected<int, float> & result){
///             std::cout << "Success, got " << result.get_value() << std::endl;
///             /// perform some action
///         });
///     }
///
///     cxx::expected<std::vector<int>, int> allHailHypnotoad(success<std::vector<int>>({6,6,6}));
///     allHailHypnotoad->push_back(7);
/// @endcode
template <typename ErrorType>
class expected<ErrorType>
{
  public:
    /// @brief default ctor is deleted since you have to clearly state if the
    ///         expected contains a success value or an error value
    expected() = delete;

    /// @brief the copy constructor calls the copy constructor of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected(const expected&) = default;

    /// @brief the move constructor calls the move constructor of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected(expected&&) = default;

    /// @brief calls the destructor of the success value or error value - depending on what
    ///         is stored in the expected
    ~expected() = default;

    /// @brief  calls the copy assignment operator of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected& operator=(const expected&) = default;

    /// @brief  calls the move assignment operator of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected& operator=(expected&&) = default;

    /// @brief  constructs an expected which is signaling success
    /// @param[in] successValue value which will be stored in the expected
    expected(const success<void>& successValue) noexcept;

    /// @brief  constructs an expected which is signaling an error and stores the
    ///         error value provided by errorValue
    /// @param[in] errorValue error value which will be stored in the expected
    expected(const error<ErrorType>& errorValue) noexcept;

    /// @brief  constructs an expected which is signaling an error and stores the
    ///         error value provided by value
    /// @param[in] errorValue error value which will be moved into the expected
    expected(error<ErrorType>&& errorValue) noexcept;

    /// @brief  creates an expected which is signaling success
    /// @return expected signalling success
    static expected create_value() noexcept;

    /// @brief  creates an expected which is signaling an error and perfectly forwards
    ///         the args to the constructor of lErrorType
    /// @param[in] args... arguments which will be forwarded to the ErrorType constructor
    /// @return expected signalling error
    template <typename... Targs>
    static expected create_error(Targs&&... args) noexcept;

    /// @brief  returns true if the expected contains an error otherwise false
    /// @return bool which contains true if the expected contains an error
    bool has_error() const noexcept;

    /// @brief  returns a reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return reference to the internally contained error
    ErrorType& get_error() & noexcept;

    /// @brief  returns a const reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return const reference to the internally contained error
    const ErrorType& get_error() const& noexcept;

    /// @brief  returns a rvalue reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return rvalue reference to the internally contained error
    ErrorType&& get_error() && noexcept;

    /// @brief  returns a const rvalue reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return rvalue reference to the internally contained error
    const ErrorType&& get_error() const&& noexcept;

    /// @brief  if the expected does contain an error the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([](cxx::expected<int, float> & result){
    ///         std::cout << "error occured : " << result.get_error() << std::endl;
    ///     })
    /// @endcode
    const expected& on_error(const std::function<void(expected&)>& callable) const noexcept;

    /// @brief  if the expected does contain an error the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([](cxx::expected<int, float> & result){
    ///         std::cout << "error occured : " << result.get_error() << std::endl;
    ///     })
    /// @endcode
    expected& on_error(const std::function<void(expected&)>& callable) noexcept;

    /// @brief  if the expected does contain an error the given callable is called
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([]{
    ///         std::cout << "error occured " << std::endl;
    ///     })
    /// @endcode
    const expected& on_error(const std::function<void()>& callable) const noexcept;

    /// @brief  if the expected does contain an error the given callable is called
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([]{
    ///         std::cout << "error occured " << std::endl;
    ///     })
    /// @endcode
    expected& on_error(const std::function<void()>& callable) noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([](cxx::expected<int, float> & result){
    ///         std::cout << "we are successful" << std::endl;
    ///     })
    /// @endcode
    const expected& on_success(const std::function<void(expected&)>& callable) const noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([](cxx::expected<int, float> & result){
    ///         std::cout << "we are successful" << std::endl;
    ///     })
    /// @endcode
    expected& on_success(const std::function<void(expected&)>& callable) noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([]{
    ///         std::cout << "we are successful!" << std::endl;
    ///     })
    /// @endcode
    const expected& on_success(const std::function<void()>& callable) const noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([]{
    ///         std::cout << "we are successful!" << std::endl;
    ///     })
    /// @endcode
    expected& on_success(const std::function<void()>& callable) noexcept;

  private:
    expected(variant<ErrorType>&& store, const bool hasError) noexcept;

  private:
    variant<ErrorType> m_store;
    bool m_hasError;
};

/// @brief specialization of the expected class which can contain an error as well as a success value
/// @param ValueType type of the value which can be stored in the expected
/// @param ErrorType type of the error which can be stored in the expected
template <typename ValueType, typename ErrorType>
class expected<ValueType, ErrorType>
{
  public:
    /// @brief default ctor is deleted since you have to clearly state if the
    ///         expected contains a success value or an error value
    expected() = delete;

    /// @brief the copy constructor calls the copy constructor of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected(const expected&) = default;

    /// @brief the move constructor calls the move constructor of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected(expected&&) = default;

    /// @brief calls the destructor of the success value or error value - depending on what
    ///         is stored in the expected
    ~expected() = default;

    /// @brief  calls the copy assignment operator of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected& operator=(const expected&) = default;

    /// @brief  calls the move assignment operator of the contained success value
    ///         or the error value - depending on what is stored in the expected
    expected& operator=(expected&&) = default;

    /// @brief  constructs an expected which is signaling success and uses the value
    ///         provided by successValue to copy construct its success value
    /// @param[in] successValue value which will be stored in the expected
    expected(const success<ValueType>& successValue) noexcept;

    /// @brief  constructs an expected which is signaling success and uses the value
    ///         provided by successValue to move construct its success value
    /// @param[in] successValue value which will be moved into the expected
    expected(success<ValueType>&& successValue) noexcept;

    /// @brief  constructs an expected which is signaling an error and stores the
    ///         error value provided by errorValue
    /// @param[in] errorValue error value which will be stored in the expected
    expected(const error<ErrorType>& errorValue) noexcept;

    /// @brief  constructs an expected which is signaling an error and stores the
    ///         error value provided by errorValue
    /// @param[in] errorValue error value which will be moved into the expected
    expected(error<ErrorType>&& errorValue) noexcept;

    /// @brief  creates an expected which is signaling success and perfectly forwards
    ///         the args to the constructor of ValueType
    /// @param[in] args... arguments which will be forwarded to the ValueType constructor
    /// @return expected signalling success
    template <typename... Targs>
    static expected create_value(Targs&&... args) noexcept;

    /// @brief  creates an expected which is signaling an error and perfectly forwards
    ///         the args to the constructor of ErrorType
    /// @param[in] args... arguments which will be forwarded to the ErrorType constructor
    /// @return expected signalling error
    template <typename... Targs>
    static expected create_error(Targs&&... args) noexcept;

    /// @brief  returns true if the expected contains an error otherwise false
    /// @return bool which contains true if the expected contains an error
    bool has_error() const noexcept;

    /// @brief  returns a reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return reference to the internally contained error
    ErrorType& get_error() & noexcept;

    /// @brief  returns a const reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return const reference to the internally contained error
    const ErrorType& get_error() const& noexcept;

    /// @brief  returns a rvalue reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return rvalue reference to the internally contained error
    ErrorType&& get_error() && noexcept;

    /// @brief  returns a const rvalue reference to the contained error value, if the expected
    ///         does not contain an error this is undefined behavior
    /// @return rvalue reference to the internally contained error
    const ErrorType&& get_error() const&& noexcept;

    /// @brief  returns a reference to the contained success value, if the expected
    ///         does not contain a success value this is undefined behavior
    /// @return reference to the internally contained value
    ValueType& get_value() & noexcept;

    /// @brief  returns a const reference to the contained success value, if the expected
    ///         does not contain a success value this is undefined behavior
    /// @return const reference to the internally contained value
    const ValueType& get_value() const& noexcept;

    /// @brief  returns a reference to the contained success value, if the expected
    ///         does not contain a success value this is undefined behavior
    /// @return rvalue reference to the internally contained value
    ValueType&& get_value() && noexcept;

    /// @brief  returns a const rvalue reference to the contained success value, if the expected
    ///         does not contain a success value this is undefined behavior
    /// @return const rvalue reference to the internally contained value
    const ValueType&& get_value() const&& noexcept;

    /// @brief  returns a copy of the contained success value if the expected does
    ///         contain a success value, otherwise it returns a copy of value
    /// @return copy of the internally contained value or copy of value
    ValueType get_value_or(const ValueType& value) const noexcept;

    /// @brief  returns a copy of the contained success value if the expected does
    ///         contain a success value, otherwise it returns a copy of value
    /// @return copy of the internally contained value or copy of value
    ValueType get_value_or(const ValueType& value) noexcept;

    /// @brief dereferencing operator which returns a reference to the contained
    ///         success value. if the expected contains an error the behavior is
    ///         undefined.
    /// @return reference to the contained value
    /// @code
    ///     cxx::expected<int, float> frodo(success<int>(45));
    ///     *frodo += 12;
    ///     std::cout << *frodo << std::endl; // prints 57
    /// @endcode
    ValueType& operator*() noexcept;

    /// @brief dereferencing operator which returns a reference to the contained
    ///         success value. if the expected contains an error the behavior is
    ///         undefined.
    /// @return const reference to the contained value
    /// @code
    ///     cxx::expected<int, float> frodo(success<int>(45));
    ///     *frodo += 12;
    ///     std::cout << *frodo << std::endl; // prints 57
    /// @endcode
    const ValueType& operator*() const noexcept;

    /// @brief arrow operator which returns the pointer to the contained success value.
    ///         if the expected contains an error the behavior is undefined.
    /// @return pointer of type ValueType to the contained value
    /// @code
    ///     cxx::expected<std::vector<int>, int> holyPiotr(success<std::vector<int>>({1,2,3}));
    ///     holyPiotr->push_back(4);
    /// @endcode
    ValueType* operator->() noexcept;

    /// @brief arrow operator which returns the pointer to the contained success value.
    ///         if the expected contains an error the behavior is undefined.
    /// @return pointer of type const ValueType to the contained value
    /// @code
    ///     cxx::expected<std::vector<int>, int> holyPiotr(success<std::vector<int>>({1,2,3}));
    ///     holyPiotr->push_back(4);
    /// @endcode
    const ValueType* operator->() const noexcept;

    /// @brief conversion operator to an error only expected which can be useful
    ///         if you would like to return only the success of a function
    /// @return converts an expected which can contain a value and an error to an
    ///         expected which contains only an error
    /// @code
    ///     cxx::expected<int, int> someErrorProneFunction(){}
    ///
    ///     cxx::expected<int> isItSuccessful() {
    ///         return someErrorProneFunction();
    ///     }
    /// @endcode
    template <typename T>
    operator expected<T>() noexcept;

    /// @brief conversion operator to an error only expected which can be useful
    ///         if you would like to return only the success of a function
    /// @return converts an expected which can contain a value and an error to an
    ///         expected which contains only an error
    /// @code
    ///     cxx::expected<int, int> someErrorProneFunction(){}
    ///
    ///     cxx::expected<int> isItSuccessful() {
    ///         return someErrorProneFunction();
    ///     }
    /// @endcode
    template <typename T>
    operator expected<T>() const noexcept;

    /// @brief  if the expected does contain an error the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([](cxx::expected<int, float> & result){
    ///         std::cout << "error occured : " << result.get_error() << std::endl;
    ///     })
    /// @endcode
    const expected& on_error(const std::function<void(expected&)>& callable) const noexcept;

    /// @brief  if the expected does contain an error the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return reference to the expected itself
    /// @code
    ///     someExpected.on_error([](cxx::expected<int, float> & result){
    ///         std::cout << "error occured : " << result.get_error() << std::endl;
    ///     })
    /// @endcode
    expected& on_error(const std::function<void(expected&)>& callable) noexcept;

    /// @brief  if the expected does contain an error the given callable is called
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_error([]{
    ///         std::cout << "error occured " << std::endl;
    ///     })
    /// @endcode
    const expected& on_error(const std::function<void()>& callable) const noexcept;

    /// @brief  if the expected does contain an error the given callable is called
    /// @param[in] callable callable which will be called if the expected contains an error
    /// @return reference to the expected itself
    /// @code
    ///     someExpected.on_error([]{
    ///         std::cout << "error occured " << std::endl;
    ///     })
    /// @endcode
    expected& on_error(const std::function<void()>& callable) noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([](cxx::expected<int, float> & result){
    ///         std::cout << "we have a result : " << result.get_value() << std::endl;
    ///     })
    /// @endcode
    const expected& on_success(const std::function<void(expected&)>& callable) const noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return reference to the expected itself
    /// @code
    ///     someExpected.on_success([](cxx::expected<int, float> & result){
    ///         std::cout << "we have a result : " << result.get_value() << std::endl;
    ///     })
    /// @endcode
    expected& on_success(const std::function<void(expected&)>& callable) noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return const reference to the expected itself
    /// @code
    ///     someExpected.on_success([]{
    ///         std::cout << "we are successful!" << std::endl;
    ///     })
    /// @endcode
    const expected& on_success(const std::function<void()>& callable) const noexcept;

    /// @brief  if the expected does contain a success value the given callable is called and
    ///         a reference to the expected is given as an argument to the callable
    /// @param[in] callable callable which will be called if the expected contains a success value
    /// @return reference to the expected itself
    /// @code
    ///     someExpected.on_success([]{
    ///         std::cout << "we are successful!" << std::endl;
    ///     })
    /// @endcode
    expected& on_success(const std::function<void()>& callable) noexcept;

  private:
    expected(variant<ValueType, ErrorType>&& f_store, const bool hasError) noexcept;

  private:
    variant<ValueType, ErrorType> m_store;
    bool m_hasError;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/expected.inl"
