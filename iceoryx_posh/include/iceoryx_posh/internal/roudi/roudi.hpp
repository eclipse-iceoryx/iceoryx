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


#include <cstdint>
#include <thread>

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

namespace iox
{
namespace roudi
{
using namespace iox::units::duration_literals;

namespace systemd
{
class ISystemd
{
  public:
    virtual ~ISystemd() = default;
    ISystemd(ISystemd const& other) = delete;
    ISystemd(ISystemd&& other) = default;
    ISystemd& operator=(ISystemd const& other) = delete;
    ISystemd& operator=(ISystemd&& other) = default;

    virtual void processNotify() = 0;
    virtual void shutdown() = 0;

  protected:
    ISystemd() = default;
};

#ifdef USE_SYSTEMD
class SystemdServiceHandler final : public ISystemd
{
  public:
    SystemdServiceHandler() = default;
    SystemdServiceHandler(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler(SystemdServiceHandler&& other) = delete;
    SystemdServiceHandler& operator=(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler& operator=(SystemdServiceHandler&& other) = delete;

    ~SystemdServiceHandler() final
    {
        /*
         * This is necessary to prevent the main thread from exiting before
         * the 'listen_thread_watchdog' has finished, hence ensuring a
         * proper termination of the entire application.
         */
        if (m_listenThreadWatchdog.joinable())
        {
            m_listenThreadWatchdog.join();
        }
    }

    void shutdown() final
    {
        m_shutdown.store(true);
    }

    void processNotify() final
    {
        /*
         * We get information about how they are running. If as a unit, then we launch
         * watchdog and send a notification about the launch, otherwise we do nothing
         */
        iox::string<SIZE_STRING> invocationIdStr;
        auto const* const ENV_VAR = "INVOCATION_ID";
        invocationIdStr.unsafe_raw_access([&](auto* buffer, auto const info) {
            size_t actualSizeWithNull{0};
            auto result = IOX_POSIX_CALL(iox_getenv_s)(&actualSizeWithNull, buffer, info.total_size, ENV_VAR)
                              .failureReturnValue(-1)
                              .evaluate();

            if (result.has_error() && result.error().errnum == ERANGE)
            {
                IOX_LOG(ERROR, "Invalid value for 'INVOCATION_ID' environment variable!");
            }

            size_t actual_size{0};
            constexpr size_t NULL_TERMINATOR_SIZE{1};
            if (actualSizeWithNull > 0)
            {
                actual_size = actualSizeWithNull - NULL_TERMINATOR_SIZE;
            }
            buffer[actual_size] = 0;
            return actual_size;
        });

        if (!invocationIdStr.empty())
        {
            IOX_LOG(WARN, "Run APP in unit(systemd)");
            m_listenThreadWatchdog = std::thread([this] {
                bool status_change_name = iox::setThreadName("watchdog");
                if (!status_change_name)
                {
                    IOX_LOG(ERROR, "Can not set name for thread watchdog");
                    return;
                }
                auto resultReady = IOX_POSIX_CALL(sd_notify)(0, "READY=1").successReturnValue(1).evaluate();
                if (resultReady.has_error())
                {
                    IOX_LOG(ERROR,
                            "Failed to send READY=1 signal. Error: "
                                + resultReady.get_error().getHumanReadableErrnum());
                    return;
                }
                IOX_LOG(DEBUG, "WatchDog READY=1");

                IOX_LOG(INFO, "Start watchdog");
                while (!m_shutdown.load())
                {
                    auto resultWatchdog = IOX_POSIX_CALL(sd_notify)(0, "WATCHDOG=1").successReturnValue(1).evaluate();
                    if (resultWatchdog.has_error())
                    {
                        IOX_LOG(ERROR,
                                "Failed to send WATCHDOG=1 signal. Error: "
                                    + resultWatchdog.get_error().getHumanReadableErrnum());
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            });
        }
    }

  private:
    std::thread m_listenThreadWatchdog; // 8
  public:
    static constexpr uint16_t SIZE_STRING = 4096; // 2
  private:
    std::atomic_bool m_shutdown{false}; // 1
};
#else
class SystemdServiceHandler final : public ISystemd
{
  public:
    SystemdServiceHandler() = default;
    SystemdServiceHandler(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler(SystemdServiceHandler&& other) = default;
    SystemdServiceHandler& operator=(SystemdServiceHandler const& other) = delete;
    SystemdServiceHandler& operator=(SystemdServiceHandler&& other) = default;

    ~SystemdServiceHandler() final = default;
    void processNotify() final
    {
        // empty implementation
    }
    void shutdown() final
    {
        // empty implementation
    }
};
#endif
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
