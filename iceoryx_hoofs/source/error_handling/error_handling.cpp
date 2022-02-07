// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"


namespace iox
{
const char* ERROR_NAMES[] = {ICEORYX_ERRORS(CREATE_ICEORYX_ERROR_STRING)};

const char* toString(const Error error) noexcept
{
    return ERROR_NAMES[static_cast<uint32_t>(error)];
}

std::ostream& operator<<(std::ostream& stream, Error value) noexcept
{
    stream << toString(value);
    return stream;
}
} // namespace iox
