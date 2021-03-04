// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_MAC_PLATFORM_SIGNAL_HPP
#define IOX_UTILS_MAC_PLATFORM_SIGNAL_HPP

#include <signal.h>

inline void psignal(int sig, const char *s)
{
    psignal(static_cast<unsigned int>(sig), s);
}

#endif // IOX_UTILS_MAC_PLATFORM_SIGNAL_HPP
