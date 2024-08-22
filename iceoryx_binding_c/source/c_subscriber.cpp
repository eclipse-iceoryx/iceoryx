// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"
#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/assertions.hpp"
#include "iox/logging.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
}

/// @todo iox-#1221 remove this workaround needed for CycloneDDS due to our change to use the head for storage
struct SubscriberWithStoragePointer
{
    void* subscriberStorage{nullptr};
    cpp2c_Subscriber subscriber;
};

constexpr uint64_t SUBSCRIBER_OPTIONS_INIT_CHECK_CONSTANT = 543212345;

void iox_sub_options_init(iox_sub_options_t* options)
{
    if (options == nullptr)
    {
        IOX_LOG(WARN, "subscriber options initialization skipped - null pointer provided");
        return;
    }

    SubscriberOptions subscriberOptions;
    options->queueCapacity = subscriberOptions.queueCapacity;
    options->historyRequest = subscriberOptions.historyRequest;
    options->nodeName = nullptr;
    options->subscribeOnCreate = subscriberOptions.subscribeOnCreate;
    options->queueFullPolicy = cpp2c::queueFullPolicy(subscriberOptions.queueFullPolicy);
    options->requirePublisherHistorySupport = false;

    options->initCheck = SUBSCRIBER_OPTIONS_INIT_CHECK_CONSTANT;
}

bool iox_sub_options_is_initialized(const iox_sub_options_t* const options)
{
    return options && options->initCheck == SUBSCRIBER_OPTIONS_INIT_CHECK_CONSTANT;
}

iox_sub_t iox_sub_init(iox_sub_storage_t* self,
                       const char* const service,
                       const char* const instance,
                       const char* const event,
                       const iox_sub_options_t* const options)
{
    if (self == nullptr)
    {
        IOX_LOG(WARN, "subscriber initialization skipped - null pointer provided for iox_sub_storage_t");
        return nullptr;
    }

    SubscriberOptions subscriberOptions;

    // use default options otherwise
    if (options != nullptr)
    {
        if (!iox_sub_options_is_initialized(options))
        {
            // note that they may have been initialized but the initCheck
            // pattern overwritten afterwards, we cannot be sure but it is a misuse
            IOX_LOG(FATAL, "subscriber options may not have been initialized with iox_sub_init");
            IOX_REPORT_FATAL(CBindingError::BINDING_C__SUBSCRIBER_OPTIONS_NOT_INITIALIZED);
        }
        subscriberOptions.queueCapacity = options->queueCapacity;
        subscriberOptions.historyRequest = options->historyRequest;
        if (options->nodeName != nullptr)
        {
            subscriberOptions.nodeName = NodeName_t(TruncateToCapacity, options->nodeName);
        }
        subscriberOptions.subscribeOnCreate = options->subscribeOnCreate;
        subscriberOptions.queueFullPolicy = c2cpp::queueFullPolicy(options->queueFullPolicy);
        subscriberOptions.requiresPublisherHistorySupport = options->requirePublisherHistorySupport;
    }

    // this is required for CycloneDDS to limit the fallout of our change to use the heap for storage
    // it should be removed with #1221
    auto meWithStoragePointer = new SubscriberWithStoragePointer();
    meWithStoragePointer->subscriberStorage = self;
    auto me = &meWithStoragePointer->subscriber;
    auto ptrDiff = reinterpret_cast<size_t>(me) - reinterpret_cast<size_t>(meWithStoragePointer);
    IOX_ENFORCE(ptrDiff >= sizeof(void*), "Size mismatch for SubscriberWithStoragePointer!");
    IOX_ENFORCE(ptrDiff <= 2 * sizeof(void*), "Size mismatch for SubscriberWithStoragePointer!");

    me->m_portData =
        PoshRuntime::getInstance().getMiddlewareSubscriber(ServiceDescription{IdString_t(TruncateToCapacity, service),
                                                                              IdString_t(TruncateToCapacity, instance),
                                                                              IdString_t(TruncateToCapacity, event)},
                                                           subscriberOptions);

    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);
    return me;
}

void iox_sub_deinit(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    auto addressOfSelf = reinterpret_cast<uint64_t>(self);
    auto* selfWithStoragePointer = reinterpret_cast<SubscriberWithStoragePointer*>(addressOfSelf - sizeof(void*));

    delete selfWithStoragePointer;
}

void iox_sub_subscribe(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    SubscriberPortUser(self->m_portData).subscribe();
}

void iox_sub_unsubscribe(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    SubscriberPortUser(self->m_portData).unsubscribe();
}

iox_SubscribeState iox_sub_get_subscription_state(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return cpp2c::subscribeState(SubscriberPortUser(self->m_portData).getSubscriptionState());
}

iox_ChunkReceiveResult iox_sub_take_chunk(iox_sub_t const self, const void** const userPayload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    auto result = SubscriberPortUser(self->m_portData).tryGetChunk();
    if (result.has_error())
    {
        return cpp2c::chunkReceiveResult(result.error());
    }

    *userPayload = result.value()->userPayload();
    return ChunkReceiveResult_SUCCESS;
}

void iox_sub_release_chunk(iox_sub_t const self, const void* const userPayload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userPayload != nullptr, "'userPayload' must not be a 'nullptr'");
    SubscriberPortUser(self->m_portData).releaseChunk(ChunkHeader::fromUserPayload(userPayload));
}

void iox_sub_release_queued_chunks(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    SubscriberPortUser(self->m_portData).releaseQueuedChunks();
}

bool iox_sub_has_chunks(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return SubscriberPortUser(self->m_portData).hasNewChunks();
}

bool iox_sub_has_lost_chunks(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return SubscriberPortUser(self->m_portData).hasLostChunksSinceLastCall();
}

iox_service_description_t iox_sub_get_service_description(iox_sub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return TranslateServiceDescription(SubscriberPortUser(self->m_portData).getCaProServiceDescription());
}
