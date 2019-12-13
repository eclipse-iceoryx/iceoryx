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

#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/internal/roudi/roudi_multi_process.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/log/logmanager.hpp"

namespace iox
{
namespace roudi
{
RouDiEnvironment::RouDiEnvironment(RouDiMultiProcess* roudiApp)
{
    iox::log::LogManager::GetLogManager().SetDefaultLogLevel(iox::log::LogLevel::kWarn);
    m_roudiApp = roudiApp;
}

RouDiEnvironment::RouDiEnvironment(const RouDiConfig_t& roudiConfig, RouDiApp::MonitoringMode monitoringMode)
    : RouDiEnvironment(new RouDiMultiProcess(monitoringMode, false, roudiConfig))
{
}

RouDiEnvironment::~RouDiEnvironment()
{
    m_runtimes.cleanupRuntimes();
    delete m_roudiApp;
}

RouDiEnvironment::RouDiEnvironment(RouDiEnvironment&& rhs)
    : m_runtimes(std::move(rhs.m_runtimes))
{
    this->m_roudiApp = rhs.m_roudiApp;
    rhs.m_roudiApp = nullptr;
    this->m_interOpWaitingTime = rhs.m_interOpWaitingTime;
}
RouDiEnvironment& RouDiEnvironment::operator=(RouDiEnvironment&& rhs)
{
    m_runtimes = std::move(rhs.m_runtimes);
    this->m_roudiApp = rhs.m_roudiApp;
    rhs.m_roudiApp = nullptr;
    this->m_interOpWaitingTime = rhs.m_interOpWaitingTime;

    return *this;
}

void RouDiEnvironment::SetInterOpWaitingTime(const std::chrono::milliseconds& v)
{
    m_interOpWaitingTime = v;
}

void RouDiEnvironment::InterOpWait()
{
    std::this_thread::sleep_for(m_interOpWaitingTime);
}

void RouDiEnvironment::CleanupAppResources(const std::string& name)
{
    m_runtimes.eraseRuntime(name);
}

} // namespace roudi
} // namespace iox
