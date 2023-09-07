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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ROUDI_ROUDI_CONFIG_HPP
#define IOX_POSH_ROUDI_ROUDI_CONFIG_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include <cstdint>

namespace iox
{
namespace config
{
struct RouDiConfig
{
    // have some spare chunks to still deliver introspection data in case there are multiple subscribers to the data
    // which are caching different samples; could probably be reduced to 2 with the instruction to not cache the
    // introspection samples
    uint32_t introspectionChunkCount{10};

    uint32_t discoveryChunkCount{10};

    RouDiConfig& setDefaults() noexcept;
    RouDiConfig& optimize() noexcept;
};
} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CONFIG_HPP
