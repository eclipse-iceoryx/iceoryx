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

#ifndef IOX_HOOFS_QNX_PLATFORM_ATTRIBUTES_HPP
#define IOX_HOOFS_QNX_PLATFORM_ATTRIBUTES_HPP

/// @todo iox-#638 Are any of the below flags available with C++14 on QCC?
#if __cplusplus >= 201703L
#define IOX_NO_DISCARD [[nodiscard]]
#else
#define IOX_NO_DISCARD
#endif

#if __cplusplus >= 201703L
#define IOX_FALLTHROUGH [[fallthrough]]
#else
#define IOX_FALLTHROUGH
#endif

#if __cplusplus >= 201703L
#define IOX_MAYBE_UNUSED [[maybe_unused]]
#else
#define IOX_MAYBE_UNUSED [[gnu::unused]]
#endif

#endif // IOX_HOOFS_QNX_PLATFORM_ATTRIBUTES_HPP
