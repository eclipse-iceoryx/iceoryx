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

#include "iceoryx_utils/internal/concurrent/fifo.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

namespace iox
{
/// @brief the communication channel fifo protocol which can be used by the
///         communication channel to communicate
/// @param[in] DataType the datatype which should be transferred
/// @param[in] Capacity capacity of the underlying fifo
template <typename DataType, uint64_t Capacity>
class FiFoProtocol
{
  public:
    /// @brief non blocking send which delivers a message
    /// @param[in] f_message message which should be send
    /// @return if the message cannot be delivered it returns false, otherwise true
    bool Send(const DataType& f_message);

    /// @brief non blocking receive.
    /// @return if the protocol received a message the optional does contain the
    ///         message otherwise a cxx::nullopt is returned
    cxx::optional<DataType> TryReceive();

    /// @brief blocking receive. if the protocol received a message the
    ///         optional does contain it.
    /// @return if the destructor is called from a different thread
    ///         then this method returns a cxx::nullopt and this class should not be
    ///         used anymore.
    cxx::optional<DataType> BlockingReceive();

    /// @brief blocking receive with timeout.
    /// @param[in] f_timeout timeout of timedReceive
    /// @return if a message is received during the
    ///         timeout period the message is stored inside the optional, otherwise
    ///         the optional contains a cxx::nullopt
    cxx::optional<DataType> timedReceive(const units::Duration& f_timeout);

  private:
    concurrent::FiFo<DataType, Capacity> m_fifo;
    posix::Semaphore m_semaphore = std::move(posix::Semaphore::create(0)
                                                 .on_error([] {
                                                     std::cerr << "unable to create the semaphore for the fifo protocol"
                                                               << std::endl;
                                                     std::terminate();
                                                 })
                                                 .get_value());
};
} // namespace iox

#include "iceoryx_utils/internal/communication_channel/protocol/fifo_protocol.inl"
