// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/posh_discovery.hpp"

namespace iox
{
namespace runtime
{
ServiceDiscovery::ServiceDiscovery() noexcept
    : m_applicationPort(PoshRuntime::getInstance().getMiddlewareApplication())

{
}

ServiceDiscovery::~ServiceDiscovery() noexcept
{
}

cxx::expected<ServiceContainer, FindServiceError>
ServiceDiscovery::findService(const cxx::variant<Wildcard_t, capro::IdString_t> service,
                              const cxx::variant<Wildcard_t, capro::IdString_t> instance) noexcept
{
    /// @todo #415 remove the string mapping, once the find call is done via shared memory
    capro::IdString_t serviceString;
    capro::IdString_t instanceString;

    if (service.index() == 0U)
    {
        serviceString = "*";
    }
    else
    {
        serviceString = *service.get_at_index<1U>();
    }

    if (instance.index() == 0U)
    {
        instanceString = "*";
    }
    else
    {
        instanceString = *instance.get_at_index<1U>();
    }

    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::FIND_SERVICE) << PoshRuntime::getInstance().getInstanceName()
               << serviceString << instanceString;

    IpcMessage requestResponse;

    if (!PoshRuntime::getInstance().sendRequestToRouDi(sendBuffer, requestResponse))
    {
        LogError() << "Could not send FIND_SERVICE request to RouDi\n";
        errorHandler(Error::kIPC_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_CHANNEL, nullptr, ErrorLevel::MODERATE);
        return cxx::error<FindServiceError>(FindServiceError::UNABLE_TO_WRITE_TO_ROUDI_CHANNEL);
    }

    ServiceContainer serviceContainer;
    uint32_t numberOfElements = requestResponse.getNumberOfElements();
    uint32_t capacity = static_cast<uint32_t>(serviceContainer.capacity());

    // Limit the services (max value is the capacity of serviceContainer)
    uint32_t numberOfServices = algorithm::min(capacity, numberOfElements);
    for (uint32_t i = 0U; i < numberOfServices; ++i)
    {
        auto deserializationResult =
            capro::ServiceDescription::deserialize(cxx::Serialization(requestResponse.getElementAtIndex(i)));
        if (deserializationResult.has_error())
        {
            return cxx::error<FindServiceError>(FindServiceError::DESERIALIZATION_FAILED);
        }
        serviceContainer.push_back(deserializationResult.value());
    }

    if (numberOfElements > capacity)
    {
        LogWarn() << numberOfElements << " instances found for service \"" << serviceString
                  << "\" which is more than supported number of services(" << MAX_NUMBER_OF_SERVICES << "\n";
        errorHandler(Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<FindServiceError>(FindServiceError::INSTANCE_CONTAINER_OVERFLOW);
    }
    return {cxx::success<ServiceContainer>(serviceContainer)};
}


bool ServiceDiscovery::offerService(const capro::ServiceDescription& serviceDescription) noexcept
{
    if (serviceDescription.isValid())
    {
        capro::CaproMessage msg(
            capro::CaproMessageType::OFFER, serviceDescription, capro::CaproMessageSubType::SERVICE);
        m_applicationPort.dispatchCaProMessage(msg);
        return true;
    }
    LogWarn() << "Could not offer service " << serviceDescription.getServiceIDString() << ","
              << " ServiceDescription is invalid\n";
    return false;
}

bool ServiceDiscovery::stopOfferService(const capro::ServiceDescription& serviceDescription) noexcept
{
    if (serviceDescription.isValid())
    {
        capro::CaproMessage msg(
            capro::CaproMessageType::STOP_OFFER, serviceDescription, capro::CaproMessageSubType::SERVICE);
        m_applicationPort.dispatchCaProMessage(msg);
        return true;
    }
    LogWarn() << "Could not stopOffer service " << serviceDescription.getServiceIDString() << ","
              << " ServiceDescription is invalid\n";
    return false;
}

const std::atomic<uint64_t>* ServiceDiscovery::getServiceRegistryChangeCounter() noexcept
{
    IpcMessage sendBuffer;
    sendBuffer << IpcMessageTypeToString(IpcMessageType::SERVICE_REGISTRY_CHANGE_COUNTER)
               << PoshRuntime::getInstance().getInstanceName();
    IpcMessage receiveBuffer;
    if (PoshRuntime::getInstance().sendRequestToRouDi(sendBuffer, receiveBuffer)
        && (2U == receiveBuffer.getNumberOfElements()))
    {
        rp::BaseRelativePointer::offset_t offset{0U};
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(0U).c_str(), offset);
        rp::BaseRelativePointer::id_t segmentId{0U};
        cxx::convert::fromString(receiveBuffer.getElementAtIndex(1U).c_str(), segmentId);
        auto ptr = rp::BaseRelativePointer::getPtr(segmentId, offset);

        return reinterpret_cast<std::atomic<uint64_t>*>(ptr);
    }
    else
    {
        LogError() << "unable to request service registry change counter caused by wrong response from RouDi: \""
                   << receiveBuffer.getMessage() << "\" with request: \"" << sendBuffer.getMessage() << "\"";
        return nullptr;
    }
}
} // namespace runtime
} // namespace iox
