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

#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"

iox_service_description_t TranslateServiceDescription(const iox::capro::ServiceDescription& serviceDescription) noexcept
{
    iox_service_description_t retVal;
    retVal.serviceId = serviceDescription.getServiceID();
    retVal.instanceId = serviceDescription.getInstanceID();
    retVal.eventId = serviceDescription.getEventID();

// ignore this warning since we already ensure that the string sizes are correct in the test
// test_service_description.cpp (iox_service_description_test.StringSizesAreCorrect)
// therefore a string truncation will never occur.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(retVal.serviceString, serviceDescription.getServiceIDString().c_str(), iox::capro::IdString_t().capacity());
    strncpy(
        retVal.instanceString, serviceDescription.getInstanceIDString().c_str(), iox::capro::IdString_t().capacity());
    strncpy(retVal.eventString, serviceDescription.getEventIDString().c_str(), iox::capro::IdString_t().capacity());
#pragma GCC diagnostic pop

    return retVal;
}

