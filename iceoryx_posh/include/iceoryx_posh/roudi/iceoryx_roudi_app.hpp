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
#ifndef IOX_POSH_ROUDI_ICEORYX_ROUDI_APP_HPP
#define IOX_POSH_ROUDI_ICEORYX_ROUDI_APP_HPP

#include "iceoryx_posh/roudi/roudi_app.hpp"

namespace iox
{
namespace roudi
{
class IceOryxRouDiApp : public RouDiApp
{
  public:
    /// @brief constructor to create the RouDi daemon with a given config
    /// @param[in] Command liner parser object, that provides the settings
    /// @param[in] RouDi config for mempool configuration
    IceOryxRouDiApp(const config::CmdLineParser& cmdLineParser, const RouDiConfig_t& roudiConfig) noexcept;

    /// @brief starts the execution of the RouDi daemon
    void run() noexcept override;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ICEORYX_ROUDI_APP_HPP
