// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/modern_api/base_subscriber.hpp"
#include "iceoryx_posh/popo/modern_api/sample.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockSubscriberPortUser
{
  public:
    MockSubscriberPortUser() = default;
    MockSubscriberPortUser(std::nullptr_t)
    {
    }
    iox::capro::ServiceDescription getCaProServiceDescription() const noexcept
    {
        return getServiceDescription();
    }
    MOCK_CONST_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD1(subscribe, void(const uint64_t));
    MOCK_METHOD0(unsubscribe, void());
    MOCK_CONST_METHOD0(getSubscriptionState, iox::SubscribeState());
    MOCK_METHOD0(
        tryGetChunk,
        iox::cxx::expected<iox::cxx::optional<const iox::mepoo::ChunkHeader*>, iox::popo::ChunkReceiveError>());
    MOCK_METHOD1(releaseChunk, void(iox::mepoo::ChunkHeader*));
    MOCK_METHOD0(releaseQueuedChunks, void());
    MOCK_CONST_METHOD0(hasNewChunks, bool());
    MOCK_METHOD0(hasLostChunksSinceLastCall, bool());
    MOCK_METHOD1(setConditionVariable, bool(iox::popo::ConditionVariableData*));
    MOCK_METHOD0(unsetConditionVariable, bool());
    MOCK_METHOD0(isConditionVariableSet, bool());
};

template <typename T>
class MockBaseSubscriber
{
  public:
    MockBaseSubscriber(const iox::capro::ServiceDescription&){};
    MOCK_CONST_METHOD0(getUid, iox::popo::uid_t());
    MOCK_CONST_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD1(subscribe, void(uint64_t));
    MOCK_CONST_METHOD0(getSubscriptionState, iox::SubscribeState());
    MOCK_METHOD0(unsubscribe, void());
    MOCK_CONST_METHOD0(hasNewSamples, bool());
    MOCK_METHOD0(hasMissedSamples, bool());
    MOCK_METHOD0_T(take,
                   iox::cxx::expected<iox::cxx::optional<iox::popo::Sample<const T>>, iox::popo::ChunkReceiveError>());
    MOCK_METHOD0(releaseQueuedSamples, void());
    MOCK_METHOD1(setConditionVariable, bool(iox::popo::ConditionVariableData*));
    MOCK_METHOD0(unsetConditionVariable, bool(void));
    MOCK_METHOD0(hasTriggered, bool(void));
};
