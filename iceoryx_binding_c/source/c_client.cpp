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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iox/assertions.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;
using namespace iox::capro;

extern "C" {
#include "iceoryx_binding_c/client.h"
}

#include <cstring>

constexpr uint64_t CLIENT_OPTIONS_INIT_CHECK_CONSTANT = 47113130815;

void iox_client_options_init(iox_client_options_t* options)
{
    IOX_ENFORCE(options != nullptr, "'options' must not be a 'nullptr'");

    ClientOptions clientOptions;
    options->responseQueueCapacity = clientOptions.responseQueueCapacity;
    strncpy(options->nodeName, clientOptions.nodeName.c_str(), IOX_CONFIG_NODE_NAME_SIZE);
    options->connectOnCreate = clientOptions.connectOnCreate;
    options->responseQueueFullPolicy = cpp2c::queueFullPolicy(clientOptions.responseQueueFullPolicy);
    options->serverTooSlowPolicy = cpp2c::consumerTooSlowPolicy(clientOptions.serverTooSlowPolicy);

    options->initCheck = CLIENT_OPTIONS_INIT_CHECK_CONSTANT;
}

bool iox_client_options_is_initialized(const iox_client_options_t* const options)
{
    IOX_ENFORCE(options != nullptr, "'options' must not be a 'nullptr'");

    return options->initCheck == CLIENT_OPTIONS_INIT_CHECK_CONSTANT;
}

iox_client_t iox_client_init(iox_client_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_client_options_t* const options)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(service != nullptr, "'service' must not be a 'nullptr'");
    IOX_ENFORCE(instance != nullptr, "'instance' must not be a 'nullptr'");
    IOX_ENFORCE(event != nullptr, "'event' must not be a 'nullptr'");
    IOX_ENFORCE(options == nullptr || (options != nullptr && iox_client_options_is_initialized(options)),
                "'options' must be either a 'nullptr' or the data behind the pointer must be initialized");

    ClientOptions clientOptions;
    if (options != nullptr)
    {
        clientOptions.responseQueueCapacity = options->responseQueueCapacity;
        clientOptions.nodeName = iox::NodeName_t(TruncateToCapacity, options->nodeName);
        clientOptions.connectOnCreate = options->connectOnCreate;
        clientOptions.responseQueueFullPolicy = c2cpp::queueFullPolicy(options->responseQueueFullPolicy);
        clientOptions.serverTooSlowPolicy = c2cpp::consumerTooSlowPolicy(options->serverTooSlowPolicy);
    }

    auto* me = new UntypedClient(ServiceDescription{IdString_t(TruncateToCapacity, service),
                                                    IdString_t(TruncateToCapacity, instance),
                                                    IdString_t(TruncateToCapacity, event)},
                                 clientOptions);

    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);
    return me;
}

void iox_client_deinit(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    delete self;
}

iox_AllocationResult iox_client_loan_request(iox_client_t const self, void** const payload, const uint64_t payloadSize)
{
    return iox_client_loan_aligned_request(self, payload, payloadSize, IOX_C_CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
}

iox_AllocationResult iox_client_loan_aligned_request(iox_client_t const self,
                                                     void** const payload,
                                                     const uint64_t payloadSize,
                                                     const uint32_t payloadAlignment)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(payload != nullptr, "'payload' must not be a 'nullptr'");

    auto result = self->loan(payloadSize, payloadAlignment);
    if (result.has_error())
    {
        return cpp2c::allocationResult(result.error());
    }

    *payload = result.value();
    return AllocationResult_SUCCESS;
}

void iox_client_release_request(iox_client_t const self, void* const payload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(payload != nullptr, "'payload' must not be a 'nullptr'");

    self->releaseRequest(payload);
}

iox_ClientSendResult iox_client_send(iox_client_t const self, void* const payload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    auto result = self->send(payload);
    if (result.has_error())
    {
        return cpp2c::clientSendResult(result.error());
    }

    return ClientSendResult_SUCCESS;
}

void iox_client_connect(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    self->connect();
}

void iox_client_disconnect(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    self->disconnect();
}

iox_ConnectionState iox_client_get_connection_state(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return cpp2c::connectionState(self->getConnectionState());
}

iox_ChunkReceiveResult iox_client_take_response(iox_client_t const self, const void** const payload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(payload != nullptr, "'payload' must not be a 'nullptr'");

    auto result = self->take();
    if (result.has_error())
    {
        return cpp2c::chunkReceiveResult(result.error());
    }

    *payload = result.value();
    return ChunkReceiveResult_SUCCESS;
}

void iox_client_release_response(iox_client_t const self, const void* const payload)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(payload != nullptr, "'payload' must not be a 'nullptr'");

    self->releaseResponse(payload);
}

void iox_client_release_queued_responses(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    self->releaseQueuedResponses();
}

bool iox_client_has_responses(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->hasResponses();
}

bool iox_client_has_missed_responses(iox_client_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->hasMissedResponses();
}

iox_service_description_t iox_client_get_service_description(iox_client_t const self)
{
    return TranslateServiceDescription(self->getServiceDescription());
}
