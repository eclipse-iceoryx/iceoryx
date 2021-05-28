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

#include "iceoryx_hoofs/platform/named_pipe.hpp"

NamedPipe::NamedPipe(const std::string& name, uint64_t maxMessageSize, const uint64_t maxNumberOfMessages) noexcept
    : m_maxMessageSize{maxMessageSize}
{
    const DWORD inputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD outputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD openMode = PIPE_ACCESS_DUPLEX;
    const DWORD pipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    const DWORD noTimeout = 0;
    const LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    m_handle = CreateNamedPipeA(name.c_str(),
                                openMode,
                                pipeMode,
                                PIPE_UNLIMITED_INSTANCES,
                                outputBufferSize,
                                inputBufferSize,
                                noTimeout,
                                noSecurityAttributes);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        __PrintLastErrorToConsole("", "", 0);
        printf("Server CreateNamedPipe failed, GLE=%d.\n", GetLastError());
    }

    ConnectNamedPipe(m_handle, NULL);
}

NamedPipe::NamedPipe(NamedPipe&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipe::~NamedPipe() noexcept
{
    destroy();
}

NamedPipe& NamedPipe::operator=(NamedPipe&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();

        m_handle = rhs.m_handle;
        m_maxMessageSize = rhs.m_maxMessageSize;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }
    return *this;
}

void NamedPipe::destroy() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(m_handle);
        DisconnectNamedPipe(m_handle);
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

NamedPipe::operator bool() const noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}

std::optional<std::string> NamedPipe::receive() noexcept
{
    std::string message;
    message.resize(m_maxMessageSize);
    DWORD bytesRead;
    LPOVERLAPPED noOverlapping = NULL;
    if (ReadFile(m_handle, message.data(), message.size(), &bytesRead, noOverlapping))
    {
        message.resize(bytesRead);
        return std::make_optional(message);
    }

    return std::nullopt;
}
