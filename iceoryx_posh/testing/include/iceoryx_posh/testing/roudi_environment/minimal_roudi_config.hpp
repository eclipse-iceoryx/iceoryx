// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#ifndef IOX_POSH_ROUDI_ENVIRONMENT_MINIMAL_ROUDI_CONFIG_HPP
#define IOX_POSH_ROUDI_ENVIRONMENT_MINIMAL_ROUDI_CONFIG_HPP

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iox/builder.hpp"

namespace iox
{
namespace testing
{
/// @brief Builder for a minimal RouDiConfig_t with only one MemPool. This significantly speeds up tests which create a
/// shared memory.
class MinimalRouDiConfigBuilder
{
    /// @brief Set the payload chunk size. Default = 128
    IOX_BUILDER_PARAMETER(uint32_t, payloadChunkSize, 128)

    /// @brief Set the payload chunk count. Default = 10
    IOX_BUILDER_PARAMETER(uint32_t, payloadChunkCount, 10)

    /// @brief Set the introspection chunk count. Default = 2
    IOX_BUILDER_PARAMETER(uint32_t, introspectionChunkCount, 2)

  public:
    /// @brief creates the previously configured RouDiConfig_t
    RouDiConfig_t create() const noexcept;
};
} // namespace testing
} // namespace iox

#endif // IOX_POSH_ROUDI_ENVIRONMENT_MINIMAL_ROUDI_CONFIG_HPP
