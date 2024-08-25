// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP
#define IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP

#include "iceoryx_platform/file.hpp"
#include "iceoryx_platform/stdlib.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/introspection/mempool_introspection.hpp"
#include "iceoryx_posh/internal/roudi/process_manager.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_config.hpp"
#include "iox/posix_user.hpp"
#include "iox/relative_pointer.hpp"
#include "iox/scope_guard.hpp"
#include "iox/smart_lock.hpp"


#include <condition_variable>
#include <cstdint>
#include <thread>

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#ifdef USE_SYSTEMD
namespace iox::roudi::systemd
{
class SystemdServiceHandler;
} // namespace iox::roudi::systemd
using SendMessageStatusApplication = iox::roudi::systemd::SystemdServiceHandler;
#else
namespace iox::roudi::systemd
{
class NoSystemdServiceHandler;
} // namespace iox::roudi::systemd
using SendMessageStatusApplication = iox::roudi::systemd::NoSystemdServiceHandler;
#endif

namespace iox
{
namespace roudi
{
using namespace iox::units::duration_literals;

namespace systemd
{
/**
 * @brief Interface class for systemd service handling
 *
 **/
class ISystemd
{
  public:
    virtual ~ISystemd() = default;
    ISystemd(ISystemd const& other) = delete;
    ISystemd(ISystemd&& other) = default;
    ISystemd& operator=(ISystemd const& other) = delete;
    ISystemd& operator=(ISystemd&& other) = default;

    /// dbus signal handler
    virtual void processNotify() = 0;
    /// Sets a shutdown flag
    virtual void shutdown() = 0;

  protected:
    ISystemd() = default;
};

/**
 * @brief Class to handle systemd service notifications
 *
 **/
class SystemdServiceHandler final : public ISystemd
{
  private:
    std::condition_variable watchdogNotifyCondition; ///< watch dog notification condition // 48
    std::mutex watchdogMutex;                        ///< watch dog mutex // 40
    std::thread m_listenThreadWatchdog;              ///< thread that listens to systemd watchdog signals // 8
  public:
    static constexpr const uint16_t SIZE_STRING = 4096;   ///< maximum size of string // 2
    static constexpr const uint8_t SIZE_THREAD_NAME = 15; ///< max size for thread name // 1
  private:
    std::atomic_bool m_shutdown{false}; ///< indicates if service is being shutdown // 1

  public:
    SystemdServiceHandler() = default;
    SystemdServiceHandler(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler(SystemdServiceHandler&& other) = delete;
    SystemdServiceHandler& operator=(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler& operator=(SystemdServiceHandler&& other) = delete;

    /**
     * @brief Destructor joins the listenThreadWatchdog if it is still joinable, to ensure a proper termination
     **/
    ~SystemdServiceHandler() final;

    /**
     * @brief Sets the shutdown flag to true, causing the systemd handler to stop.
     **/
    void shutdown() final;

    /**
     * @brief Fetch required environment variable as a string
     * @param env_var Pointer to environment variable
     * @return Environment variable as std::string
     **/
    static std::string getEnvironmentVariable(const char* const env_var);

    /**
     * @brief Helper function to set thread name
     * @param threadName Thread name to be set
     * @return True if successfully set, otherwise false
     **/
    static bool setThreadNameHelper(iox::string<SIZE_THREAD_NAME>& threadName);

#ifdef USE_SYSTEMD
    /**
     * @brief Helper function to send SDNotify signals
     * @param state SDNotify state to be sent
     * @return True if signal sending is successful, otherwise false
     **/
    static bool sendSDNotifySignalHelper(const std::string_view state)
    {
        auto result = IOX_POSIX_CALL(sd_notify)(0, state.data()).successReturnValue(1).evaluate();
        if (result.has_error())
        {
            IOX_LOG(ERROR,
                    "Failed to send " << state.data()
                                      << " signal. Error: " << result.get_error().getHumanReadableErrnum());
            return false;
        }
        return true;
    }
#else
    static bool sendSDNotifySignalHelper([[maybe_unused]] const std::string_view state)
    {
        // empty implementation
        return true;
    }
#endif
    /**
     * @brief Function to manage the watchdog loop
     **/
    void watchdogLoopHelper();

    /**
     * @brief Method to process systemd notification logic
     **/
    void processNotify() final;
};

/**
 * @brief Empty implementation handler for non-systemd systems
 *
 **/
class NoSystemdServiceHandler final : public ISystemd
{
  public:
    NoSystemdServiceHandler() = default;
    NoSystemdServiceHandler(NoSystemdServiceHandler const& other) = delete;
    NoSystemdServiceHandler(NoSystemdServiceHandler&& other) = default;
    NoSystemdServiceHandler& operator=(NoSystemdServiceHandler const& other) = delete;
    NoSystemdServiceHandler& operator=(NoSystemdServiceHandler&& other) = default;

    /**
     * @brief Empty implementation of destructor
     **/
    ~NoSystemdServiceHandler() final = default;

    /**
     * @brief Empty implementation of processNotify
     **/
    void processNotify() final
    {
        // empty implementation
    }

    /**
     * @brief Empty implementation of shutdown
     **/
    void shutdown() final
    {
        // empty implementation
    }
};
} // namespace systemd

class RouDi
{
  public:
    RouDi& operator=(const RouDi& other) = delete;
    RouDi(const RouDi& other) = delete;

