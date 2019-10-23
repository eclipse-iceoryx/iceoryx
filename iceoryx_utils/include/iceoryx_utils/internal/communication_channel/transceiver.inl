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

#include "iceoryx_utils/internal/communication_channel/transceiver.hpp"

namespace iox
{
template <typename DataType, template <typename> typename TransportLayer>
inline Transceiver<DataType, TransportLayer>::Transceiver(TransportLayer_t* const f_transportLayerAliceToBob,
                                                          TransportLayer_t* const f_transportLayerBobToAlice)
    : Receiver<DataType, TransportLayer>(f_transportLayerAliceToBob)
    , Transmitter<DataType, TransportLayer>(f_transportLayerBobToAlice)
{
}
} // namespace iox
