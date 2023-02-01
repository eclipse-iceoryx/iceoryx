// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace roudi
{
RouDiEnvironment::RouDiEnvironment(BaseCTor, const uint16_t uniqueRouDiId)
{
    // setUniqueRouDiId is called multiple times but it is okay for the tests
    auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>([](auto, auto) {});
    iox::popo::UniquePortId::setUniqueRouDiId(uniqueRouDiId);
}

RouDiEnvironment::RouDiEnvironment(const RouDiConfig_t& roudiConfig,
                                   const roudi::MonitoringMode monitoringMode,
                                   const uint16_t uniqueRouDiId)
    : RouDiEnvironment(BaseCTor::BASE, uniqueRouDiId)
{
    m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(roudiConfig));
    m_roudiApp = std::unique_ptr<RouDi>(new RouDi(m_roudiComponents->rouDiMemoryManager,
                                                  m_roudiComponents->portManager,
                                                  RouDi::RoudiStartupParameters{monitoringMode, false}));
}

RouDiEnvironment::~RouDiEnvironment()
{
    if (m_runtimes.m_doCleanupOnDestruction)
    {
        // setUniqueRouDiId is called multiple times but it is okay for the tests
        auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>([](auto, auto) {});
        popo::UniquePortId::setUniqueRouDiId(roudi::DEFAULT_UNIQUE_ROUDI_ID);
    }
    CleanupRuntimes();
}

void RouDiEnvironment::SetInterOpWaitingTime(const std::chrono::milliseconds& v)
{
    m_interOpWaitingTime = v;
}

void RouDiEnvironment::InterOpWait()
{
    std::this_thread::sleep_for(m_interOpWaitingTime);
}

void RouDiEnvironment::CleanupAppResources(const RuntimeName_t& name)
{
    m_runtimes.eraseRuntime(name);
}

void RouDiEnvironment::CleanupRuntimes()
{
    m_runtimes.cleanupRuntimes();
}

} // namespace roudi
} // namespace iox
