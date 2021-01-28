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

#ifndef IOX_BINDING_C_PUBLISHER_OPTIONS_H
#define IOX_BINDING_C_PUBLISHER_OPTIONS_H

#include <stdint.h>

#define MAX_NODE_NAME_LENGTH_ON_C 100

/// @brief This struct is used to configure the publisher
struct c_PublisherOptions
{
    /// @brief The size of the history chunk queue
    uint64_t historyCapacity;

    /// @brief The name of the node where the publisher should belong to
    char nodeName[MAX_NODE_NAME_LENGTH_ON_C];
};

#endif

