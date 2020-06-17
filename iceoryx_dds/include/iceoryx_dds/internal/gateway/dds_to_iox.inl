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

#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/string.hpp"

namespace iox
{
namespace dds
{
template <typename channel_t>
inline DDS2IceoryxGateway<channel_t>::DDS2IceoryxGateway() noexcept : iox::dds::DDSGatewayGeneric<channel_t>()
{
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::loadConfiguration(const GatewayConfig& config) noexcept
{
}


template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::forward(const channel_t& channel) noexcept
{
}

} // namespace dds
} // namespace iox
