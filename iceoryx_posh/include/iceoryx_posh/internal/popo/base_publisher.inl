// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_PUBLISHER_INL
#define IOX_POSH_POPO_BASE_PUBLISHER_INL

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename port_t>
inline BasePublisher<T, port_t>::BasePublisher(const capro::ServiceDescription& service,
                                               const PublisherOptions& publisherOptions)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewarePublisher(service, publisherOptions))
{
}

template <typename T, typename port_t>
inline BasePublisher<T, port_t>::~BasePublisher()
{
    m_port.destroy();
}

template <typename T, typename port_t>
inline uid_t BasePublisher<T, port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename T, typename port_t>
inline capro::ServiceDescription BasePublisher<T, port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::offer() noexcept
{
    m_port.offer();
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template <typename T, typename port_t>
inline bool BasePublisher<T, port_t>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template <typename T, typename port_t>
inline bool BasePublisher<T, port_t>::hasSubscribers() const noexcept
{
    return m_port.hasSubscribers();
}

template <typename T, typename port_t>
const port_t& BasePublisher<T, port_t>::port() const noexcept
{
    return m_port;
}

template <typename T, typename port_t>
port_t& BasePublisher<T, port_t>::port() noexcept
{
    return m_port;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_PUBLISHER_INL
