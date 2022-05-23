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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_ALLOCATEE_PTR_INL
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_ALLOCATEE_PTR_INL

#include "owl/kom/sample_allocatee_ptr.hpp"

namespace owl
{
namespace kom
{
template <typename SampleType>
inline SampleType* SampleAllocateePtr<SampleType>::operator->() noexcept
{
    if (!this->has_value())
    {
        // We don't allow undefined behaviour
        std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
        std::terminate();
    }
    return this->value().get();
}

template <typename SampleType>
inline const SampleType* SampleAllocateePtr<SampleType>::operator->() const noexcept
{
    const_cast<const SampleType*>(const_cast<SampleAllocateePtr<SampleType>*>(this)->operator->());
}

//! [SampleAllocateePtr dereferencing]
template <typename SampleType>
inline SampleType& SampleAllocateePtr<SampleType>::operator*() noexcept
{
    if (!this->has_value())
    {
        // We don't allow undefined behaviour
        std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
        std::terminate();
    }
    return *(this->value().get());
}
//! [SampleAllocateePtr dereferencing]

template <typename SampleType>
inline const SampleType& SampleAllocateePtr<SampleType>::operator*() const noexcept
{
    const_cast<const SampleType*>(const_cast<SampleAllocateePtr<SampleType>*>(this)->operator->());
}

template <typename SampleType>
inline SampleAllocateePtr<SampleType>::operator bool() const noexcept
{
    return this->has_value();
}
} // namespace kom
} // namespace owl
#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_ALLOCATEE_PTR_HPP
