// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_MOCKS_BASE_PORT_MOCK_HPP
#define IOX_POSH_MOCKS_BASE_PORT_MOCK_HPP

#include "iceoryx_posh/internal/popo/ports/base_port.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockBasePort
{
  public:
    using MemberType_t = iox::popo::BasePortData;
    explicit MockBasePort(MemberType_t*)
    {
    }
    MockBasePort() = default;
    ~MockBasePort() = default;

    MockBasePort(const MockBasePort&) = delete;
    MockBasePort& operator=(const MockBasePort&) = delete;
    MockBasePort(MockBasePort&&) noexcept
    {
    }
    MockBasePort& operator=(MockBasePort&&) noexcept
    {
        return *this;
    }

    MOCK_METHOD(const iox::capro::ServiceDescription&, getCaProServiceDescription, (), (const, noexcept));
    MOCK_METHOD(const iox::RuntimeName_t&, getRuntimeName, (), (const, noexcept));
    MOCK_METHOD(iox::popo::UniquePortId, getUniqueID, (), (const, noexcept));
    MOCK_METHOD(const iox::NodeName_t&, getNodeName, (), (const, noexcept));
    MOCK_METHOD(void, destroy, (), (noexcept));
    MOCK_METHOD(bool, toBeDestroyed, (), (const, noexcept));

    explicit operator bool() const
    {
        return true;
    }
};

#endif // IOX_POSH_MOCKS_BASE_PORT_MOCK_HPP
