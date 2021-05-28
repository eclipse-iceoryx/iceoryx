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
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"

NamedPipeReceiver::NamedPipeReceiver(const std::string& name,
                                     uint64_t maxMessageSize,
                                     const uint64_t maxNumberOfMessages) noexcept
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
        printf("Server CreateNamedPipeReceiver failed for: %s.\n", name.c_str());
        return;
    }

    ConnectNamedPipe(m_handle, NULL);
}

NamedPipeReceiver::NamedPipeReceiver(NamedPipeReceiver&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipeReceiver::~NamedPipeReceiver() noexcept
{
    destroy();
}

NamedPipeReceiver& NamedPipeReceiver::operator=(NamedPipeReceiver&& rhs) noexcept
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

void NamedPipeReceiver::destroy() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(m_handle);
        DisconnectNamedPipe(m_handle);
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

NamedPipeReceiver::operator bool() const noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}

std::optional<std::string> NamedPipeReceiver::receive() noexcept
{
    if (!*this)
    {
        return std::nullopt;
    }

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

NamedPipeSender::NamedPipeSender(const std::string& name, const uint64_t timeoutInMs) noexcept
{
    DWORD disableSharing = 0;
    LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    DWORD defaultAttributes = 0;
    HANDLE noTemplateFile = NULL;
    m_handle = CreateFileA(name.c_str(),
                           GENERIC_READ | GENERIC_WRITE,
                           disableSharing,
                           noSecurityAttributes,
                           OPEN_EXISTING,
                           defaultAttributes,
                           noTemplateFile);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            __PrintLastErrorToConsole("", "", 0);
            printf("Could not open pipe %s for reading.\n", name.c_str());
            return;
        }

        if (timeoutInMs != 0)
        {
            if (!WaitNamedPipeA(name.c_str(), timeoutInMs))
            {
                __PrintLastErrorToConsole("", "", 0);
                printf("Unable to wait %d ms for pipe %s.\n", timeoutInMs, name.c_str());
                return;
            }
            else
            {
                m_handle = CreateFileA(name.c_str(),
                                       GENERIC_READ | GENERIC_WRITE,
                                       disableSharing,
                                       noSecurityAttributes,
                                       OPEN_EXISTING,
                                       defaultAttributes,
                                       noTemplateFile);
                if (m_handle == INVALID_HANDLE_VALUE)
                {
                    return;
                }
            }
        }
    }

    DWORD pipeMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(m_handle, &pipeMode, NULL, NULL))
    {
        __PrintLastErrorToConsole("", "", 0);
        printf("SetNamedPipeHandleState failed for pipe %s\n", name.c_str());
        destroy();
    }
}

NamedPipeSender::NamedPipeSender(NamedPipeSender&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipeSender::~NamedPipeSender() noexcept
{
    destroy();
}

NamedPipeSender& NamedPipeSender::operator=(NamedPipeSender&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();

        m_handle = rhs.m_handle;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }
    return *this;
}

NamedPipeSender::operator bool() const noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}

bool NamedPipeSender::send(const std::string& message) noexcept
{
    DWORD numberOfSentBytes = 0;
    if (!WriteFile(m_handle, message.data(), message.size(), &numberOfSentBytes, NULL))
    {
        printf("Unable to send message: %s\n", message.c_str());
        return false;
    }
    else if (numberOfSentBytes != message.size())
    {
        printf("Could only send the first %ld bytes of the message: %s\n", numberOfSentBytes, message.c_str());
        return false;
    }
    return true;
}

void NamedPipeSender::destroy() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}
