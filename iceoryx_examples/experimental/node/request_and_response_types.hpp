// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_REQUEST_AND_RESPONSE_TYPES_HPP
#define IOX_EXAMPLES_REQUEST_AND_RESPONSE_TYPES_HPP

#include <cstdint>

//! [request]
struct AddRequest
{
    uint64_t augend{0};
    uint64_t addend{0};
};
//! [request]

//! [response]
struct AddResponse
{
    uint64_t sum{0};
};
//! [response]

#endif // IOX_EXAMPLES_REQUEST_AND_RESPONSE_TYPES_HPP
