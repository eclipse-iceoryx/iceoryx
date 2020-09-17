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
#ifndef IOX_UTILS_WIN_PLATFORM_GETOPT_HPP
#define IOX_UTILS_WIN_PLATFORM_GETOPT_HPP

#define no_argument 0
#define required_argument 1

#define optarg "fuu"

extern int optind;
extern int opterr;
extern int optout;

struct option
{
    const char* name;
    int has_arg;
    int* flag;
    int val;
};

inline int getopt_long(int, char* const[], const char*, const struct option*, int*)
{
    return 0;
}

#endif // IOX_UTILS_WIN_PLATFORM_GETOPT_HPP
