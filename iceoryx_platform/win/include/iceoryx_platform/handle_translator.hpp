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
#ifndef IOX_HOOFS_WIN_PLATFORM_HANDLE_TRANSLATOR_HPP
#define IOX_HOOFS_WIN_PLATFORM_HANDLE_TRANSLATOR_HPP

#include "iceoryx_platform/windows.hpp"
#include <map>
#include <mutex>
#include <queue>

/// @brief In windows file handles have the type HANDLE (void*) in linux it is
///        usually an int. To establish the portability we keep track of the
///        windows handles and assign them a unique int so that they can be used
///        in a platform independent manner.
///        This class translates a windows handle of type HANDLE to its linux
///        file handle int pendant.
class HandleTranslator
{
  public:
    static constexpr int INVALID_LINUX_FD = -1;

    HandleTranslator(const HandleTranslator&) = delete;
    HandleTranslator(HandleTranslator&&) = delete;
    HandleTranslator& operator=(const HandleTranslator&) = delete;
    HandleTranslator& operator=(HandleTranslator&&) = delete;
    ~HandleTranslator() = default;

    static HandleTranslator& getInstance() noexcept;
    HANDLE get(const int linuxFd) const noexcept;
    int add(HANDLE windowsHandle) noexcept;
    void remove(const int linuxFd) noexcept;

  private:
    HandleTranslator() noexcept = default;

    int m_currentLinuxFileHandle = 0;
    mutable std::mutex m_mtx;
    std::map<int, HANDLE> m_linuxToWindows;
    std::queue<int> m_freeFileDescriptors;
};

#endif
