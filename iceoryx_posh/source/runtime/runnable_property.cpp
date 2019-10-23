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

#include "iceoryx_posh/internal/runtime/runnable_property.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/serialization.hpp"

namespace iox
{
namespace runtime
{
RunnableProperty::RunnableProperty(const iox::cxx::CString100& name, const uint64_t runnableDeviceIdentifier) noexcept
    : m_name(name)
    , m_runnableDeviceIdentifier(runnableDeviceIdentifier)
{
}

RunnableProperty::RunnableProperty(const std::string& serialized) noexcept
{
    if (!cxx::Serialization(serialized).extract(m_name, m_runnableDeviceIdentifier))
    {
        LogError() << "unable to create RunnableProperty from serialized string " << serialized;
    }
}

RunnableProperty::operator std::string() const noexcept
{
    return cxx::Serialization::create(m_name, m_runnableDeviceIdentifier).toString();
}
} // namespace runtime
} // namespace iox
