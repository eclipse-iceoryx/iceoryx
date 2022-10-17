// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_SERVICE_DESCRIPTION_H
#define IOX_BINDING_C_SERVICE_DESCRIPTION_H

#include "iceoryx_binding_c/config.h"

typedef struct
{
    char serviceString[IOX_CONFIG_SERVICE_STRING_SIZE];
    char instanceString[IOX_CONFIG_SERVICE_STRING_SIZE];
    char eventString[IOX_CONFIG_SERVICE_STRING_SIZE];
} iox_service_description_t;

#endif
