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

#ifndef IOX_POSH_POPO_REQUEST_INL
#define IOX_POSH_POPO_REQUEST_INL

namespace iox
{
namespace popo
{
template <typename T>
template <typename S, typename>
inline void Request<T>::send() noexcept
{
    BaseType::publish();
}

template <typename T>
inline RequestHeader& Request<T>::getRequestHeader() noexcept
{
    return BaseType::template getUserHeader<RequestHeader>();
}

template <typename T>
inline const RequestHeader& Request<T>::getRequestHeader() const noexcept
{
    return reinterpret_cast<const RequestHeader&>(BaseType::getUserHeader());
}
} // namespace popo
} // namespace iox

#endif
