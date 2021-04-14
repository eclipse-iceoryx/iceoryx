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

#include "iceoryx_dds/dds/iox_chunk_datagram_header.hpp"

namespace iox
{
namespace dds
{
Endianess getEndianess()
{
    uint32_t endianDetector{0x01020304};
    switch (reinterpret_cast<uint8_t*>(&endianDetector)[0U])
    {
    case 1U:
        return Endianess::BIG;
    case 4U:
        return Endianess::LITTLE;
    case 2U:
        return Endianess::MIXED;
    default:
        return Endianess::UNDEFINED;
    }

    return Endianess::UNDEFINED;
}

} // namespace dds
} // namespace iox
