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

#include "iceoryx_utils/internal/cxx/pair.hpp"
#include "iceoryx_utils/internal/communication_channel/transceiver.hpp"

namespace iox
{
/// @brief implementation of the bidirectional communication channel which
///         can be used for inter process communication through different
///         transport layers (aka. protocols) like message queue, qnx message
///         passing, sockets or shared memory fifo variations
/// @code
///     // create communication channel in shared memory
///     BidirectionalCommunicationChannel<std::string, FiFoProtocol> channel;
///
///     auto transceiverA = channel.getFirstTransceiver();
///     auto transceiverB = channel.getSecondTransceiver();
///     SendTransceiverToApplicationA(transceiverA);
///     SendTransceiverToApplicationB(transceiverB);
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
class BidirectionalCommunicationChannel
{
  public:
    using TransportLayer_t = TransportLayer<DataType>;
    using Transceiver_t = Transceiver<DataType, TransportLayer>;
    using TransceiverPair_t = cxx::pair<Transceiver_t, Transceiver_t>;
    using TransportLayerPair_t = cxx::pair<TransportLayer_t, TransportLayer_t>;

    /// @brief creates a new communication channel
    BidirectionalCommunicationChannel();

    /// @brief creates a communication channel and forwards the transportLayer arguments
    ///         into the corresponding transportlayer constructor
    ///        if you need more then one constructor argument you have to pack all the
    ///         required constructor arguments into one struct and give this as a parameter
    /// @param[in] argumentAliceToBob constructor argument for the layer from alice to bob
    /// @param[in] argumentBobToAlice constructor argument for the layer from bob to alice
    template <typename TransportLayerCTorArgument>
    BidirectionalCommunicationChannel(const TransportLayerCTorArgument& argumentAliceToBob,
                                      const TransportLayerCTorArgument& argumentBobToAlice);

    /// @brief the copy/move constructor and assignment operator must be disabled since
    ///         transceiver pair is holding a pointer to the transportLayer which is
    ///         stored inside this channel
    BidirectionalCommunicationChannel(const BidirectionalCommunicationChannel&) = delete;
    BidirectionalCommunicationChannel(BidirectionalCommunicationChannel&&) = delete;
    BidirectionalCommunicationChannel& operator=(const BidirectionalCommunicationChannel&) = delete;
    BidirectionalCommunicationChannel& operator=(BidirectionalCommunicationChannel&&) = delete;

    /// @brief retrieves a pair of transceivers over which two communication partners can communicate.
    ///        IMPORTANT: distribute this pair ONLY to one communication partner pair. If you have
    ///         multiple communication partners you have to create multiple channels!
    Transceiver_t* getFirstTransceiver();
    Transceiver_t* getSecondTransceiver();

  private:
    TransportLayerPair_t m_transportLayerPair;
    TransceiverPair_t m_transceiverPair;
};
} // namespace iox

#include "iceoryx_utils/internal/communication_channel/bidirectional_communication_channel.inl"

