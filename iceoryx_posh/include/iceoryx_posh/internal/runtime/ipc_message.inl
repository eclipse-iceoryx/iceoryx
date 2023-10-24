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
#ifndef IOX_POSH_RUNTIME_IPC_MESSAGE_INL
#define IOX_POSH_RUNTIME_IPC_MESSAGE_INL

#include "iceoryx_posh/internal/runtime/ipc_message.hpp"

namespace iox
{
namespace runtime
{
template <typename T>
void IpcMessage::addEntry(const T& entry) noexcept
{
    std::stringstream newEntry;
    newEntry << entry;

    if (!isValidEntry(newEntry.str()))
    {
        IOX_LOG(ERROR, "\'" << newEntry.str().c_str() << "\' is an invalid IPC channel entry");
        m_isValid = false;
    }
    else
    {
        m_msg.append(newEntry.str() + m_separator);
        ++m_numberOfElements;
    }
}

template <typename T>
IpcMessage& IpcMessage::operator<<(const T& entry) noexcept
{
    addEntry(entry);
    return *this;
}

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_MESSAGE_INL
