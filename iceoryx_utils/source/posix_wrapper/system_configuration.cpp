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

#include "iceoryx_utils/internal/posix_wrapper/system_configuration.hpp"

#include "iceoryx_utils/cxx/smart_c.hpp"

namespace iox
{
namespace posix
{
cxx::optional<uint64_t> pageSize()
{
    auto size = makeSmartC(sysconf, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<long>(-1)}, {}, _SC_PAGESIZE);
    if (size.hasErrors())
    {
        return cxx::nullopt_t();
    }

    return cxx::make_optional<uint64_t>(size.getReturnValue());
}
} // namespace posix
} // namespace iox
