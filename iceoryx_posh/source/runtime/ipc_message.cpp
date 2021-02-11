// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/ipc_message.hpp"

#include <algorithm>

namespace iox
{
namespace runtime
{
const char IpcMessage::m_separator = ',';

IpcMessage::IpcMessage(const std::initializer_list<std::string>& msg) noexcept
{
    for (auto element : msg)
    {
        addEntry(element);
    }
}

IpcMessage::IpcMessage(const std::string& msg) noexcept
{
    setMessage(msg);
}

uint32_t IpcMessage::getNumberOfElements() const noexcept
{
    return m_numberOfElements;
}

std::string IpcMessage::getElementAtIndex(const uint32_t index) const noexcept
{
    std::string messageRemainder(m_msg);
    size_t startPos = 0u;
    size_t endPos = messageRemainder.find_first_of(m_separator, startPos);

    for (uint32_t counter = 0u; endPos != std::string::npos; ++counter)
    {
        if (counter == index)
        {
            return messageRemainder.substr(startPos, endPos - startPos);
        }

        startPos = endPos + 1u;
        endPos = messageRemainder.find_first_of(m_separator, startPos);
    }

    return std::string();
}

bool IpcMessage::isValidEntry(const std::string& entry) const noexcept
{
    if (entry.find(m_separator) != std::string::npos)
    {
        return false;
    }
    return true;
}

bool IpcMessage::isValid() const noexcept
{
    return m_isValid;
}

std::string IpcMessage::getMessage() const noexcept
{
    return m_msg;
}

void IpcMessage::setMessage(const std::string& msg) noexcept
{
    clearMessage();

    m_msg = msg;
    if (!m_msg.empty() && m_msg.back() != m_separator)
    {
        m_isValid = false;
    }
    else
    {
        m_numberOfElements =
            static_cast<uint32_t>(std::count_if(m_msg.begin(), m_msg.end(), [&](char c) { return c == m_separator; }));
    }
}

void IpcMessage::clearMessage() noexcept
{
    m_msg.clear();
    m_numberOfElements = 0u;
    m_isValid = true;
}

bool IpcMessage::operator==(const IpcMessage& rhs) const noexcept
{
    return this->getMessage() == rhs.getMessage();
}

} // namespace runtime
} // namespace iox
