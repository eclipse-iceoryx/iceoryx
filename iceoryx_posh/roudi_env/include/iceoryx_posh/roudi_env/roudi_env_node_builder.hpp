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

#ifndef IOX_POSH_ROUDI_ENV_ROUDI_ENV_NODE_BUILDER_HPP
#define IOX_POSH_ROUDI_ENV_ROUDI_ENV_NODE_BUILDER_HPP

#include "iox/posh/experimental/node.hpp"

namespace iox::roudi_env
{

/// @brief 'RouDiEnvNodeBuilder' can be used for integration tests in combination with the 'RouDiEnv' to use the
/// 'Node' in the same process as 'RouDi'
class RouDiEnvNodeBuilder : public iox::posh::experimental::NodeBuilder
{
  public:
    explicit RouDiEnvNodeBuilder(const NodeName_t& name) noexcept;
};
} // namespace iox::roudi_env

#endif // IOX_POSH_ROUDI_ENV_ROUDI_ENV_NODE_BUILDER_HPP
