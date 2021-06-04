// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_STORABLE_FUNCTION_HPP
#define IOX_HOOFS_STORABLE_FUNCTION_HPP

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/internal/cxx/static_storage.hpp"

#include <iostream>
#include <utility>


namespace iox
{
namespace cxx
{
template <typename ReturnType, typename... Args>
using signature = ReturnType(Args...);

template <typename StorageType, typename T>
class storable_function;

/// @brief A storable alternative of std::function which uses memory defined by a StorageType.
///        This can be dynamic storage, static storage or anything else adhering to the allocation interface (cf.
///        static_storage).

/// @note This is not achievable with std::function and a custom allocator, as then the memory will still not
///       be part of the object and copying (and moving may cause subtle issues). Hence a complete implementation
///       is required.
///       Furthermore the allocator support of std::function in the STL is deprecated.

/// @tparam StorageType The type of internal storage to store the actual data.
///                     Needs to provide allocate and deallocate functions.
///                     See static_storage.hpp for a static memory version.
/// @tparam ReturnType  The return type of the stored callable.
/// @tparam Args        The arguments of the stored callable.
template <typename StorageType, typename ReturnType, typename... Args>
class storable_function<StorageType, signature<ReturnType, Args...>>
{
  public:
    using signature_t = signature<ReturnType, Args...>;

    storable_function() noexcept = default;

    /// @brief construct from functor (including lambdas)
    ///
    /// @note  Will not compile for StorageType = static_storage if the functor cannot be stored.
    ///        For other StorageTypes it will terminate at runtime if the functor cannot be stored
    ///        (and this cannot be detected at compile-time).
    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                     && is_invocable_r<ReturnType, Functor, Args...>::value,
                                                 void>::type>
    storable_function(const Functor& functor) noexcept;

    /// @brief construct from function pointer (including static functions)
    storable_function(ReturnType (*function)(Args...)) noexcept;

    /// @brief construct from object reference and member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(T& object, ReturnType (T::*method)(Args...)) noexcept;

    /// @brief construct from object reference and const member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(const T& object, ReturnType (T::*method)(Args...) const) noexcept;

    storable_function(const storable_function& other) noexcept;

    storable_function(storable_function&& other) noexcept;

    storable_function& operator=(const storable_function& rhs) noexcept;

    storable_function& operator=(storable_function&& rhs) noexcept;

    ~storable_function() noexcept;

    /// @brief invoke the stored function
    /// @param args arguments to invoke the stored function with
    /// @return return value of the stored function
    ///
    /// @note 1) Invoking the function if there is no stored function (i.e. operator bool returns false)
    ///          leads to terminate being called.
    ///
    ///       2) Deliberately not noexcept but can only throw if the stored callable can throw an exception (hence
    ///          will never throw if we use only our own noexcept functions).
    ///
    ///       3) If arguments are passed by value, the copy constructor may be invoked twice:
    ///          once when passing the arguments to operator() and once when they are passed to the stored callable
    ///          itself. This appears to be unavoidable and also happens in std::function.
    ///          The user can always provide a wrapped callable which takes a reference,
    ///          which is generally preferable for large objects anyway.
    ///
    ///       4) Arguments of class type cannot have the move constructor explicitly deleted since the arguments
    ///          must be forwarded internally which is done by move or, if no move is specified, by copy.
    ///          If the move operation is explicitly deleted the compiler will not fall back to copy but emit an error.
    ///          Not specifying move or using a default implementation is fine.
    ///          This is also the case for std::function (for the gcc implementation at least).
    ///
    ReturnType operator()(Args... args);


    /// @brief indicates whether a function is currently stored
    /// @return true if a function is stored, false otherwise
    operator bool() noexcept;

    /// @brief swap this with another storable function
    /// @param f the function to swap this with
    void swap(storable_function& f) noexcept;

    /// @brief size in bytes required to store a CallableType in a storable_function
    /// @return number of bytes StorageType must be able to allocate to store CallableType
    /// @note this is not smallest possible due to alignment, it may work with a smaller size but
    ///       is not guaranteed (but it is guaranteed to work with the number of bytes returned)
    template <typename CallableType>
    static constexpr uint64_t required_storage_size() noexcept;

    /// @brief checks whether CallableType is storable
    /// @return true if CallableType can be stored, false if it is not guaranteed that it can be stored
    /// @note it might be storable for some alignments of CallableType even if it returns false,
    ///       in this case it is advised to increase the size of storage via the StorageType
    template <typename CallableType>
    static constexpr bool is_storable() noexcept;

  private:
    // Required to perform the correct operations with the underlying erased type
    // This means storable_function cannot be used where pointers become invalid, e.g. across process boundaries
    // Therefore we cannot store a storable_function in shared memory (the same holds for std::function).
    // This is inherent to the type erasure technique we (have to) use.
    struct operations
    {
        // function pointers defining copy, move and destroy semantics
        void (*copyFunction)(const storable_function& src, storable_function& dest){nullptr};
        void (*moveFunction)(storable_function& src, storable_function& dest){nullptr};
        void (*destroyFunction)(storable_function& f){nullptr};

        operations() noexcept = default;
        operations(const operations& other) noexcept = default;
        operations& operator=(const operations& other) noexcept = default;
        operations(operations&& other) noexcept = default;
        operations& operator=(operations&& other) noexcept = default;

        void copy(const storable_function& src, storable_function& dest) noexcept;

        void move(storable_function& src, storable_function& dest) noexcept;

        void destroy(storable_function& f) noexcept;
    };

  private:
    operations m_operations;   // operations depending on type-erased callable (copy, move, destroy)
    StorageType m_storage;     // storage for the callable
    void* m_callable{nullptr}; // pointer to stored type-erased callable
    ReturnType (*m_invoker)(void*, Args&&...){nullptr}; // indirection to invoke the stored callable,
                                                        // nullptr if no callable is stored

    /// @note For static_storage as the StorageType we detect at compile time if the functor can be stored.
    ///       If this is not the case, compilation will fail.
    ///       For other StorageTypes where storing might fail due to insufficent memory this may not be detectable
    ///       at compile time and we call terminate at runtime if the functor could not be stored.

    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                     && is_invocable_r<ReturnType, Functor, Args...>::value,
                                                 void>::type>
    void storeFunctor(const Functor& functor) noexcept;

    bool empty() const noexcept;

    // we need these templates to preserve the actual CallableType for the underlying call
    template <typename CallableType>
    static void copy(const storable_function& src, storable_function& dest) noexcept;

    template <typename CallableType>
    static void move(storable_function& src, storable_function& dest) noexcept;

    template <typename CallableType>
    static void destroy(storable_function& f) noexcept;

    template <typename CallableType>
    static ReturnType invoke(void* callable, Args&&... args);

    static void copyFreeFunction(const storable_function& src, storable_function& dest) noexcept;

    static void moveFreeFunction(storable_function& src, storable_function& dest) noexcept;

    static ReturnType invokeFreeFunction(void* callable, Args&&... args);
};

/// @brief swap two storable functions
/// @param f the first function to swap with g
/// @param g the second function to swap with f
template <typename S, typename T>
static void swap(storable_function<S, T>& f, storable_function<S, T>& g) noexcept;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/storable_function.inl"

#endif // IOX_HOOFS_STORABLE_FUNCTION_HPP
