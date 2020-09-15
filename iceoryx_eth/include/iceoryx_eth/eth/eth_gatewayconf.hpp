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

#ifndef IOX_ETH_GATEWAY_CONFIG_HPP
#define IOX_ETH_GATEWAY_CONFIG_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include <vector>

namespace iox{

    namespace popo{

        struct EthGatewayConf
        {
            iox::capro::ServiceDescription m_serviceDescription;
            uint8_t unique_id;
        };

        
    }
}


extern std::vector<iox::popo::EthGatewayConf> pMap;

#endif