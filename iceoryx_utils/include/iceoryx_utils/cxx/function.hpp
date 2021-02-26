// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_FUNCTION_HPP
#define IOX_UTILS_FUNCTION_HPP

#include "iceoryx_utils/internal/cxx/static_storage.hpp"
#include "iceoryx_utils/internal/cxx/storable_function.hpp"

namespace iox
{
namespace cxx
{
/// @brief A static memory replacement for std::function
///        Allows storing a callable with a given signature if its size does not exceed a limit.
///        This limit can be adjusted by changing the Bytes parameter.
///        In contrast to cxx::function_ref cxx::function objects own everything needed
///        to invoke the underlying callable and can be safely stored.
///        They also support copy and move semantics in natural way
///        by copying or moving the underlying callable.
///
///        Similarly to std::function, they cannot be stored in Shared Memory
///        to be invoked in a different process.
///
///        For the API see storable_function
template <typename Signature, uint64_t Bytes = 128>
using function = storable_function<static_storage<Bytes>, Signature>;

/// @note This alias is needed to improve usability (reordering of template arguments)
/// We reorder template arguments for the user API since
/// the storage type must precede the (required) variadic arguments in the internal one
/// if the static storage is insufficient to store the callable we get a compile time error

/// @note the following would essentially be a complete std::function replacement
/// which would allocate dynamically if the static storages of Bytes is not sufficient
/// to store the callable (i.e. we use an optimized_storage)
/// template <typename Signature, uint64_t Bytes = 128>
/// using function = detail::storable_function<optimized_storage<Bytes>, Signature>;
/// or alternatively
/// template <typename Signature>
/// using function = detail::storable_function<dynamic_storage, Signature>;
///
/// optimized_storage and dynamic_storage would have to be implemented
///


} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_FUNCTION_HPP