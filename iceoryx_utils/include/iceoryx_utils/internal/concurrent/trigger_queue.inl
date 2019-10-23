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

#include "iceoryx_utils/internal/concurrent/trigger_queue.hpp"

namespace iox
{
namespace concurrent
{
template <typename T, uint64_t CAPACITY>
cxx::optional<TriggerQueue<T, CAPACITY>> TriggerQueue<T, CAPACITY>::CreateTriggerQueue()
{
    cxx::optional<TriggerQueue<T, CAPACITY>> triggerQueue;
    triggerQueue.emplace();

    if (triggerQueue->m_isInitialized)
    {
        return triggerQueue;
    }
    else
    {
        return cxx::nullopt_t();
    }
}

template <typename T, uint64_t CAPACITY>
inline bool TriggerQueue<T, CAPACITY>::push(const T& in)
{
    if (stl_queue_push(in))
    {
        m_semaphore->post();
        return true;
    }
    else
    {
        return false;
    }
}

template <typename T, uint64_t CAPACITY>
inline bool TriggerQueue<T, CAPACITY>::blocking_pop(T& out)
{
    m_semaphore->wait();
    return stl_queue_pop(out);
}

template <typename T, uint64_t CAPACITY>
inline bool TriggerQueue<T, CAPACITY>::try_pop(T& out)
{
    if (!m_semaphore->tryWait())
    {
        return false;
    }

    return stl_queue_pop(out);
}

template <typename T, uint64_t CAPACITY>
inline bool TriggerQueue<T, CAPACITY>::empty()
{
    return m_queue->empty();
}

template <typename T, uint64_t CAPACITY>
inline uint64_t TriggerQueue<T, CAPACITY>::size()
{
    return m_queue->size();
}

template <typename T, uint64_t CAPACITY>
inline uint64_t TriggerQueue<T, CAPACITY>::capacity()
{
    return CAPACITY;
}

template <typename T, uint64_t CAPACITY>
inline void TriggerQueue<T, CAPACITY>::send_wakeup_trigger()
{
    m_semaphore->post();
}

/// @todo remove with lockfree fifo START
template <typename T, uint64_t CAPACITY>
inline bool TriggerQueue<T, CAPACITY>::stl_queue_pop(T& out)
{
    if (m_queue->empty())
    {
        return false;
    }

    auto guardedQueue = m_queue.GetScopeGuard();
    out = guardedQueue->front();
    guardedQueue->pop();
    return true;
}

template <typename T, uint64_t CAPACITY>
bool TriggerQueue<T, CAPACITY>::stl_queue_push(const T& in)
{
    auto guardedQueue = m_queue.GetScopeGuard();
    if (guardedQueue->size() >= CAPACITY)
    {
        return false;
    }
    guardedQueue->push(in);
    return true;
}
/// @todo remove with lockfree fifo END

} // namespace concurrent
} // namespace iox
