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

#include "iceoryx_posh/mepoo/chunk_header.hpp"


extern "C" {
#include "iceoryx_binding_c/chunk.h"
}

using namespace iox::mepoo;

void* iox_chunk_header_to_payload(iox_chunk_header_t const header)
{
    return reinterpret_cast<ChunkHeader*>(header)->payload();
}

iox_chunk_header_t iox_chunk_payload_to_header(const void* const payload)
{
    return ChunkHeader::fromPayload(payload);
}
