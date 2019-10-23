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

#include "iceoryx_utils/communication_channel/bidirectional_communication_channel.hpp"

namespace iox
{
template <typename DataType, template <typename> typename TransportLayer>
inline BidirectionalCommunicationChannel<DataType, TransportLayer>::BidirectionalCommunicationChannel()
    : m_transportLayerPair()
    , m_transceiverPair{{&m_transportLayerPair.first, &m_transportLayerPair.second},
                        {&m_transportLayerPair.second, &m_transportLayerPair.first}}
{
}

template <typename DataType, template <typename> typename TransportLayer>
template <typename TransportLayerCTorArgument>
inline BidirectionalCommunicationChannel<DataType, TransportLayer>::BidirectionalCommunicationChannel(
    const TransportLayerCTorArgument& argumentAliceToBob, const TransportLayerCTorArgument& argumentBobToAlice)
    : m_transportLayerPair({{argumentAliceToBob}, {argumentBobToAlice}})
    , m_transceiverPair{{&m_transportLayerPair.first, &m_transportLayerPair.second},
                        {&m_transportLayerPair.second, &m_transportLayerPair.first}}
{
}

template <typename DataType, template <typename> typename TransportLayer>
inline typename BidirectionalCommunicationChannel<DataType, TransportLayer>::Transceiver_t*
BidirectionalCommunicationChannel<DataType, TransportLayer>::getFirstTransceiver()
{
    return &m_transceiverPair.first;
}

template <typename DataType, template <typename> typename TransportLayer>
inline typename BidirectionalCommunicationChannel<DataType, TransportLayer>::Transceiver_t*
BidirectionalCommunicationChannel<DataType, TransportLayer>::getSecondTransceiver()
{
    return &m_transceiverPair.second;
}

} // namespace iox
