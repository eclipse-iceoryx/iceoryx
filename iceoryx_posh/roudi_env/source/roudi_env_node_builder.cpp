// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi_env/roudi_env_node_builder.hpp"

namespace iox::roudi_env
{
RouDiEnvNodeBuilder::RouDiEnvNodeBuilder(const NodeName_t& name) noexcept
    : NodeBuilder(name)
{
    std::move(*this).shares_address_space_with_roudi(true);
}
} // namespace iox::roudi_env
