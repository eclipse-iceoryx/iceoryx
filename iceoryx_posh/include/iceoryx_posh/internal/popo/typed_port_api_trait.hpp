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

#ifndef IOX_POSH_POPO_TYPED_PORT_API_TRAIT_HPP
#define IOX_POSH_POPO_TYPED_PORT_API_TRAIT_HPP

#include <type_traits>

namespace iox
{
namespace popo
{
/// @brief This type trait ensures that the template parameter for Publisher, Subscriber, Client and Server fulfill
/// specific constrains.
/// @code
/// template <typename Data>
/// class Producer
/// {
///     using DataTypeAssert = typename TypedPortApiTrait<Data>::Assert;
///   public:
///     // ...
/// }
/// @endcode
/// @note 'typename TypedPortApiTrait<T>::Assert' has to be used otherwise the compiler ignores the static_assert's
template <typename T>
struct TypedPortApiTrait
{
    static_assert(!std::is_void<T>::value, "Must not be void. Use the untyped API for void types");
    static_assert(!std::is_const<T>::value, "Must not be const");
    static_assert(!std::is_reference<T>::value, "Must not be a reference");
    static_assert(!std::is_pointer<T>::value, "Must not be a pointer");
    using Assert = void;
};
} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_PORT_API_TRAIT_HPP
