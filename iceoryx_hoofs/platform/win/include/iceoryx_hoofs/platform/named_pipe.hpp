// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_NAMED_PIPE_HPP
#define IOX_HOOFS_WIN_PLATFORM_NAMED_PIPE_HPP

#include "iceoryx_hoofs/platform/windows.hpp"

#include <cstdint>
#include <optional>
#include <string>

struct NamedPipeReceiver
{
    NamedPipeReceiver() noexcept = default;
    NamedPipeReceiver(const std::string& name, uint64_t maxMessageSize, const uint64_t maxNumberOfMessages) noexcept;
    NamedPipeReceiver(const NamedPipeReceiver&) = delete;
    NamedPipeReceiver(NamedPipeReceiver&& rhs) noexcept;
    ~NamedPipeReceiver() noexcept;

    NamedPipeReceiver& operator=(const NamedPipeReceiver& rhs) = delete;
    NamedPipeReceiver& operator=(NamedPipeReceiver&& rhs) noexcept;

    operator bool() const noexcept;

    std::optional<std::string> receive() noexcept;

  private:
    void destroy() noexcept;

    HANDLE m_handle = INVALID_HANDLE_VALUE;
    uint64_t m_maxMessageSize = 0U;
};

class NamedPipeSender
{
  public:
    NamedPipeSender() noexcept = default;
    NamedPipeSender(const std::string& name, const uint64_t timeoutInMs) noexcept;

    NamedPipeSender(const NamedPipeSender& rhs) = delete;
    NamedPipeSender(NamedPipeSender&& rhs) noexcept;
    ~NamedPipeSender() noexcept;

    NamedPipeSender& operator=(const NamedPipeSender& rhs) = delete;
    NamedPipeSender& operator=(NamedPipeSender&& rhs) noexcept;

    operator bool() const noexcept;

    bool send(const std::string& message) noexcept;
    HANDLE m_handle = INVALID_HANDLE_VALUE;

  private:
    void destroy() noexcept;
};

#endif
