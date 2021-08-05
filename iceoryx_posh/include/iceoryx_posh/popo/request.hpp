// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2021 by AVIN Systems Private Limited All rights reserved.
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

#ifndef IOX_POSH_POPO_REQUEST_HPP
#define IOX_POSH_POPO_REQUEST_HPP

#include "iceoryx_posh/popo/smart_chunk.hpp"

namespace iox
{
namespace popo
{
template <typename T>
class RpcInterface;

///
/// @brief The Request class is a mutable abstraction over types which are written to loaned shared memory.
/// These requests are transmittable to the iceoryx system.
///
template <typename T>
class Request : public SmartChunk<RpcInterface<Request<T>>, T, RequestHeader>
{
    using Base_t =  SmartChunk<RpcInterface<Request<T>>, T, RequestHeader>;
    /// @brief Helper type to enable the constructor for the producer, i.e. when T has a non const qualifier
    template <typename S, typename TT>
    using ForProducerOnly = typename Base_t::template ForProducerOnly<S, TT>;

    /// @brief Helper type to enable the constructor for the consumer, i.e. when T has a const qualifier
    template <typename S, typename TT>
    using ForConsumerOnly = typename Base_t::template ForConsumerOnly<S, TT>;

  public:
    /// @brief Constructor for a Request used by the producer
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param requestUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    /// @param producer is a reference to the producer to be able to use the `send` and `release` methods
    template <typename S = T, typename = ForProducerOnly<S, T>>
    Request(cxx::unique_ptr<T>&& requestUniquePtr, RpcInterface<Request<T>>& producer) noexcept;

    /// @brief Constructor for a Request used by the Subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param requestUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    template <typename S = T, typename = ForConsumerOnly<S, T>>
    Request(cxx::unique_ptr<T>&& requestUniquePtr) noexcept;

    ~Request() noexcept = default;

    Request<T>& operator=(Request<T>&& rhs) noexcept = default;
    Request(Request<T>&& rhs) noexcept = default;

    Request(const Request<T>&) = delete;
    Request<T>& operator=(const Request<T>&) = delete;

    ///
    /// @brief Retrieve the request header of the underlying memory chunk loaned to the request.
    /// @return The request header of the underlying memory chunk.
    ///
    RequestHeader& getRequestHeader() noexcept;

    ///
    /// @brief Retrieve the request header of the underlying memory chunk loaned to the request.
    /// @return The request header of the underlying memory chunk.
    ///
    const RequestHeader& getRequestHeader() const noexcept;

    ///
    /// @brief send the request via the producer from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForProducerOnly<S, T>>
    void send() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/request.inl"

#endif // IOX_POSH_POPO_REQUEST_HPP
