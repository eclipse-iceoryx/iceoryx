// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/posh_runtime_single_process.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace runtime
{
PoshRuntime*& getSingleProcessRuntime() noexcept
{
    static PoshRuntime* singleProcessRuntime = nullptr;
    return singleProcessRuntime;
}

PoshRuntime& singleProcessRuntimeFactory(optional<const RuntimeName_t*>) noexcept
{
    return *getSingleProcessRuntime();
}

PoshRuntimeSingleProcess::PoshRuntimeSingleProcess(const RuntimeName_t& name) noexcept
    : PoshRuntimeImpl(
        make_optional<const RuntimeName_t*>(&name), DEFAULT_DOMAIN_ID, RuntimeLocation::SAME_PROCESS_LIKE_ROUDI)
{
    auto currentFactory = PoshRuntime::getRuntimeFactory();
    if (currentFactory != nullptr && *currentFactory == PoshRuntime::defaultRuntimeFactory)
    {
        getSingleProcessRuntime() = this;
        PoshRuntime::setRuntimeFactory(singleProcessRuntimeFactory);
    }
    else
    {
        IOX_LOG(ERROR,
                "PoshRuntimeSingleProcess can only created once per process and only if the default PoshRuntime "
                "factory method is set!");
        IOX_REPORT_FATAL(PoshError::POSH__RUNTIME_IS_CREATED_MULTIPLE_TIMES);
    }
}

PoshRuntimeSingleProcess::~PoshRuntimeSingleProcess()
{
    PoshRuntime::setRuntimeFactory(PoshRuntime::defaultRuntimeFactory);
}

} // namespace runtime
} // namespace iox
