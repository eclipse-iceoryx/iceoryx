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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_HPP
#define IOX_POSH_POPO_TYPED_PUBLISHER_HPP

#include "iceoryx_posh/internal/popo/publisher_impl.hpp"

namespace iox::posh::experimental
{
class PublisherBuilder;
}

namespace iox
{
namespace popo
{
/// @brief The Publisher class for the publish-subscribe messaging pattern in iceoryx.
/// @param[in] T user payload type
/// @param[in] H user header type
template <typename T, typename H = mepoo::NoUserHeader>
class Publisher : public PublisherImpl<T, H>
{
  public:
    using PublisherImpl<T, H>::PublisherImpl;

  private:
    friend class iox::posh::experimental::PublisherBuilder;

    explicit Publisher(typename PublisherImpl<T, H>::PortType&& port) noexcept
        : PublisherImpl<T, H>(std::move(port))
    {
    }
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_HPP
