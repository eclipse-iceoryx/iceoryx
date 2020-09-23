// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_POOL_DATA_HPP
#define IOX_POSH_ROUDI_PORT_POOL_DATA_HPP

#include "iceoryx_posh/internal/roudi/port_pool_data_base.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolData : public PortPoolDataBase
{
    /// @deprecated #25
    FixedPositionContainer<iox::popo::SenderPortData, MAX_PUBLISHERS> m_senderPortMembers;
    /// @deprecated #25
    FixedPositionContainer<iox::popo::ReceiverPortData, MAX_SUBSCRIBERS> m_receiverPortMembers;

    FixedPositionContainer<iox::popo::PublisherPortData, MAX_PUBLISHERS> m_publisherPortMembers;
    FixedPositionContainer<iox::popo::SubscriberPortData, MAX_SUBSCRIBERS> m_subscriberPortMembers;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_POOL_DATA_HPP
