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

#include "iceoryx_utils/internal/communication_channel/receiver.hpp"
#include "iceoryx_utils/internal/communication_channel/transmitter.hpp"

namespace iox
{
/// @brief implementation of the unidirectional communication channel which
///         can be used for inter process communication through different
///         transport layers (aka. protocols) like message queue, qnx message
///         passing, sockets or shared memory fifo variations
/// @code
///     // create communication channel in shared memory
///     UniidirectionalCommunicationChannel<std::string, FiFoProtocol> channel;
///
///     SendTransmitterToApplicationA(channel.getTransmitter());
///     SendReceiverToApplicationB(channel.getReceiver());
///
///     // in application A
///     channelToApplicationB.Send("Hello World");
///
///     // in application B
///     auto message = channelToApplicationA.BlockingReceive();
///     if ( message.has_value() ) {
///         std::cout << "received message " << message.value() << " from application A\n";
///     }
/// @endcode
template <typename DataType, template <typename> typename TransportLayer>
class UnidirectionalCommunicationChannel
{
  public:
    using TransportLayer_t = TransportLayer<DataType>;
    using Transmitter_t = Transmitter<DataType, TransportLayer>;
    using Receiver_t = Receiver<DataType, TransportLayer>;

    /// @brief creates a new communication channel
    UnidirectionalCommunicationChannel();

    /// @brief the copy/move constructor and assignment operator must be disabled since
    ///         transceiver pair is holding a pointer to the transportLayer which is
    ///         stored inside this channel
    UnidirectionalCommunicationChannel(const UnidirectionalCommunicationChannel&) = delete;
    UnidirectionalCommunicationChannel(UnidirectionalCommunicationChannel&&) = delete;
    UnidirectionalCommunicationChannel& operator=(const UnidirectionalCommunicationChannel&) = delete;
    UnidirectionalCommunicationChannel& operator=(UnidirectionalCommunicationChannel&&) = delete;

    /// @brief creates a communication channel and forwards the transportLayer argument
    ///         into the corresponding transportlayer constructor
    ///        if you need more then one constructor argument you have to pack all the
    ///         required constructor arguments into one struct and give this as a parameter
    /// @param[in] argument constructor argument for the layer
    template <typename TransportLayerCTorArgument>
    UnidirectionalCommunicationChannel(const TransportLayerCTorArgument& argument);

    /// @brief returns the transmitter so that it can be given to the sender partner
    Transmitter_t* getTransmitter();

    /// @brief returns the receiver so that it can be given to the receiving partner
    Receiver_t* getReceiver();

  private:
    TransportLayer_t m_transportLayer;
    Transmitter_t m_transmitter;
    Receiver_t m_receiver;
};
} // namespace iox

#include "iceoryx_utils/internal/communication_channel/unidirectional_communication_channel.inl"
