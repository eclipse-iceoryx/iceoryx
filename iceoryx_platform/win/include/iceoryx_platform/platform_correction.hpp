// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

// !NO INCLUDE GUARD HERE!
// We want that these macros are always executed and
// not just once, otherwise the windows macros are kicking in and nothing
// compiles


// Usage Instructions: This header has to be ALWAYS the last header which
// is included otherwise some windows header pops up and defines some macros

#include <cstring>

#if defined(_MSC_VER)

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#pragma pointers_to_members(full_generality)
#endif

#undef CreateMutex
#undef max
#undef min
#undef ERROR
#undef interface
#undef CreateSemaphore
#undef NO_ERROR
#undef OPEN_EXISTING
#undef IGNORE
#undef OPTIONAL
