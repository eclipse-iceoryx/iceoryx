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

#ifndef IOX_POSH_POPO_RESPONSE_INL
#define IOX_POSH_POPO_RESPONSE_INL

#include "iceoryx_posh/popo/response.hpp"

namespace iox
{
namespace popo
{
template <typename T>
template <typename S, typename>
inline cxx::expected<ServerSendError> Response<T>::send() noexcept
{
    return BaseType::m_members.producerRef.get().send(std::move(*(this)));
}

template <typename T>
template <typename S, typename>
inline cxx::expected<ServerSendError> Response<T>::sendResponse() noexcept
{
    return BaseType::m_members.producerRef.get().send(std::move(*(this)));
}

template <typename T, typename S, typename>
cxx::expected<ServerSendError> send(Response<T>&& responseToSend) noexcept
{
    return responseToSend.sendResponse();
}

template <typename T>
inline cxx::add_const_conditionally_t<ResponseHeader, T>& Response<T>::getResponseHeader() noexcept
{
    return BaseType::getUserHeader();
}

template <typename T>
inline const ResponseHeader& Response<T>::getResponseHeader() const noexcept
{
    return reinterpret_cast<const ResponseHeader&>(BaseType::getUserHeader());
}
} // namespace popo
} // namespace iox

#endif
