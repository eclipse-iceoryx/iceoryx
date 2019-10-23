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

#include "iceoryx_utils/communication_channel/unidirectional_communication_channel.hpp"

namespace iox
{
template <typename DataType, template <typename> typename TransportLayer>
inline UnidirectionalCommunicationChannel<DataType, TransportLayer>::UnidirectionalCommunicationChannel()
    : m_transportLayer()
    , m_transmitter(&m_transportLayer)
    , m_receiver(&m_transportLayer)
{
}

template <typename DataType, template <typename> typename TransportLayer>
template <typename TransportLayerCTorArgument>
inline UnidirectionalCommunicationChannel<DataType, TransportLayer>::UnidirectionalCommunicationChannel(
    const TransportLayerCTorArgument& argument)
    : m_transportLayer(argument)
    , m_transmitter(&m_transportLayer)
    , m_receiver(&m_transportLayer)
{
}

template <typename DataType, template <typename> typename TransportLayer>
inline typename UnidirectionalCommunicationChannel<DataType, TransportLayer>::Transmitter_t*
UnidirectionalCommunicationChannel<DataType, TransportLayer>::getTransmitter()
{
    return &m_transmitter;
}

template <typename DataType, template <typename> typename TransportLayer>
inline typename UnidirectionalCommunicationChannel<DataType, TransportLayer>::Receiver_t*
UnidirectionalCommunicationChannel<DataType, TransportLayer>::getReceiver()
{
    return &m_receiver;
}
} // namespace iox
