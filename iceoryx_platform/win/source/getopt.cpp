// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "iceoryx_platform/getopt.hpp"
#include "iceoryx_platform/logging.hpp"
#include "iceoryx_platform/windows.hpp"

#include <cstdio>

int optind;
int opterr;
int optout;

int getopt_long(int argc, char* const[], const char*, const struct option*, int*)
{
    if (argc > 1)
    {
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'getopt_long' is not implemented in windows!");
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "command line arguments are not supported in windows!");
    }
    return -1;
}
