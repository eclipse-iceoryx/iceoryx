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

#include "iceoryx_posh/roudi/introspection_types.hpp"

namespace iox
{
namespace client
{
namespace introspection
{
using namespace iox::roudi;

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
struct ComposedSenderPortData
{
    ComposedSenderPortData(const SenderPortData& portData, const PortThroughputData& throughputData)
        : portData(&portData)
        , throughputData(&throughputData)
    {
    }
    const SenderPortData* portData;
    const PortThroughputData* throughputData;
};

struct ComposedReceiverPortData
{
    ComposedReceiverPortData(const ReceiverPortData& f_portData,
                             const SenderPortData* f_correspondingSenderPortData,
                             const ReceiverPortChangingData& f_receiverPortChangingData)
        : portData(&f_portData)
        , correspondingSenderPort(f_correspondingSenderPortData)
        , receiverPortChangingData(&f_receiverPortChangingData)
    {
    }
    const ReceiverPortData* portData;
    const SenderPortData* correspondingSenderPort;
    const ReceiverPortChangingData* receiverPortChangingData;
};

} // namespace introspection
} // namespace client
} // namespace iox
