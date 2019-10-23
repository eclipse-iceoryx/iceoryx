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

#include "iceoryx_posh/roudi/roudi_app.hpp"

/// @todo once we have a config file for RouDi, this can be a private header

namespace iox
{
namespace roudi
{
class IceOryxRouDiApp : public RouDiApp
{
  public:

    /// @brief contructor to create the RouDi daemon with a given config
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] config the configuration to use
    IceOryxRouDiApp(int argc, char* argv[], const RouDiConfig_t& config) noexcept;

    /// @brief starts the execution of the RouDi daemon
    void run() noexcept override;
};

} // namespace roudi
} // namespace iox
