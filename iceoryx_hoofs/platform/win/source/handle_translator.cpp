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

#include "iceoryx_hoofs/platform/handle_translator.hpp"

HandleTranslator& HandleTranslator::getInstance() noexcept
{
    static HandleTranslator globalHandleTranslator;
    return globalHandleTranslator;
}

HANDLE HandleTranslator::get(const int handle) const noexcept
{
    return m_handleList[static_cast<size_t>(handle)];
}

int HandleTranslator::add(HANDLE handle) noexcept
{
    for (int64_t limit = m_handleList.size(), k = 0; k < limit; ++k)
    {
        if (m_handleList[k] == nullptr)
        {
            m_handleList[k] = handle;
            return k;
        }
    }

    m_handleList.emplace_back(handle);
    return m_handleList.size() - 1;
}

void HandleTranslator::remove(const int handle) noexcept
{
    m_handleList[static_cast<uint64_t>(handle)] = nullptr;
}
