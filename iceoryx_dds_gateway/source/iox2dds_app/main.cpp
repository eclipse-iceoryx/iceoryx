// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include <atomic>

#include <iceoryx_posh/runtime/posh_runtime.hpp>
#include <iceoryx_utils/cxx/helplets.hpp>
#include <iceoryx_utils/cxx/optional.hpp>
#include <ioxdds/gateway/iox2dds.hpp>

class ShutdownManager
{
  public:
    static void scheduleShutdown(int num)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        char reason;
        psignal(num, &reason);
        m_shutdownFlag.store(true, std::memory_order_relaxed);
    }
    static bool shouldShutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_shutdownFlag.load(std::memory_order_relaxed);
    }

  private:
    static std::mutex m_mutex;
    static std::atomic_bool m_shutdownFlag;
    ShutdownManager() = default;
};
std::mutex ShutdownManager::m_mutex;
std::atomic_bool ShutdownManager::m_shutdownFlag(false);

int main(int argc, char* argv[])
{
    // Set OS signal handlers
    signal(SIGINT, ShutdownManager::scheduleShutdown);
    signal(SIGTERM, ShutdownManager::scheduleShutdown);

    // Start application
    iox::runtime::PoshRuntime::getInstance("/gateway_iox2dds");

    iox::gateway::dds::Iceoryx2DDSGateway<> gateway;
    auto discoveryThread = std::thread([&gateway] { gateway.discoveryLoop(); });
    auto forwardingThread = std::thread([&gateway] { gateway.forwardingLoop(); });

    // Run until SIGINT or SIGTERM
    while (!ShutdownManager::shouldShutdown())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    // Shutdown gracefully
    gateway.shutdown();
    discoveryThread.join();
    forwardingThread.join();

    return 0;
}
