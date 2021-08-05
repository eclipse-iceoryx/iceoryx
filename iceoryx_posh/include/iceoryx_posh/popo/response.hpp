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

#ifndef IOX_POSH_POPO_RESPONSE_HPP
#define IOX_POSH_POPO_RESPONSE_HPP

#include "iceoryx_posh/popo/smart_chunk.hpp"

namespace iox
{
namespace popo
{
template <typename T>
class RpcInterface;

///
/// @brief The Response class is a mutable abstraction over types which are written to loaned shared memory.
/// These responses are transmittable to the iceoryx system.
///
template <typename T>
class Response : public SmartChunk<RpcInterface<Response<T>>, T, ResponseHeader>
{

    using Base_t =  SmartChunk<RpcInterface<Response<T>>, T, ResponseHeader>;
    /// @brief Helper type to enable the constructor for the producer, i.e. when T has a non const qualifier
    template <typename S, typename TT>
    using ForProducerOnly = typename Base_t::template ForProducerOnly<S, TT>;

    /// @brief Helper type to enable the constructor for the consumer, i.e. when T has a const qualifier
    template <typename S, typename TT>
    using ForConsumerOnly = typename Base_t::template ForConsumerOnly<S, TT>;

  public:
    /// @brief Constructor for a Response used by the Producer
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param responseUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    /// @param producer is a reference to the producer to be able to use the `send` and `release` methods
    template <typename S = T, typename = ForProducerOnly<S, T>>
    Response(cxx::unique_ptr<T>&& responseUniquePtr, RpcInterface<Response<T>>& Producer) noexcept;

    /// @brief Constructor for a Response used by the Subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param responseUniquePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    template <typename S = T, typename = ForConsumerOnly<S, T>>
    Response(cxx::unique_ptr<T>&& responseUniquePtr) noexcept;

    ~Response() noexcept = default;

    Response<T>& operator=(Response<T>&& rhs) noexcept = default;
    Response(Response<T>&& rhs) noexcept = default;

    Response(const Response<T>&) = delete;
    Response<T>& operator=(const Response<T>&) = delete;

    ///
    /// @brief Retrieve the response header of the underlying memory chunk loaned to the response.
    /// @return The response header of the underlying memory chunk.
    ///
    ResponseHeader& getResponseHeader() noexcept;

    ///
    /// @brief Retrieve the response header of the underlying memory chunk loaned to the response.
    /// @return The response header of the underlying memory chunk.
    ///
    const ResponseHeader& getResponseHeader() const noexcept;

    ///
    /// @brief send the response via the producer from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = ForProducerOnly<S, T>>
    void send() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/response.inl"

#endif // IOX_POSH_POPO_RESPONSE_HPP
