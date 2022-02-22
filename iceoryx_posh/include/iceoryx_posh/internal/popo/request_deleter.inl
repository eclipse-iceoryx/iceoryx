// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_REQUEST_DELETER_INL
#define IOX_POSH_POPO_REQUEST_DELETER_INL

#include "iceoryx_posh/internal/popo/request_deleter.hpp"

namespace iox
{
namespace popo
{
template <typename Port>
RequestDeleter<Port>::RequestDeleter(Port& port) noexcept
    : m_port(&port)
{
}

template <typename Port>
template <typename T>
void RequestDeleter<Port>::operator()(T* const payload) noexcept
{
    auto* requestHeader = iox::popo::RequestHeader::fromPayload(payload);
    m_port->releaseRequest(requestHeader);
}

template <typename Port>
template <typename T>
void RequestDeleter<Port>::operator()(const T* const payload) const noexcept
{
    const auto* requestHeader = iox::popo::RequestHeader::fromPayload(payload);
    m_port->releaseRequest(requestHeader);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_REQUEST_DELETER_INL
