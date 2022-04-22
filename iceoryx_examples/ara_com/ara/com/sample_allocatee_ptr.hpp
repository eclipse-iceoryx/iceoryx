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

#ifndef IOX_EXAMPLES_ARA_COM_SAMPLE_ALLOCATEE_PTR_HPP
#define IOX_EXAMPLES_ARA_COM_SAMPLE_ALLOCATEE_PTR_HPP

#include "iceoryx_posh/popo/sample.hpp"

namespace ara
{
namespace com
{
/// @brief This class mimics a std::unique_ptr
template <typename SampleType>
class SampleAllocateePtr : private iox::cxx::optional<iox::popo::Sample<SampleType>>
{
  public:
    using ParentType = iox::cxx::optional<iox::popo::Sample<SampleType>>;

    using ParentType::ParentType;

    SampleType* operator->() noexcept
    {
        if (!this->has_value())
        {
            // We don't allow undefined behaviour
            std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
            std::terminate();
        }
        return this->value().get();
    }

    const SampleType* operator->() const noexcept
    {
        const_cast<const SampleType*>(const_cast<SampleAllocateePtr<SampleType>*>(this)->operator->());
    }

    SampleType& operator*() noexcept
    {
        if (!this->has_value())
        {
            // We don't allow undefined behaviour
            std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
            std::terminate();
        }
        return *(this->value().get());
    }

    const SampleType& operator*() const noexcept
    {
        const_cast<const SampleType*>(const_cast<SampleAllocateePtr<SampleType>*>(this)->operator->());
    }

    template <typename T>
    friend class EventPublisher;
};
} // namespace com
} // namespace ara
#endif // IOX_EXAMPLES_ARA_COM_SAMPLE_ALLOCATEE_PTR_HPP
