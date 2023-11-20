// Copyright 2023, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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
#ifndef IOX_HOOFS_BUFFER_BUFFER_INFO_HPP
#define IOX_HOOFS_BUFFER_BUFFER_INFO_HPP

#include <cstdint>

namespace iox
{
/// @brief struct used to define the used size and total size of a buffer
struct BufferInfo
{
    uint64_t used_size{0};
    uint64_t total_size{0};
};

} // namespace iox

#endif
