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

#include "receiver.hpp"
#include "transmitter.hpp"

namespace iox
{
/// @brief offers the combined interface of the Transmitter and Receiver for
///         bidirectional communication channel usage. every communication partner
///         will get one transceiver over which they can communicate.
template <typename DataType, template <typename> typename TransportLayer>
class Transceiver : public Receiver<DataType, TransportLayer>, public Transmitter<DataType, TransportLayer>
{
  public:
    using TransportLayer_t = TransportLayer<DataType>;

    /// @brief the constructor requires two transportLayer pointer since a transportLayer is only
    ///         unidirectional.
    Transceiver(TransportLayer_t* const f_transportLayerAliceToBob, TransportLayer_t* const f_transportLayerBobToAlice);
};
} // namespace iox

#include "iceoryx_utils/internal/communication_channel/transceiver.inl"
