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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_HPP
#define IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/list.hpp"

#include <atomic>
#include <mutex>
#include <thread>

namespace iox
{
namespace roudi
{
/// @brief This class handles the process intropection for RouDi.
///        It is recommended to use the ProcessIntrospectionType alias which sets
///        the intended template parameter.
///        The class tracks the adding and removal of processes and sends it to
///        the introspection client if subscribed.
template <typename PublisherPort>
class ProcessIntrospection
{
  public:
    ProcessIntrospection();
    ~ProcessIntrospection();

    // delete copy constructor and assignment operator
    ProcessIntrospection(ProcessIntrospection const&) = delete;
    ProcessIntrospection& operator=(ProcessIntrospection const&) = delete;
    // delete move constructor and assignment operator
    ProcessIntrospection(ProcessIntrospection&&) = delete;
    ProcessIntrospection& operator=(ProcessIntrospection&&) = delete;


    /// @brief This function is used to add a process to the process introspection
    /// @param[in] f_pid is the PID of the process to add
    /// @param[in] f_name is the name of the process
    void addProcess(int f_pid, const ProcessName_t& f_name);

    /// @brief This function is used to remove process to the process introspection
    /// @param[in] f_pid is the PID of the process to remove
    void removeProcess(int f_pid);

    /// @brief This function is used to add a node to the process introspection
    /// @param[in] f_processName is the name of the proces
    /// @param[in] f_nodeName is the name of the node to add
    void addNode(const ProcessName_t& f_process, const NodeName_t& f_node);

    /// @brief This function is used to remove a node to the process introspection
    /// @param[in] f_processName is the name of the proces
    /// @param[in] f_nodeName is the name of the node to remove
    void removeNode(const ProcessName_t& f_process, const NodeName_t& f_node);

    /// @brief This functions registers the POSH publisher port which is used
    ///        to send the data to the instrospcetion client
    /// @param publisherPort is the publisher port for transmission
    void registerPublisherPort(PublisherPort&& publisherPort);

    /// @brief This function starts a thread which periodically sends
    ///        the introspection data to the client. The send interval
    ///        can be set by @ref setSendInterval "setSendInterval(...)".
    ///        Before this function is called, the publisher port hast to be
    ///        registered with @ref registerPublisherPort "registerPublisherPort()".
    void run();

    /// @brief This function stops the thread previously started by @ref run "run()"
    void stop();

    /// @brief This function configures the interval for the transmission of the
    ///        port introspection data.
    /// @param[in] interval_ms is the interval time in milliseconds
    void setSendInterval(unsigned int interval_ms);

  protected:
    cxx::optional<PublisherPort> m_publisherPort;
    void send();

  private:
    using ProcessList_t = cxx::list<ProcessIntrospectionData, MAX_PROCESS_NUMBER>;
    ProcessList_t m_processList;
    bool m_processListNewData{true}; // true because we want to have a valid field, even with an empty list

    std::atomic<bool> m_runThread;
    std::thread m_thread;
    std::mutex m_mutex;

    unsigned int m_sendIntervalCount{10};
    const std::chrono::milliseconds m_sendIntervalSleep{100};
};

/// @brief typedef for the templated process introspection class that is used by RouDi for the
/// actual process introspection functionality.
using ProcessIntrospectionType = ProcessIntrospection<PublisherPortUserType>;

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/introspection/process_introspection.inl"

#endif // IOX_POSH_ROUDI_INTROSPECTION_PROCESS_INTROSPECTION_HPP
