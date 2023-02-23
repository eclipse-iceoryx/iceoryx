// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#include "iceoryx_platform/file.hpp"

int iox_flock(int, int)
{
    // This file locking is used in iceoryx for two reasons:
    // 1) To prevent another instance of RouDi to cleanup the memory resources of a running RouDi.
    // 2) To prevent another Posh application with the same runtime name to cleanup the memory resources of a running
    // application.
    //
    // Both of these things will never happen on FreeRTOS, so we can just leave the implementation empty.
    return 0;
}
