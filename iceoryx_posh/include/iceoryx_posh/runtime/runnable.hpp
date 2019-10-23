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

#pragma once

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace runtime
{
struct RunnableData;

/// @brief class which represents a runnable
class Runnable
{
  public:
    /// @brief constructor which requires the name of the runnable
    /// @param[in] data pointer to the data
    Runnable(const iox::cxx::CString100& runnableName) noexcept;

    /// @brief destructor
    ~Runnable() noexcept;

    Runnable(const Runnable&) = delete;
    Runnable& operator=(const Runnable&) = delete;

    /// @brief move constructor
    /// @param[in] rhs source object
    Runnable(Runnable&& rhs) noexcept;

    /// @brief move assignment operator
    /// @param[in] rhs source object, where to move from
    Runnable& operator=(Runnable&& rhs) noexcept;

    /// @brief returns the name of the runnable
    /// @return string which contains the runnable name
    cxx::CString100 getRunnableName() const noexcept;

    /// @brief returns the name of the process
    /// @return string which contains the process name
    cxx::CString100 getProcessName() const noexcept;

  private:
    RunnableData* m_data;
};
} // namespace runtime
} // namespace iox
