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

namespace iox
{
namespace cxx
{
/// @brief reference counter abstraction for the usage in constructs like a
///         shared_ptr. A pointer to a memory position where the reference
///         counter is stored is put into the constructor and then this object
///         performs reference counting on it.
template <typename T>
class ReferenceCounter
{
  public:
    /// @brief increments f_referenceCounter if f_referenceCounter != nullptr
    /// @param[in] referenceCounter pointer to the underlying number where the reference count is stored
    ReferenceCounter(T* const referenceCounter) noexcept;

    /// @brief copy constructor, increments the rhs managed reference counter if it is != nullptr
    ReferenceCounter(const ReferenceCounter& rhs) noexcept;

    /// @brief move constructor, takes over the f_rhs managed reference counter
    ReferenceCounter(ReferenceCounter&& f_rhs) noexcept;

    /// @brief decrements f_referenceCounter if f_referenceCounter != nullptr
    ~ReferenceCounter() noexcept;

    /// @brief copy assignment, increments the rhs managed reference counter if it is != nullptr
    ReferenceCounter& operator=(const ReferenceCounter& rhs) noexcept;

    /// @brief move assignment
    ReferenceCounter& operator=(ReferenceCounter&& rhs) noexcept;

    /// @brief returns the current value of the reference counter
    T getValue() const noexcept;

  private:
    void incrementReferenceCounter() noexcept;
    void decrementReferenceCounter() noexcept;

  private:
    T* m_referenceCounter;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/reference_counter.inl"
