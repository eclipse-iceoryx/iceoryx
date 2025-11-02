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

#include "iceoryx_platform/signal.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"

int sigemptyset(sigset_t* set)
{
    return 0;
}

int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)
{
    auto previousSignalHandler = Win32Call(signal, signum, act->sa_handler).value;
    if (oldact != nullptr)
    {
        oldact->sa_handler = previousSignalHandler;
    }
    return 0;
}

int kill(pid_t pid, int sig)
{
    return 0;
}
