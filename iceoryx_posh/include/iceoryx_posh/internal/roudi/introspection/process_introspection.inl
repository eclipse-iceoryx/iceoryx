// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_INL
#define IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_INL

#include "process_introspection.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/posix_wrapper/thread.hpp"

#include <chrono>

namespace iox
{
namespace roudi
{
template <typename PublisherPort>
ProcessIntrospection<PublisherPort>::ProcessIntrospection()
    : m_runThread(false)
{
}

template <typename PublisherPort>
ProcessIntrospection<PublisherPort>::~ProcessIntrospection()
{
    stop();
    if (m_publisherPort.has_value())
    {
        m_publisherPort->stopOffer();
    }
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::addProcess(int f_pid, const ProcessName_t& f_name)
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

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::removeProcess(int f_pid)
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

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::addNode(const ProcessName_t& f_process, const NodeName_t& f_node)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    bool processFound = false;
    for (auto it_process = m_processList.begin(); it_process != m_processList.end(); ++it_process)
    {
        if (it_process->m_name == f_process)
        {
            processFound = true;
            bool alreadyInList = false;
            for (auto it_node = it_process->m_nodes.begin(); it_node != it_process->m_nodes.end(); ++it_node)
            {
                if (*it_node == f_node)
                {
                    LogWarn() << "Node " << f_node.c_str() << " already registered";
                    alreadyInList = true;
                }
            }
            if (!alreadyInList)
            {
                it_process->m_nodes.emplace_back(f_node);
            }
        }
    }
    if (!processFound)
    {
        LogWarn() << "Trying to register node " << f_node.c_str() << " but the related process is not registered";
    }
    m_processListNewData = true;
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::removeNode(const ProcessName_t& f_process, const NodeName_t& f_node)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    bool processFound = false;
    for (auto it_process = m_processList.begin(); it_process != m_processList.end(); ++it_process)
    {
        if (it_process->m_name == f_process)
        {
            processFound = true;
            bool removedFromList = false;
            for (auto it_node = it_process->m_nodes.begin(); it_node != it_process->m_nodes.end(); ++it_node)
            {
                if (*it_node == f_node)
                {
                    it_process->m_nodes.erase(it_node);
                    removedFromList = true;
                    break;
                }
            }
            if (!removedFromList)
            {
                LogWarn() << "Trying to remove node " << f_node.c_str() << " but it was not registered";
            }
        }
    }
    if (!processFound)
    {
        LogWarn() << "Trying to remove node " << f_node.c_str() << " but the related process is not registered";
    }
    m_processListNewData = true;
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::registerPublisherPort(PublisherPort&& publisherPort)
{
    // we do not want to call this twice
    if (!m_publisherPort.has_value())
    {
        m_publisherPort.emplace(std::move(publisherPort));
    }
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::run()
{
    // TODO: error handling for non debug builds
    cxx::Expects(m_publisherPort.has_value());

    // this is a field, there needs to be a sample before activate is called
    send();
    m_publisherPort->offer();

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
    posix::setThreadName(m_thread.native_handle(), "ProcessIntr");
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::send()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_processListNewData)
    {
        auto maybeChunkHeader = m_publisherPort->tryAllocateChunk(sizeof(ProcessIntrospectionFieldTopic));
        if (!maybeChunkHeader.has_error())
        {
            auto sample = static_cast<ProcessIntrospectionFieldTopic*>(maybeChunkHeader.value()->payload());
            new (sample) ProcessIntrospectionFieldTopic;

            for (auto& intrData : m_processList)
            {
                sample->m_processList.emplace_back(intrData);
            }
            m_processListNewData = false;

            m_publisherPort->sendChunk(maybeChunkHeader.value());
        }
    }
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::stop()
{
    m_runThread = false;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename PublisherPort>
void ProcessIntrospection<PublisherPort>::setSendInterval(unsigned int interval_ms)
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

#endif // IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_INL
