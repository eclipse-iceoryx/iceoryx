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

#include "iceoryx_introspection/introspection_types.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

#include <vector>

namespace iox
{
namespace client
{
namespace introspection
{
using SubscriberType = iox::popo::Subscriber;

bool waitForSubscription(SubscriberType& port);

std::vector<ComposedSenderPortData> composeSenderPortData(const PortIntrospectionFieldTopic* portData,
                                                          const PortThroughputIntrospectionFieldTopic* throughputData);

std::vector<ComposedReceiverPortData> composeReceiverPortData(const PortIntrospectionFieldTopic& portData);

void runIntrospection(const int updatePeriodMs, const IntrospectionSelection introspectionSelection);

} // namespace introspection
} // namespace client
} // namespace iox
