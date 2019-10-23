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

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

namespace iox
{
/// @brief the Receiver part of the communication channel used by one of the
///     communication partners to receive messages. it is also used by the
///     transceiver which inherits from it to provide a sending and receiving
///     interface
template <typename DataType, template <typename> typename TransportLayer>
class Receiver
{
  public:
    using TransportLayer_t = TransportLayer<DataType>;

    /// @brief constructor of the receiver which requires a pointer to a non
    ///         changing memory position of the transportlayer
    /// @param[in] f_transportLayer transportLayer where the receiver can receive from
    Receiver(TransportLayer_t* const f_transportLayer);

    /// @brief blocking receive with timeout.
    /// @param[in] f_timeout the timeout
    /// @return if no message was received it returns cxx::nullopt otherwise
    ///           the message is contained in the optional
    cxx::optional<DataType> timedReceive(const units::Duration& f_timeout);

    /// @brief blocking receive
    /// @return if the destructor from another was called it returns cxx::nullopt otherwise
    ///           the message is contained in the optional.
    cxx::optional<DataType> BlockingReceive();

    /// @brief non blocking receive
    /// @return if no message was received it returns cxx::nullopt otherwise
    ///           the message is contained in the optional
    cxx::optional<DataType> TryReceive();

  private:
    TransportLayer_t* m_transportLayer;
};
} // namespace iox

#include "iceoryx_utils/internal/communication_channel/receiver.inl"

