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

#ifndef IOX_PLATFORM_MINIMAL_POSIX_TYPES_HPP
#define IOX_PLATFORM_MINIMAL_POSIX_TYPES_HPP

#include <sys/types.h>

using iox_ssize_t = ssize_t;

using iox_gid_t = gid_t;
using iox_uid_t = uid_t;

#endif // IOX_PLATFORM_MINIMAL_POSIX_TYPES_HPP
