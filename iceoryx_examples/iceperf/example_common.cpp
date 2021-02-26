// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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


#include "example_common.hpp"
#include <cstring>
#include <iostream>

Benchmarks getBenchmarkFromString(const char* argv) noexcept
{
    if (strcmp(argv, "all") == 0)
    {
        return Benchmarks::ALL;
    }
    else if (strcmp(argv, "cpp-api") == 0)
    {
        return Benchmarks::CPP_API;
    }
    else if (strcmp(argv, "c-api") == 0)
    {
        return Benchmarks::C_API;
    }
    else
    {
        std::cout << "parameter must be either 'all', 'cpp-api', 'c-api' or empty" << std::endl;
        exit(1);
    }
}
