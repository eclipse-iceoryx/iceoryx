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

#pragma once

namespace iox
{
#include <stdint.h>

template <typename T>
constexpr bool isPowerOfTwo(T n)
{
    return (n != 0) && ((n & (n - 1)) == 0);
}

// needed to trick the compiler
// unused template argument T is needed in a SFINAE use case
template <typename T = uint64_t, T n = 0>
struct is_power_of_two
{
    static constexpr bool value = isPowerOfTwo(n);
};
} // namespace iox