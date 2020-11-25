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
#ifndef IOX_POSH_RUNTIME_RUNNABLE_PROPERTY_HPP
#define IOX_POSH_RUNTIME_RUNNABLE_PROPERTY_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace runtime
{
/// @brief helper struct which is convertable to string and constructable from a string
///         which is required to send the createRunnable request over the message queue
struct RunnableProperty
{
    /// @brief constructor
    /// @param[in] name name of the runnable
    /// @param[in] runnableDeviceIdentifier identifier of the device on which the runnable will run
    RunnableProperty(const iox::RunnableName_t& name, const uint64_t runnableDeviceIdentifier) noexcept;

    /// @brief serialization constructor, used by the message queue message to create RunnableProperty
    ///         from a received message
    /// @param[in] serialized raw serialized string where all the values are stored
    RunnableProperty(const cxx::Serialization& serialized) noexcept;

    /// @brief serialization of the runnable properties
    operator cxx::Serialization() const noexcept;

    iox::RunnableName_t m_name;
    uint64_t m_runnableDeviceIdentifier;
};
} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_RUNNABLE_PROPERTY_HPP
