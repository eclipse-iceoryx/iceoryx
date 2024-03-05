// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_APP_HPP
#define IOX_POSH_ROUDI_ROUDI_APP_HPP

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iox/detail/deprecation_marker.hpp"
#include "iox/logging.hpp"

#include <cstdint>
#include <cstdio>

namespace iox
{
namespace roudi
{
/// @brief base class for RouDi daemons
class RouDiApp
{
  public:
    /// @brief Common ctor to run RouDi
    /// @param[in] config the configuration to use
    RouDiApp(const IceoryxConfig& config) noexcept;

    virtual ~RouDiApp() noexcept {};

    /// @brief interface to start the execution of the RouDi daemon
    /// @return Return code for programm execution
    virtual uint8_t run() noexcept = 0;

  protected:
    /// @brief waits for the next signal to RouDi daemon
    IOX_DEPRECATED_SINCE(3, "Please use iox::waitForTerminationRequest() from 'iox/signal_watcher.hpp'")
    bool waitForSignal() noexcept;

    bool m_run{true};
    IceoryxConfig m_config;

  private:
    bool checkAndOptimizeConfig(const IceoryxConfig& config) noexcept;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_APP_HPP
