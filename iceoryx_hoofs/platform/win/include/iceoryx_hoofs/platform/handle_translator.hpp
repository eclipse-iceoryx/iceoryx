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
#ifndef IOX_HOOFS_WIN_PLATFORM_HANDLE_TRANSLATOR_HPP
#define IOX_HOOFS_WIN_PLATFORM_HANDLE_TRANSLATOR_HPP

#include "iceoryx_hoofs/platform/windows.hpp"
#include <vector>

class HandleTranslator
{
  public:
    static HandleTranslator& getInstance() noexcept;
    HANDLE get(const int handle) const noexcept;
    int add(HANDLE handle) noexcept;
    void remove(int handle) noexcept;

  private:
    HandleTranslator() noexcept = default;
    struct handle_t
    {
        HANDLE windowsHandle;
    };
    std::vector<handle_t> m_handleList;
};

#endif
