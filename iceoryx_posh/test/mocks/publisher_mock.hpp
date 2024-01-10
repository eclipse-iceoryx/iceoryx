// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_MOCKS_PUBLISHER_MOCK_HPP
#define IOX_POSH_MOCKS_PUBLISHER_MOCK_HPP

#include "iceoryx_posh/internal/popo/base_publisher.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iox/expected.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockPublisherPortUser
{
  public:
    using MemberType_t = iox::popo::PublisherPortData;
    MockPublisherPortUser() = default;
    MockPublisherPortUser(std::nullptr_t)
    {
    }
    MockPublisherPortUser(MemberType_t*){};

    MockPublisherPortUser(const MockPublisherPortUser& rhs [[maybe_unused]]){};
    MockPublisherPortUser(MockPublisherPortUser&& rhs [[maybe_unused]]){};
    MockPublisherPortUser& operator=(const MockPublisherPortUser& rhs [[maybe_unused]])
    {
        return *this;
    };
    MockPublisherPortUser& operator=(MockPublisherPortUser&& rhs [[maybe_unused]])
    {
        return *this;
    };
    iox::capro::ServiceDescription getCaProServiceDescription() const noexcept
    {
        return getServiceDescription();
    }
    MOCK_CONST_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD4(tryAllocateChunk,
                 iox::expected<iox::mepoo::ChunkHeader*, iox::popo::AllocationError>(
                     const uint64_t, const uint32_t, const uint32_t, const uint32_t));
    MOCK_METHOD1(releaseChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD1(sendChunk, void(iox::mepoo::ChunkHeader* const));
    MOCK_METHOD0(tryGetPreviousChunk, iox::optional<iox::mepoo::ChunkHeader*>());
    MOCK_METHOD0(offer, void());
    MOCK_METHOD0(stopOffer, void());
    MOCK_CONST_METHOD0(isOffered, bool());
    MOCK_CONST_METHOD0(hasSubscribers, bool());

    operator bool() const
    {
        return true;
    }

    MOCK_CONST_METHOD0(getUniqueID, iox::popo::UniquePortId());
    MOCK_METHOD0(destroy, void());
};

template <typename T>
class MockBasePublisher
{
  public:
    using PortType = MockPublisherPortUser;

    MockBasePublisher(const iox::capro::ServiceDescription&, const iox::popo::PublisherOptions&){};
    MOCK_CONST_METHOD0(getUid, iox::popo::uid_t());
    MOCK_CONST_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD0(offer, void(void));
    MOCK_METHOD0(stopOffer, void(void));
    MOCK_CONST_METHOD0(isOffered, bool(void));
    MOCK_CONST_METHOD0(hasSubscribers, bool(void));

    const MockPublisherPortUser& port() const noexcept
    {
        return m_port;
    }

    MockPublisherPortUser& port() noexcept
    {
        return m_port;
    }

    // for testing
    MockPublisherPortUser& mockPort() noexcept
    {
        return port();
    }

    MockPublisherPortUser m_port;
};

#endif // IOX_POSH_MOCKS_PUBLISHER_MOCK_HPP
