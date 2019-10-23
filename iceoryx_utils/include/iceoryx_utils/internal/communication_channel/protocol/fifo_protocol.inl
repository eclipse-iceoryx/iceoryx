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

#include "iceoryx_utils/communication_channel/protocol/fifo_protocol.hpp"

namespace iox
{
template <typename DataType, uint64_t Capacity>
inline bool FiFoProtocol<DataType, Capacity>::Send(const DataType& message)
{
    if (m_fifo.push(message))
    {
        m_semaphore.post();
        return true;
    }
    return false;
}

template <typename DataType, uint64_t Capacity>
inline cxx::optional<DataType> FiFoProtocol<DataType, Capacity>::TryReceive()
{
    if (m_semaphore.tryWait())
    {
        return m_fifo.pop();
    }
    return cxx::nullopt_t();
}

template <typename DataType, uint64_t Capacity>
inline cxx::optional<DataType> FiFoProtocol<DataType, Capacity>::BlockingReceive()
{
    if (m_semaphore.wait())
    {
        return m_fifo.pop();
    }
    return cxx::nullopt_t();
}

template <typename DataType, uint64_t Capacity>
inline cxx::optional<DataType> FiFoProtocol<DataType, Capacity>::timedReceive(const units::Duration& timeout)
{
    auto timeoutAsTimespec = timeout.timespec(units::TimeSpecReference::Epoch);
    if (m_semaphore.timedWait(&timeoutAsTimespec, true))
    {
        return m_fifo.pop();
    }
    return cxx::nullopt_t();
}

} // namespace iox
