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

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_posh/internal/roudi/roudi_multi_process.hpp"

namespace iox
{
namespace roudi
{
IceOryxRouDiApp::IceOryxRouDiApp(int argc, char* argv[], const RouDiConfig_t& config) noexcept
    : RouDiApp(argc, argv, config)
{
}

void IceOryxRouDiApp::run() noexcept
{
    if (m_run)
    {
        static cxx::optional<RouDiMultiProcess> roudi;
        auto cleaner = cxx::makeScopedStatic(roudi, m_monitoringMode, true, m_config);

        waitToFinish();
    }
}

} // namespace roudi
} // namespace iox
