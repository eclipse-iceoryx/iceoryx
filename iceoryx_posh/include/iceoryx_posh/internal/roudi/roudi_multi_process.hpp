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

#include <cstdio>
#include <sys/file.h>
#include <thread>

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/roudi/introspection/mempool_introspection.hpp"
#include "iceoryx_posh/internal/roudi/roudi_lock.hpp"
#include "iceoryx_posh/internal/roudi/roudi_process.hpp"
#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include "ac3log/simplelogger.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class RouDiMultiProcess
{
  public:
    RouDiMultiProcess& operator=(const RouDiMultiProcess& other) = delete;
    RouDiMultiProcess(const RouDiMultiProcess& other) = delete;

    RouDiMultiProcess(RouDiApp::MonitoringMode f_monitoringMode = RouDiApp::MonitoringMode::ON,
                      const bool f_killProcessesInDestructor = true,
                      const RouDiConfig_t f_config = RouDiConfig_t().setDefaults());

    virtual ~RouDiMultiProcess();

  protected:
    /// @brief Stops threads and kills all process known to RouDi
    /// Called in d'tor
    ///
    /// @note Intentionally not virtual to be able to call it in derived class
    void shutdown();
    virtual void processMessage(const runtime::MqMessage& f_data,
                                const iox::runtime::MqMessageType& f_cmd,
                                const std::string& f_name);
    virtual void cyclicUpdateHook();
    void mqMessageErrorHandler();

    void parseRegisterMessage(const runtime::MqMessage& f_message,
                              int& f_pid,
                              uid_t& f_userId,
                              int64_t& f_transmissionTimestamp);
    bool registerProcess(const std::string& f_name, int f_pid, posix::PosixUser f_user, int64_t transmissionTimestamp);

  private:
    void mqThread();

    void processThread();

    /// cleanup mqueue, etc.
    bool cleanupBeforeStart();

    //----member

    cxx::GenericRAII m_unregisterRelativePtr{[] {}, [] { RelativePointer::unregisterAll(); }};
    bool m_killProcessesInDestructor;
    std::atomic_bool m_runThreads;

    const uint32_t m_MessageQueueTimeoutMilliseconds = 100;

    /// locks the socket for preventing multiple start of RouDi
    RouDiLock m_roudilock;

    /// dummy variable to call cleanupBeforeStart after getting the roudi lock
    bool m_cleanupBeforeStart;

  protected:
    SharedMemoryManager m_shmMgr;
    ProcessManager m_prcMgr;

  private:
    std::thread m_processManagementThread;
    std::thread m_processMQThread;

  protected:
    ProcessIntrospectionType m_processIntrospection;
    MemPoolIntrospectionType m_mempoolIntrospection;

  private:
    RouDiApp::MonitoringMode m_monitoringMode{RouDiApp::MonitoringMode::ON};
};

} // namespace roudi
} // namespace iox
