// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_TYPES_HPP
#define IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_TYPES_HPP

#include "iceoryx_posh/roudi/introspection_types.hpp"

namespace iox
{
namespace client
{
namespace introspection
{
using namespace iox::roudi;

constexpr units::Duration WAIT_INTERVAL = 10_ms;

enum class PrettyOptions
{
    title,
    highlight,
    error,
    bold,
    normal
};

struct IntrospectionSelection
{
    bool mempool{false};
    bool process{false};
    bool port{false};
};

/// @note this contains just pointer to the real data, therefore pay attention to the lifetime of the original data
struct ComposedPublisherPortData
{
    ComposedPublisherPortData(const PublisherPortData& portData, const PortThroughputData& throughputData)
        : portData(&portData)
        , throughputData(&throughputData)
    {
    }
    const PublisherPortData* portData;
    const PortThroughputData* throughputData;
};

struct ComposedSubscriberPortData
{
    ComposedSubscriberPortData(const SubscriberPortData& portData,
                               const SubscriberPortChangingData& subscriberPortChangingData)
        : portData(&portData)
        , subscriberPortChangingData(&subscriberPortChangingData)
    {
    }
    const SubscriberPortData* portData;
    const SubscriberPortChangingData* subscriberPortChangingData;
};

} // namespace introspection
} // namespace client
} // namespace iox

#endif // IOX_TOOLS_ICEORYX_INTROSPECTION_INTROSPECTION_TYPES_HPP
