// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace posix
{
template <typename SmartC>
bool Semaphore::setHandleFromCall(const SmartC& call) noexcept
{
    if (call.hasErrors())
    {
        return false;
    }

    m_handlePtr = call.getReturnValue();
    return true;
}
} // namespace posix
} // namespace iox
