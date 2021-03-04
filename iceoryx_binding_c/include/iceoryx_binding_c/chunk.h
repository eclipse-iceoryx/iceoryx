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

#ifndef IOX_BINDING_C_CHUNK_H
#define IOX_BINDING_C_CHUNK_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"

/// @brief handle of the chunk header
typedef void* iox_chunk_header_t;

/// @brief gets the payload from the chunk header
/// @param[in] header pointer to the chunk header
/// @return pointer to the payload
void* iox_chunk_header_to_payload(iox_chunk_header_t const header);

/// @brief gets the chunk header from the payload
/// @param[in] payload pointer to the payload
/// @return pointer to the chunk header
iox_chunk_header_t iox_chunk_payload_to_header(const void* const payload);

#endif
