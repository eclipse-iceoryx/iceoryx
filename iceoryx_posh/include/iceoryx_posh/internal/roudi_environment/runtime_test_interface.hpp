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

#include <atomic>
#include <map>
#include <mutex>

namespace iox
{
namespace runtime
{
class PoshRuntime;
}
namespace roudi
{
class RuntimeTestInterface
{
    friend class RouDiEnvironment;

  private:
    bool m_doCleanupOnDestruction{true};

    thread_local static runtime::PoshRuntime* t_activeRuntime;
    thread_local static std::atomic<uint64_t> t_currentRouDiContext;
    static std::atomic<uint64_t> s_currentRouDiContext;

    static std::mutex s_runtimeAccessMutex;

    static std::map<std::string, runtime::PoshRuntime*> s_runtimes;

    /// This is a replacement for the PoshRuntime::GetInstance factory method
    /// @param [in] f_name ist the name of the runtime
    /// @return a reference to a PoshRuntime
    /// @note The runtime is stored in a vector and a thread local storage.
    ///
    ///       In a multithreaded environment each thread has its own runtime. This means that for each thread
    ///       iox::runtime::PoshRuntime::GetInstance(...) must be called. Threads that call GetInstance(...)
    ///       with the same name, share the same runtime.
    ///
    ///       It is also possible to use multiple runtimes in a singlethreaded environment. There are some points to
    ///       take care of, though.  There are some classes that call PoshRuntime::GetInstance() without a
    ///       parameter. In this case the already created runtime is used. In the context of the roudi environment this
    ///       means that the active runtime is used. The active runtime is the one from the latest
    ///       iox::runtime::PoshRuntimeImp::GetInstance(...) call with a parameter.
    ///       Places where a GetInstance() call without parameter happens are:
    ///         - constructors of Publisher, Subscriber and GatewayGeneric
    ///         - FindService, OfferService and StopOfferService
    ///       This means that iox::runtime::PoshRuntimeImp::GetInstance(...) must be called before the above classes
    ///       are created or functions are called, to make the correct runtime active.
    static runtime::PoshRuntime& runtimeFactoryGetInstance(const std::string& f_name);

  public:
    RuntimeTestInterface(RuntimeTestInterface&& rhs);
    RuntimeTestInterface& operator=(RuntimeTestInterface&& rhs);

    RuntimeTestInterface(const RuntimeTestInterface&) = delete;
    RuntimeTestInterface& operator=(const RuntimeTestInterface&) = delete;

  private:
    RuntimeTestInterface();
    ~RuntimeTestInterface();

    void cleanupRuntimes();

    void eraseRuntime(const std::string& f_name);
};

} // namespace roudi
} // namespace iox

