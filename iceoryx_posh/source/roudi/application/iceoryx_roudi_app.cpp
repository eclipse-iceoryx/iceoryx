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

#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

namespace iox
{
namespace roudi
{
IceOryxRouDiApp::IceOryxRouDiApp(const CmdLineParser& cmdLineParser, const RouDiConfig_t& roudiConfig) noexcept
    : RouDiApp(cmdLineParser, roudiConfig)
{
}

void IceOryxRouDiApp::run() noexcept
{
    if (m_run)
    {
        static cxx::optional<IceOryxRouDiComponents> m_rouDiComponents;
        auto componentsScopeGuard = cxx::makeScopedStatic(m_rouDiComponents, m_config);

        static cxx::optional<RouDi> roudi;
        auto roudiScopeGuard = cxx::makeScopedStatic(roudi,
                                                     m_rouDiComponents.value().m_rouDiMemoryManager,
                                                     m_rouDiComponents.value().m_portManager,
                                                     m_monitoringMode,
                                                     true,
                                                     RouDi::MQThreadStart::IMMEDIATE,
                                                     m_compatibilityCheckLevel);
        waitForSignal();
    }
}
} // namespace roudi
} // namespace iox
