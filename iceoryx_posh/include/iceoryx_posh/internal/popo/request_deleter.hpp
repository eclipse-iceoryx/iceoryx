// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_REQUEST_DELETER_HPP
#define IOX_POSH_POPO_REQUEST_DELETER_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/rpc_header.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The RequestDeleter struct is a custom deleter in functor form which releases loans to a request's
/// underlying memory chunk via the corresponding port.
/// Each port should create its own instance of this deleter struct.
/// @tparam Port is either the ClientPortUser or ServerPortUser and need to have a `releaseResponse` method
template <typename Port>
struct RequestDeleter
{
  public:
    explicit RequestDeleter(Port& port) noexcept;

    /// @brief Handles deletion of the request.
    /// @param[in] payload The pointer to the payload of the request.
    template <typename T>
    void operator()(T* const payload) noexcept;

    /// @brief Handles deletion of the request.
    /// @param[in] payload The pointer to the user-payload of the request.
    template <typename T>
    void operator()(const T* const payload) const noexcept;

  private:
    Port* m_port;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/request_deleter.inl"

#endif // IOX_POSH_POPO_REQUEST_DELETER_HPP
