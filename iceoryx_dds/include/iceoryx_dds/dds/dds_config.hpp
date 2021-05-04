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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_DDS_DDS_CONFIG_HPP
#define IOX_DDS_DDS_CONFIG_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace dds
{
using namespace units::duration_literals;
static constexpr units::Duration DISCOVERY_PERIOD = 1000_ms;
static constexpr units::Duration FORWARDING_PERIOD = 50_ms;
static constexpr uint32_t SUBSCRIBER_CACHE_SIZE = 128u;

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_CONFIG_HPP
