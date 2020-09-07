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
#ifndef IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP
#define IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP

#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/internal/roudi/roudi_lock.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"

namespace iox
{
namespace roudi
{
struct IceOryxRouDiComponents
{
  public:
    IceOryxRouDiComponents(const RouDiConfig_t& roudiConfig)
        : m_rouDiMemoryManager(roudiConfig)
    {
    }
    virtual ~IceOryxRouDiComponents() = default;

    /// @brief Locks the socket for preventing multiple start of RouDi
    RouDiLock m_roudilock;

    /// @brief Handles MemoryProvider and MemoryBlocks
    IceOryxRouDiMemoryManager m_rouDiMemoryManager;

    /// @todo Make this an object
    /// @brief Cleanup resources and initalize memory
    cxx::GenericRAII m_initalizer{
        [this]() {
            // this temporary object will create a roudi mqueue and close it immediatelly
            // if there was an outdated roudi message queue, it will be cleaned up
            // if there is an outdated mqueue, the start of the apps will be terminated
            runtime::MqBase::cleanupOutdatedMessageQueue(MQ_ROUDI_NAME);

            m_rouDiMemoryManager.createAndAnnounceMemory().or_else([](RouDiMemoryManagerError error) {
                LogFatal() << "Could not create SharedMemory! Error: " << static_cast<uint64_t>(error);
                errorHandler(Error::kROUDI_COMPONENTS__SHARED_MEMORY_UNAVAILABLE, nullptr, iox::ErrorLevel::FATAL);
            });
            return true;
        },
        []() {}};

    /// @brief Handles the ports in shared memory
    PortManager m_portManager{&m_rouDiMemoryManager};
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP
