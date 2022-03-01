// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_PUBLISHER_INTERFACE_HPP
#define IOX_POSH_POPO_PUBLISHER_INTERFACE_HPP

#include "iceoryx_posh/popo/sample.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The PublisherInterface class defines the publisher interface used by the Sample class to make it generic.
/// This allows any publisher specialization to be stored as a reference by the Sample class.
/// It is also needed to avoid circular dependencies between Sample and Publisher.
///
template <typename T, typename H>
class PublisherInterface
{
  public:
    using SampleType = Sample<T, H>;

    /// @brief Publishes the given sample and then releases its loan.
    /// @param sample The sample to publish.
    virtual void publish(SampleType&& sample) noexcept = 0;

  protected:
    PublisherInterface() = default;
};
} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PUBLISHER_INTERFACE_HPP
