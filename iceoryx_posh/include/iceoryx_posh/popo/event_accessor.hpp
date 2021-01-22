// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_EVENT_ACCESSOR_HPP
#define IOX_POSH_POPO_EVENT_ACCESSOR_HPP

#include <cstdint>

namespace iox
{
namespace popo
{
/// @brief Class which allows to restrict public methods to be used only by
///         friends of EventAccessor. Used for example by the WaitSet and Trigger
class EventAccessor
{
    template <uint64_t>
    class WaitSet;
    class Trigger;
    class TriggerHandle;

    /// only friends of EventAccessor should be able to construct this class
  private:
    constexpr EventAccessor() noexcept = default;
};


} // namespace popo
} // namespace iox
#endif
