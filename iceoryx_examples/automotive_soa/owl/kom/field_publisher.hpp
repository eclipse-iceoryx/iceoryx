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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/publisher.hpp"

#include "owl/types.hpp"

namespace owl
{
namespace kom
{
template <typename T>
class FieldPublisher
{
  public:
    using FieldType = T;

    FieldPublisher(const FieldPublisher&) = delete;
    FieldPublisher(FieldPublisher&&) = delete;
    FieldPublisher& operator=(const FieldPublisher&) = delete;
    FieldPublisher& operator=(FieldPublisher&&) = delete;

    FieldPublisher(const core::String& service,
                   const core::String& instance,
                   const core::String& event,
                   FieldType& field) noexcept
        : m_publisher({service, instance, event}, {1U})
    {
        // publisher is automatically offered
        Update(field);
    }

    ~FieldPublisher() noexcept
    {
        m_publisher.stopOffer();
    }

    void Update(const FieldType& userField) noexcept
    {
        auto maybeField = m_publisher.loan();

        if (maybeField.has_error())
        {
            std::cout << "Error occured during allocation, couldn't send sample!" << std::endl;
            return;
        }

        auto& field = maybeField.value();
        *(field.get()) = userField;
        field.publish();
    }

    void RegisterGetHandler(iox::cxx::function<void()> getHandler) noexcept
    {
        /// @todo #1332 implement
    }

    void RegisterSetHandler(iox::cxx::function<void()> setHandler) noexcept
    {
        /// @todo #1332 implement
    }

  private:
    iox::popo::Publisher<FieldType> m_publisher;
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP
