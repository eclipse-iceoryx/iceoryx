// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "iox/optional.hpp"
#include "iox/scoped_static.hpp"
#include "iox/signal_watcher.hpp"

namespace iox
{
namespace roudi
{
IceOryxRouDiApp::IceOryxRouDiApp(const IceoryxConfig& config) noexcept
    : RouDiApp(config)
{
}

uint8_t IceOryxRouDiApp::run() noexcept
{
    if (m_run)
    {
        static optional<IceOryxRouDiComponents> m_rouDiComponents;
        auto componentsScopeGuard = makeScopedStatic(m_rouDiComponents, m_config);

        static optional<RouDi> roudi;
        auto roudiScopeGuard = makeScopedStatic(
            roudi, m_rouDiComponents.value().rouDiMemoryManager, m_rouDiComponents.value().portManager, m_config);
        iox::waitForTerminationRequest();
    }
    return EXIT_SUCCESS;
}
} // namespace roudi
} // namespace iox
