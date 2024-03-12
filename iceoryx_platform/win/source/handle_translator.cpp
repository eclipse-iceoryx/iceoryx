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

#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/logging.hpp"

#include <iostream>
#include <sstream>

constexpr int HandleTranslator::INVALID_LINUX_FD;

HandleTranslator& HandleTranslator::getInstance() noexcept
{
    static HandleTranslator globalHandleTranslator;
    return globalHandleTranslator;
}

HANDLE HandleTranslator::get(const int linuxFd) const noexcept
{
    if (linuxFd == INVALID_LINUX_FD)
    {
        return INVALID_HANDLE_VALUE;
    }

    std::lock_guard<std::mutex> lock(m_mtx);

    auto iter = m_linuxToWindows.find(linuxFd);
    if (iter == m_linuxToWindows.end())
    {
        return INVALID_HANDLE_VALUE;
    }

    return iter->second;
}

int HandleTranslator::add(HANDLE windowsHandle) noexcept
{
    if (windowsHandle == INVALID_HANDLE_VALUE)
    {
        return INVALID_LINUX_FD;
    }

    std::lock_guard<std::mutex> lock(m_mtx);

    int linuxFd = INVALID_LINUX_FD;
    if (!m_freeFileDescriptors.empty())
    {
        linuxFd = m_freeFileDescriptors.front();
        m_freeFileDescriptors.pop();
    }
    else
    {
        linuxFd = m_currentLinuxFileHandle;
        ++m_currentLinuxFileHandle;
    }

    m_linuxToWindows[linuxFd] = windowsHandle;
    return linuxFd;
}

void HandleTranslator::remove(const int linuxFd) noexcept
{
    if (linuxFd == INVALID_LINUX_FD)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mtx);

    auto iter = m_linuxToWindows.find(linuxFd);
    if (iter == m_linuxToWindows.end())
    {
        std::stringstream stream;
        stream << "Unable to release not registered file handle " << linuxFd << " since it was not acquired";
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, stream.str().c_str());
        return;
    }

    m_linuxToWindows.erase(iter);
    m_freeFileDescriptors.emplace(linuxFd);
}
