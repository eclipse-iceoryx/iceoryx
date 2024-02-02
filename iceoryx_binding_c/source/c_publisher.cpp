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

#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"
#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/assertions.hpp"
#include "iox/logging.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/publisher.h"
}

constexpr uint64_t PUBLISHER_OPTIONS_INIT_CHECK_CONSTANT = 123454321;

void iox_pub_options_init(iox_pub_options_t* options)
{
    if (options == nullptr)
    {
        IOX_LOG(WARN, "publisher options initialization skipped - null pointer provided");
        return;
    }

    PublisherOptions publisherOptions;
    options->historyCapacity = publisherOptions.historyCapacity;
    options->nodeName = nullptr;
    options->offerOnCreate = publisherOptions.offerOnCreate;
    options->subscriberTooSlowPolicy = cpp2c::consumerTooSlowPolicy(publisherOptions.subscriberTooSlowPolicy);

    options->initCheck = PUBLISHER_OPTIONS_INIT_CHECK_CONSTANT;
}

bool iox_pub_options_is_initialized(const iox_pub_options_t* const options)
{
    return options && options->initCheck == PUBLISHER_OPTIONS_INIT_CHECK_CONSTANT;
}

iox_pub_t iox_pub_init(iox_pub_storage_t* self,
                       const char* const service,
                       const char* const instance,
                       const char* const event,
                       const iox_pub_options_t* const options)
{
    if (self == nullptr)
    {
        IOX_LOG(WARN, "publisher initialization skipped - null pointer provided for iox_pub_storage_t");
        return nullptr;
    }

    PublisherOptions publisherOptions;

    // use default options otherwise
    if (options != nullptr)
    {
        if (!iox_pub_options_is_initialized(options))
        {
            // note that they may have been initialized but the initCheck
            // pattern overwritten afterwards, we cannot be sure but it is a misuse
            IOX_LOG(FATAL, "publisher options may not have been initialized with iox_pub_options_init");
            IOX_REPORT_FATAL(CBindingError::BINDING_C__PUBLISHER_OPTIONS_NOT_INITIALIZED);
        }
        publisherOptions.historyCapacity = options->historyCapacity;
        if (options->nodeName != nullptr)
        {
            publisherOptions.nodeName = NodeName_t(TruncateToCapacity, options->nodeName);
        }
        publisherOptions.offerOnCreate = options->offerOnCreate;
        publisherOptions.subscriberTooSlowPolicy = c2cpp::consumerTooSlowPolicy(options->subscriberTooSlowPolicy);
    }

    auto* me = new cpp2c_Publisher();
    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);

    me->m_portData = PoshRuntime::getInstance().getMiddlewarePublisher(
        ServiceDescription{
            IdString_t(TruncateToCapacity, service),
            IdString_t(TruncateToCapacity, instance),
            IdString_t(TruncateToCapacity, event),
        },
        publisherOptions);
    return me;
}

void iox_pub_deinit(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    self->m_portData->m_toBeDestroyed.store(true, std::memory_order_relaxed);
    delete self;
}

iox_AllocationResult iox_pub_loan_chunk(iox_pub_t const self, void** const userPayload, const uint64_t userPayloadSize)
{
    return iox_pub_loan_aligned_chunk_with_user_header(self,
                                                       userPayload,
                                                       userPayloadSize,
                                                       IOX_C_CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
                                                       IOX_C_CHUNK_NO_USER_HEADER_SIZE,
                                                       IOX_C_CHUNK_NO_USER_HEADER_ALIGNMENT);
}

iox_AllocationResult iox_pub_loan_aligned_chunk(iox_pub_t const self,
                                                void** const userPayload,
                                                const uint64_t userPayloadSize,
                                                const uint32_t userPayloadAlignment)
{
    return iox_pub_loan_aligned_chunk_with_user_header(self,
                                                       userPayload,
                                                       userPayloadSize,
                                                       userPayloadAlignment,
                                                       IOX_C_CHUNK_NO_USER_HEADER_SIZE,
                                                       IOX_C_CHUNK_NO_USER_HEADER_ALIGNMENT);
}

iox_AllocationResult iox_pub_loan_aligned_chunk_with_user_header(iox_pub_t const self,
                                                                 void** const userPayload,
                                                                 const uint64_t userPayloadSize,
                                                                 const uint32_t userPayloadAlignment,
                                                                 const uint32_t userHeaderSize,
                                                                 const uint32_t userHeaderAlignment)
{
    auto result = PublisherPortUser(self->m_portData)
                      .tryAllocateChunk(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment)
                      .and_then([&userPayload](ChunkHeader* h) { *userPayload = h->userPayload(); });
    if (result.has_error())
    {
        return cpp2c::allocationResult(result.error());
    }

    return AllocationResult_SUCCESS;
}

void iox_pub_release_chunk(iox_pub_t const self, void* const userPayload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userPayload != nullptr, "'userPayload' must not be a 'nullptr'");
    PublisherPortUser(self->m_portData).releaseChunk(ChunkHeader::fromUserPayload(userPayload));
}

void iox_pub_publish_chunk(iox_pub_t const self, void* const userPayload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userPayload != nullptr, "'userPayload' must not be a 'nullptr'");
    PublisherPortUser(self->m_portData).sendChunk(ChunkHeader::fromUserPayload(userPayload));
}

void iox_pub_offer(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    PublisherPortUser(self->m_portData).offer();
}

void iox_pub_stop_offer(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    PublisherPortUser(self->m_portData).stopOffer();
}

bool iox_pub_is_offered(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return PublisherPortUser(self->m_portData).isOffered();
}

bool iox_pub_has_subscribers(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return PublisherPortUser(self->m_portData).hasSubscribers();
}

iox_service_description_t iox_pub_get_service_description(iox_pub_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return TranslateServiceDescription(PublisherPortUser(self->m_portData).getCaProServiceDescription());
}
