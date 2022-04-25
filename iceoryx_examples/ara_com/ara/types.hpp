// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_ARA_COM_TYPES_HPP
#define IOX_EXAMPLES_ARA_COM_TYPES_HPP

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "com/sample_allocatee_ptr.hpp"

#include <future>
#include <memory>

namespace ara
{
class Runtime;
namespace core
{
template <typename T>
using Result = T;

using String = iox::cxx::string<100>;
} // namespace core

namespace com
{
template <typename T>
using SamplePtr = iox::cxx::optional<iox::popo::Sample<const T>>;

struct FindServiceHandle
{
    // Only the Runtime shall be able to create handles, not the user
    friend class ara::Runtime;

    core::String getServiceIdentifier() noexcept
    {
        return m_serviceIdentifier;
    }

    core::String getInstanceIdentifer() noexcept
    {
        return m_instanceIdentifier;
    }

  private:
    FindServiceHandle(core::String serviceIdentifier,
                      core::String instanceIdentifier,
                      iox::popo::MessagingPattern pattern) noexcept
        : m_serviceIdentifier(serviceIdentifier)
        , m_instanceIdentifier(instanceIdentifier)
        , m_pattern(pattern)
    {
    }
    core::String m_serviceIdentifier;
    core::String m_instanceIdentifier;
    iox::popo::MessagingPattern m_pattern;
};

template <typename T>
using ServiceHandleContainer = iox::cxx::vector<T, 50>;

template <typename T>
using FindServiceHandler = iox::cxx::function<void(ServiceHandleContainer<T>, FindServiceHandle)>;

// We use the STL version as future is not available in hoofs
template <typename T>
using Future = std::future<T>;

// We use the STL version as promise is not available in hoofs
template <typename T>
using Promise = std::promise<T>;
} // namespace com
} // namespace ara

#endif // IOX_EXAMPLES_ARA_COM_TYPES_HPP
