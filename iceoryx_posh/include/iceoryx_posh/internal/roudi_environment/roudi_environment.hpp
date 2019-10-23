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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_posh/internal/roudi_environment/runtime_test_interface.hpp"

#include <atomic>
#include <map>
#include <mutex>

namespace iox
{
namespace roudi
{
class RouDiMultiProcess;

class RouDiEnvironment
{
  public:
    RouDiEnvironment(const RouDiConfig_t& roudiConfig = RouDiConfig_t().setDefaults(),
                     RouDiApp::MonitoringMode monitoringMode = RouDiApp::MonitoringMode::OFF);
    virtual ~RouDiEnvironment();

    RouDiEnvironment(RouDiEnvironment&& rhs);
    RouDiEnvironment& operator=(RouDiEnvironment&& rhs);

    RouDiEnvironment(const RouDiEnvironment&) = delete;
    RouDiEnvironment& operator=(const RouDiEnvironment&) = delete;

    void SetInterOpWaitingTime(const std::chrono::milliseconds& v);
    void InterOpWait();

    void CleanupAppResources(const std::string& name);

  protected:
    /// @brief for implementations on top of RouDiEnvironment
    RouDiEnvironment(RouDiMultiProcess* roudiApp);

  private:
    RuntimeTestInterface m_runtimes;
    std::chrono::milliseconds m_interOpWaitingTime = std::chrono::milliseconds(200);
    RouDiMultiProcess* m_roudiApp{nullptr};
};

} // namespace roudi
} // namespace iox

