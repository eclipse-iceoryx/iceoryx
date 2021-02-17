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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CYCLONE_CONTEXT_HPP
#define CYCLONE_CONTEXT_HPP

#include <dds/dds.hpp>

namespace iox
{
namespace dds
{
///
/// @brief The CycloneContext manages cyclone configurations and singleton artifacts shared throughout an application.
///
class CycloneContext
{
  public:
    ///
    /// @brief getParticipant Get the DDS Domain Participant for the current runtime.
    /// @return The DDS Domain Participant.
    ///
    static ::dds::domain::DomainParticipant& getParticipant() noexcept;
};

} // namespace dds
} // namespace iox

#endif // CYCLONE_CONTEXT_HPP
