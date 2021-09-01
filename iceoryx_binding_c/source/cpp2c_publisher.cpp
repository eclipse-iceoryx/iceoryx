// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"

cpp2c_Publisher::cpp2c_Publisher(cpp2c_Publisher&& rhs) noexcept
{
    *this = std::move(rhs);
}

cpp2c_Publisher::~cpp2c_Publisher() noexcept
{
    if (m_portData)
    {
        iox::popo::PublisherPortUser(m_portData).destroy();
    }
}

cpp2c_Publisher& cpp2c_Publisher::operator=(cpp2c_Publisher&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (m_portData)
        {
            iox::popo::PublisherPortUser(m_portData).destroy();
        }

        m_portData = rhs.m_portData;
        rhs.m_portData = nullptr;
    }
    return *this;
}
