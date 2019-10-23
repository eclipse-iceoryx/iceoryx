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

namespace iox
{
/// @brief the Transmitter part of the communication channel used by one of the
///     communication partners to send messages. it is also used by the
///     transceiver which inherits from it to provide a sending and receiving
///     interface
template <typename DataType, template <typename> typename TransportLayer>
class Transmitter
{
  public:
    using TransportLayer_t = TransportLayer<DataType>;

    /// @brief constructor of the transmitter which requires a pointer to a non
    ///         changing memory position of the transportlayer
    /// @param[in] f_transportLayer transportLayer where the transmitter can send to
    Transmitter(TransportLayer_t* const f_transportLayer);

    /// @brief sends a message
    /// @param[in] f_message message which should be send
    /// @return if the message could not be sent it returns false, otherwise true
    bool Send(const DataType& f_message);

  private:
    TransportLayer_t* m_transportLayer;
};

} // namespace iox

#include "iceoryx_utils/internal/communication_channel/transmitter.inl"

