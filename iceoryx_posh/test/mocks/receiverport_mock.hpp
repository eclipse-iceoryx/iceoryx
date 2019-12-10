// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "test.hpp"

#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port_data.hpp"
#include "iceoryx_posh/mepoo/chunk_info.hpp"

class ReceiverPort_MOCK
{
  public:
      using MemberType_t = iox::popo::ReceiverPortData;

    ReceiverPort_MOCK()
    {
    }

    ReceiverPort_MOCK(MemberType_t*)
    {
    }

    ReceiverPort_MOCK(ReceiverPort_MOCK&&) = default;
    ReceiverPort_MOCK& operator=(ReceiverPort_MOCK&&) = default;
    ReceiverPort_MOCK(const ReceiverPort_MOCK&) = default;
    ReceiverPort_MOCK& operator=(const ReceiverPort_MOCK&) = default;

    MOCK_METHOD0(getCaProMessage, iox::cxx::optional<iox::capro::CaproMessage>());
    MOCK_METHOD1(getCaProMessage, iox::cxx::optional<iox::capro::CaproMessage>(iox::capro::CaproMessage));
    MOCK_METHOD0(cleanup, void());

    MOCK_METHOD0(cyclicServiceUpdate, void());

    MOCK_METHOD1(subscribe_impl, void(const uint32_t));
    void subscribe(const bool f_autoResubscribe = false, const uint32_t f_deliverySize = iox::MAX_RECEIVER_QUEUE_SIZE)
    {
        (void)f_autoResubscribe;
        subscribe_impl(f_deliverySize);
    }
    void subscribe(const uint32_t f_deliverySize = iox::MAX_RECEIVER_QUEUE_SIZE)
    {
        subscribe_impl(f_deliverySize);
    }

    MOCK_METHOD0(unsubscribe, void());
    MOCK_CONST_METHOD0(isSubscribed, bool());
    MOCK_CONST_METHOD0(getSubscribeState, iox::SubscribeState());
    MOCK_METHOD1(releaseSample, bool(const iox::mepoo::ChunkInfo*));
    MOCK_METHOD1(getChunk, bool(iox::mepoo::SharedChunk&));
    MOCK_METHOD0(newData, bool());
    MOCK_METHOD0(clearDeliveryFiFo, void());
    MOCK_METHOD0(UnsetCallbackReferences, void());
    MOCK_METHOD0(GetShmSemaphore, iox::posix::Semaphore*());
    MOCK_METHOD1(deliver, bool(iox::mepoo::SharedChunk));

    MOCK_CONST_METHOD0(getDeliveryFiFoSize, uint64_t());
    MOCK_CONST_METHOD0(getSubscriptionState, uint32_t());
    MOCK_CONST_METHOD0(getDeliveryFiFoCapacity, uint32_t());
    MOCK_CONST_METHOD0(getCaProServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD0(AreCallbackReferencesSet, bool());
};