    RouDi(RouDiMemoryInterface& roudiMemoryInterface,
          PortManager& portManager,
          const config::RouDiConfig& roudiConfig) noexcept;

    virtual ~RouDi() noexcept;

    /// @brief Triggers the discovery loop to run immediately instead of waiting for the next tick interval
    /// @param[in] timeout is the time to wait to unblock the function call in case the discovery loop never signals to
    /// have finished the run
    void triggerDiscoveryLoopAndWaitToFinish(units::Duration timeout) noexcept;

  protected:
    /// @brief Starts the thread processing messages from the runtimes
    /// Once this is done, applications can register and Roudi is fully operational.
    void startProcessRuntimeMessagesThread() noexcept;

    /// @brief Stops threads and kills all process known to RouDi
    /// Called in d'tor
    ///
    /// @note Intentionally not virtual to be able to call it in derived class
    void shutdown() noexcept;
    virtual void processMessage(const runtime::IpcMessage& message,
                                const iox::runtime::IpcMessageType& cmd,
                                const RuntimeName_t& runtimeName) noexcept;
    virtual void cyclicUpdateHook() noexcept;
    void IpcMessageErrorHandler() noexcept;

    version::VersionInfo parseRegisterMessage(const runtime::IpcMessage& message,
                                              uint32_t& pid,
                                              iox_uid_t& userId,
                                              int64_t& transmissionTimestamp) noexcept;

    /// @brief Handles the registration request from process
    /// @param [in] name of the process which wants to register at roudi; this is equal to the IPC channel name
    /// @param [in] pid is the host system process id
    /// @param [in] user is the posix user id to which the process belongs
    /// @param [in] transmissionTimestamp is an ID for the application to check for the expected response
    /// @param [in] sessionId is an ID generated by RouDi to prevent sending outdated IPC channel transmission
    /// @param [in] versionInfo Version of iceoryx used
    void registerProcess(const RuntimeName_t& name,
                         const uint32_t pid,
                         const PosixUser user,
                         const int64_t transmissionTimestamp,
                         const uint64_t sessionId,
                         const version::VersionInfo& versionInfo) noexcept;

    /// @brief Creates a unique ID which can be used to check outdated IPC channel transmissions
    /// @return a unique, monotonic and consecutive increasing number
    static uint64_t getUniqueSessionIdForProcess() noexcept;

  private:
    void processRuntimeMessages(runtime::IpcInterfaceCreator&& roudiIpcInterface) noexcept;

    void monitorAndDiscoveryUpdate() noexcept;

    ScopeGuard m_unregisterRelativePtr{[] { UntypedRelativePointer::unregisterAll(); }};
    const config::RouDiConfig m_roudiConfig;
    std::atomic_bool m_runMonitoringAndDiscoveryThread;
    std::atomic_bool m_runHandleRuntimeMessageThread;

    popo::UserTrigger m_discoveryLoopTrigger;
    optional<UnnamedSemaphore> m_discoveryFinishedSemaphore;

    const units::Duration m_runtimeMessagesThreadTimeout{100_ms};

  protected:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    /// @note destroy the memory right at the end of the dTor, since the memory is not needed anymore and we know that
    /// the lifetime of the MemoryBlocks must be at least as long as RouDi; this saves us from issues if the
    /// RouDiMemoryManager outlives some MemoryBlocks
    ScopeGuard m_roudiMemoryManagerCleaner{[this]() {
        if (this->m_roudiMemoryInterface->destroyMemory().has_error())
        {
            IOX_LOG(WARN, "unable to cleanup roudi memory interface");
        };
    }};
    PortManager* m_portManager{nullptr};
    concurrent::smart_lock<ProcessManager> m_prcMgr;

  private:
    std::thread m_monitoringAndDiscoveryThread;
    std::thread m_handleRuntimeMessageThread;

  protected:
    ProcessIntrospectionType m_processIntrospection;
    MemPoolIntrospectionType m_mempoolIntrospection;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP
