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

NamedPipeReceiverInstance::NamedPipeReceiverInstance(const std::string& name,
                                                     uint64_t maxMessageSize,
                                                     const uint64_t maxNumberOfMessages) noexcept
    : m_maxMessageSize{maxMessageSize}
{
    auto pipeName = generatePipePathName(name);
    const DWORD inputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD outputBufferSize = maxMessageSize * maxNumberOfMessages;
    const DWORD openMode = PIPE_ACCESS_DUPLEX;
    const DWORD pipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    const DWORD noTimeout = 0;
    const LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    m_handle = CreateNamedPipeA(pipeName.c_str(),
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
        printf("Server CreateNamedPipeReceiverInstance failed for: %s.\n", pipeName.c_str());
        return;
    }

    ConnectNamedPipe(m_handle, NULL);
}

NamedPipeReceiverInstance::NamedPipeReceiverInstance(NamedPipeReceiverInstance&& rhs) noexcept
{
    *this = std::move(rhs);
}

NamedPipeReceiverInstance::~NamedPipeReceiverInstance() noexcept
{
    destroy();
}

NamedPipeReceiverInstance& NamedPipeReceiverInstance::operator=(NamedPipeReceiverInstance&& rhs) noexcept
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

void NamedPipeReceiverInstance::destroy() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        Win32Call(FlushFileBuffers, m_handle);
        Win32Call(DisconnectNamedPipe, m_handle);
        Win32Call(CloseHandle, m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

NamedPipeReceiverInstance::operator bool() const noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}

std::optional<std::string> NamedPipeReceiverInstance::receive() noexcept
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
    auto pipeName = generatePipePathName(name);
    DWORD disableSharing = 0;
    LPSECURITY_ATTRIBUTES noSecurityAttributes = NULL;
    DWORD defaultAttributes = 0;
    HANDLE noTemplateFile = NULL;
    m_handle = Win32Call(CreateFileA,
                         pipeName.c_str(),
                         GENERIC_READ | GENERIC_WRITE,
                         disableSharing,
                         noSecurityAttributes,
                         OPEN_EXISTING,
                         defaultAttributes,
                         noTemplateFile)
                   .value;

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            printf("Could not open pipe %s for reading.\n", pipeName.c_str());
            return;
        }

        if (timeoutInMs != 0)
        {
            if (!Win32Call(WaitNamedPipeA, pipeName.c_str(), timeoutInMs).value)
            {
                printf("Unable to wait %d ms for pipe %s.\n", timeoutInMs, pipeName.c_str());
                return;
            }
            else
            {
                m_handle = Win32Call(CreateFileA,
                                     pipeName.c_str(),
                                     GENERIC_READ | GENERIC_WRITE,
                                     disableSharing,
                                     noSecurityAttributes,
                                     OPEN_EXISTING,
                                     defaultAttributes,
                                     noTemplateFile)
                               .value;
                if (m_handle == INVALID_HANDLE_VALUE)
                {
                    return;
                }
            }
        }
    }

    DWORD pipeMode = PIPE_READMODE_MESSAGE;
    if (!Win32Call(SetNamedPipeHandleState, m_handle, &pipeMode, static_cast<LPDWORD>(NULL), static_cast<LPDWORD>(NULL))
             .value)
    {
        printf("SetNamedPipeHandleState failed for pipe %s\n", pipeName.c_str());
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
    if (!Win32Call(
             WriteFile, m_handle, message.data(), message.size(), &numberOfSentBytes, static_cast<LPOVERLAPPED>(NULL))
             .value)
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
        Win32Call(CloseHandle, m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

NamedPipeReceiver::NamedPipeReceiver(const std::string& name,
                                     uint64_t maxMessageSize,
                                     const uint64_t maxNumberOfMessages) noexcept
    : m_pipeName{name}
    , m_maxMessageSize{maxMessageSize}
    , m_maxNumberOfMessages{maxNumberOfMessages}
{
    m_pipeInstances.resize(m_maxNumberOfMessages);
    m_keepRunning.store(true);
    m_receiveThread = std::thread(&NamedPipeReceiver::receiveLoop, this);
}

NamedPipeReceiver::~NamedPipeReceiver() noexcept
{
    if (m_keepRunning.load() == true)
    {
        m_keepRunning.store(false);
        m_receiveThread.join();
    }
}

std::optional<std::string> NamedPipeReceiver::receive() noexcept
{
    return timedReceive(0U);
}

std::optional<std::string> NamedPipeReceiver::timedReceive(const uint64_t timeoutInMs) noexcept
{
    int64_t remainingTimeInMs = static_cast<int64_t>(timeoutInMs);
    int64_t minimumRetries = 10;
    do
    {
        {
            std::lock_guard<std::mutex> lock(m_receivedMessagesMutex);
            if (!m_receivedMessages.empty())
            {
                std::string msg = m_receivedMessages.front();
                m_receivedMessages.pop();
                return std::make_optional(msg);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_receiveLoopIntervalInMs));
        remainingTimeInMs -= m_receiveLoopIntervalInMs;
        --minimumRetries;
    } while (remainingTimeInMs > 0 || minimumRetries > 0);

    return std::nullopt;
}

void NamedPipeReceiver::receiveLoop() noexcept
{
    while (m_keepRunning.load())
    {
        for (auto& pipe : m_pipeInstances)
        {
            if (!pipe)
            {
                pipe = NamedPipeReceiverInstance(m_pipeName, m_maxMessageSize, m_maxNumberOfMessages);
            }

            auto message = pipe.receive();
            if (message)
            {
                {
                    std::lock_guard<std::mutex> lock(m_receivedMessagesMutex);
                    if (m_receivedMessages.size() >= m_maxNumberOfMessages)
                    {
                        m_receivedMessages.pop();
                    }
                    m_receivedMessages.push(message->c_str());
                }

                pipe = NamedPipeReceiverInstance(m_pipeName, m_maxMessageSize, m_maxNumberOfMessages);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_receiveLoopIntervalInMs));
    }

    m_pipeInstances.clear();
}

std::string generatePipePathName(const std::string& name) noexcept
{
    return "\\\\.\\pipe\\" + name;
}
