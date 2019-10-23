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

#include <functional>
#include <thread>

#include "iceoryx_utils/internal/concurrent/trigger_queue.hpp"

namespace iox
{
namespace concurrent
{
class ActiveObject
{
  protected:
    ActiveObject();
    virtual ~ActiveObject();
    void addTask(const std::function<void()> f);
    void mainLoop();
    bool isInitialized() const;
    void stopRunning();

    friend class cxx::optional<ActiveObject>;

  private:
    static constexpr uint32_t taskQueueSize = 128;
    using taskQueue_t = concurrent::TriggerQueue<std::function<void()>, taskQueueSize>;

    cxx::optional<taskQueue_t> m_tasks = taskQueue_t::CreateTriggerQueue();
    bool m_isInitialized = m_tasks.has_value();

    bool m_keepRunning{true};
    std::thread m_mainLoopThread;
};
} // namespace concurrent
} // namespace iox
