// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_WINDOWS_HPP
#define IOX_HOOFS_WIN_PLATFORM_WINDOWS_HPP

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h> // needs to be included before windows.h and therefore the blank line to prevent clang format from reordering

#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include "iceoryx_platform/platform_correction.hpp"

#endif // IOX_HOOFS_WIN_PLATFORM_WINDOWS_HPP
