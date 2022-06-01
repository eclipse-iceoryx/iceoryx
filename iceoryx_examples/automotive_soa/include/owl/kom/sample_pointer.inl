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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_POINTER_INL
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_POINTER_INL

#include "owl/kom/sample_pointer.hpp"

namespace owl
{
namespace kom
{
template <typename SampleType>
inline SampleType* SamplePointer<SampleType>::operator->() noexcept
{
    if (!this->has_value())
    {
        // We don't allow undefined behaviour
        std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return this->value().get();
}

template <typename SampleType>
inline const SampleType* SamplePointer<SampleType>::operator->() const noexcept
{
    const_cast<const SampleType*>(const_cast<SamplePointer<SampleType>*>(this)->operator->());
}

//! [SamplePointer dereferencing]
template <typename SampleType>
inline SampleType& SamplePointer<SampleType>::operator*() noexcept
{
    return *this->operator->();
}
//! [SamplePointer dereferencing]

template <typename SampleType>
inline const SampleType& SamplePointer<SampleType>::operator*() const noexcept
{
    const_cast<const SampleType*>(const_cast<SamplePointer<SampleType>*>(this)->operator->());
}

template <typename SampleType>
inline SamplePointer<SampleType>::operator bool() const noexcept
{
    return this->has_value();
}
} // namespace kom
} // namespace owl
#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_SAMPLE_POINTER_HPP
