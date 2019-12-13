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

#include "process_introspection.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"

#include <chrono>

namespace iox
{
namespace roudi
{
template <typename SenderPort>
ProcessIntrospection<SenderPort>::ProcessIntrospection()
    : m_runThread(false)
{
}

template <typename SenderPort>
ProcessIntrospection<SenderPort>::~ProcessIntrospection()
{
    stop();
    if (m_senderPort)
    {
        m_senderPort.deactivate(); // stop offer
    }
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::addProcess(int f_pid, const cxx::CString100& f_name)
{
    ProcessIntrospectionData procIntrData;
    procIntrData.m_pid = f_pid;
    procIntrData.m_name = f_name;

    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_processList.push_back(procIntrData);
        m_processListNewData = true;
    }
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::removeProcess(int f_pid)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    for (auto it = m_processList.begin(); it != m_processList.end(); ++it)
    {
        if (it->m_pid == f_pid)
        {
            m_processList.erase(it);
            break;
        }
    }
    m_processListNewData = true;
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::addRunnable(const cxx::CString100& f_process, const cxx::CString100& f_runnable)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    bool processFound = false;
    for (auto it_process = m_processList.begin(); it_process != m_processList.end(); ++it_process)
    {
        if (it_process->m_name == f_process)
        {
            processFound = true;
            bool alreadyInList = false;
            for (auto it_runnable = it_process->m_runnables.begin(); it_runnable != it_process->m_runnables.end();
                 ++it_runnable)
            {
                if (*it_runnable == f_runnable)
                {
                    LogWarn() << "Runnable " << f_runnable.to_cstring() << " already registered";
                    alreadyInList = true;
                }
            }
            if (!alreadyInList)
            {
                it_process->m_runnables.emplace_back(f_runnable);
            }
        }
    }
    if (!processFound)
    {
        LogWarn() << "Trying to register runnable " << f_runnable.to_cstring()
                  << " but the related process is not registered";
    }
    m_processListNewData = true;
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::removeRunnable(const cxx::CString100& f_process,
                                                      const cxx::CString100& f_runnable)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    bool processFound = false;
    for (auto it_process = m_processList.begin(); it_process != m_processList.end(); ++it_process)
    {
        if (it_process->m_name == f_process)
        {
            processFound = true;
            bool removedFromList = false;
            for (auto it_runnable = it_process->m_runnables.begin(); it_runnable != it_process->m_runnables.end();
                 ++it_runnable)
            {
                if (*it_runnable == f_runnable)
                {
                    it_process->m_runnables.erase(it_runnable);
                    removedFromList = true;
                    break;
                }
            }
            if (!removedFromList)
            {
                LogWarn() << "Trying to remove runnable " << f_runnable.to_cstring() << " but it was not registered";
            }
        }
    }
    if (!processFound)
    {
        LogWarn() << "Trying to remove runnable " << f_runnable.to_cstring()
                  << " but the related process is not registered";
    }
    m_processListNewData = true;
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::registerSenderPort(SenderPort&& f_senderPort)
{
    // we do not want to call this twice
    if (!m_senderPort)
    {
        m_senderPort = std::move(f_senderPort);
        m_senderPort.enableDoDeliverOnSubscription();
    }
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::run()
{
    // TODO: error handling for non debug builds
    cxx::Expects(m_senderPort);

    // this is a field, there needs to be a sample before activate is called
    send();
    m_senderPort.activate(); // corresponds to offer

    m_runThread = true;
    static uint32_t ct = 0;
    m_thread = std::thread([this] {
        while (m_runThread.load(std::memory_order_relaxed))
        {
            if (0 == (ct % m_sendIntervalCount))
            {
                send();
            }
            ++ct;

            std::this_thread::sleep_for(m_sendIntervalSleep);
        }
    });

    // set thread name
    pthread_setname_np(m_thread.native_handle(), "ProcessIntr");
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::send()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_processListNewData)
    {
        auto chunkHeader = m_senderPort.reserveChunk(sizeof(ProcessIntrospectionFieldTopic));
        auto sample = static_cast<ProcessIntrospectionFieldTopic*>(chunkHeader->payload());
        new (sample) ProcessIntrospectionFieldTopic;

        for (auto& intrData : m_processList)
        {
            sample->m_processList.emplace_back(intrData);
        }
        m_processListNewData = false;

        m_senderPort.deliverChunk(chunkHeader);
    }
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::stop()
{
    m_runThread = false;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename SenderPort>
void ProcessIntrospection<SenderPort>::setSendInterval(unsigned int interval_ms)
{
    if (std::chrono::milliseconds(interval_ms) >= m_sendIntervalSleep)
    {
        m_sendIntervalCount = static_cast<unsigned int>(std::chrono::milliseconds(interval_ms) / m_sendIntervalSleep);
    }
    else
    {
        m_sendIntervalCount = 1;
    }
}


} // namespace roudi
} // namespace iox
