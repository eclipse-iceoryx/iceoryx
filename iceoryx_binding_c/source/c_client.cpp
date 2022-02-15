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
#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;
using namespace iox::capro;
using namespace iox::cxx;

extern "C" {
#include "iceoryx_binding_c/client.h"
}

#include <cstring>

constexpr uint64_t CLIENT_OPTIONS_INIT_CHECK_CONSTANT = 47113130815;

void iox_client_options_init(iox_client_options_t* options)
{
    iox::cxx::Expects(options != nullptr);

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
    iox::cxx::Expects(options != nullptr);

    return options->initCheck == CLIENT_OPTIONS_INIT_CHECK_CONSTANT;
}

iox_client_t iox_client_init(iox_client_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_client_options_t* const options)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(service != nullptr);
    iox::cxx::Expects(instance != nullptr);
    iox::cxx::Expects(event != nullptr);
    iox::cxx::Expects(options == nullptr || (options != nullptr && iox_client_options_is_initialized(options)));

    ClientOptions clientOptions;
    if (options != nullptr)
    {
        clientOptions.responseQueueCapacity = options->responseQueueCapacity;
        clientOptions.nodeName.append(TruncateToCapacity, options->nodeName);
        clientOptions.connectOnCreate = options->connectOnCreate;
        clientOptions.responseQueueFullPolicy = c2cpp::queueFullPolicy(options->responseQueueFullPolicy);
        clientOptions.serverTooSlowPolicy = c2cpp::consumerTooSlowPolicy(options->serverTooSlowPolicy);
    }

    new (self) UntypedClient(ServiceDescription{IdString_t(TruncateToCapacity, service),
                                                IdString_t(TruncateToCapacity, instance),
                                                IdString_t(TruncateToCapacity, event)},
                             clientOptions);

    return self;
}

void iox_client_deinit(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);

    reinterpret_cast<UntypedClient*>(self)->~UntypedClient();
}

ENUM iox_AllocationResult iox_client_loan(iox_client_t const self,
                                          void** const userPayload,
                                          const uint32_t userPayloadSize,
                                          const uint32_t userPayloadAlignment)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(userPayload != nullptr);

    auto result = reinterpret_cast<UntypedClient*>(self)->loan(userPayloadSize, userPayloadAlignment);
    if (result.has_error())
    {
        return cpp2c::allocationResult(result.get_error());
    }

    *userPayload = result.value();
    return AllocationResult_SUCCESS;
}

void iox_client_release_request(iox_client_t const self, void* const userPayload)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(userPayload != nullptr);

    reinterpret_cast<UntypedClient*>(self)->releaseRequest(userPayload);
}

void iox_client_send(iox_client_t const self, void* const userPayload)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(userPayload != nullptr);

    reinterpret_cast<UntypedClient*>(self)->send(userPayload);
}

void iox_client_connect(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    reinterpret_cast<ClientPortUser*>(self)->connect();
}

void iox_client_disconnect(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    reinterpret_cast<ClientPortUser*>(self)->disconnect();
}

ENUM iox_ConnectionState iox_client_get_connection_state(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    reinterpret_cast<ClientPortUser*>(self)->getConnectionState();
}

iox_ChunkReceiveResult iox_client_take(iox_client_t const self, const void** const userPayload)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(userPayload != nullptr);

    auto result = reinterpret_cast<UntypedClient*>(self)->take();
    if (result.has_error())
    {
        return cpp2c::chunkReceiveResult(result.get_error());
    }

    *userPayload = result.value();
    return ChunkReceiveResult_SUCCESS;
}

void iox_client_release_response(iox_client_t const self, void* const userPayload)
{
    iox::cxx::Expects(self != nullptr);
    iox::cxx::Expects(userPayload != nullptr);

    reinterpret_cast<UntypedClient*>(self)->releaseResponse(userPayload);
}

void iox_client_release_queued_responses(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    reinterpret_cast<UntypedClient*>(self)->releaseQueuedResponses();
}

bool iox_client_has_responses(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    return reinterpret_cast<UntypedClient*>(self)->hasResponses();
}

bool iox_client_has_missed_responses(iox_client_t const self)
{
    iox::cxx::Expects(self != nullptr);
    return reinterpret_cast<UntypedClient*>(self)->hasMissedResponses();
}
