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
/// @note set the storage type and reorder template arguments for the user API
/// the storage type must precede the (required) variadic arguments in the internal one
/// if the static storage is insufficient to store the callable we get a compile time error

/// @note this alias is needed to improve usability
///       for the API see storable_function
template <typename Signature, uint64_t Bytes = 128>
using function = storable_function<static_storage<Bytes>, Signature>;

/// @note the following would essentially be a complete std::function replacement
/// which would allocate dynamically if the static storages of Bytes is not sufficient
/// to store the callable (i.e. we use an optimized_storage)
// template <typename Signature, uint64_t Bytes = 128>
// using function = detail::storable_function<optimized_storage<Bytes>, Signature>;
// or alternatively
// template <typename Signature>
// using function = detail::storable_function<dynamic_storage, Signature>;

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_FUNCTION_HPP