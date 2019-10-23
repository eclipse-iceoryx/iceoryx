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

#include "iceoryx_utils/internal/concurrent/active_object.hpp"

namespace iox
{
namespace concurrent
{
ActiveObject::ActiveObject()
    : m_mainLoopThread(&ActiveObject::mainLoop, this)
{
}

ActiveObject::~ActiveObject()
{
    stopRunning();
}

void ActiveObject::addTask(const std::function<void()> f)
{
    m_tasks->push(f);
}

void ActiveObject::mainLoop()
{
    while (m_keepRunning)
    {
        std::function<void()> task;
        if (m_tasks->blocking_pop(task) && task)
        {
            task();
        }
    }
}

bool ActiveObject::isInitialized() const
{
    return m_isInitialized;
}

void ActiveObject::stopRunning()
{
    if (m_mainLoopThread.joinable())
    {
        addTask([=] { this->m_keepRunning = false; });
        m_mainLoopThread.join();
    }
}
} // namespace concurrent
} // namespace iox
