// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_CLIENT_HPP
#define IOX_POSH_POPO_TYPED_CLIENT_HPP

#include "iceoryx_posh/popo/base_client.hpp"
#include "iceoryx_posh/popo/rpc_interface.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"

namespace iox
{
namespace popo
{

template <typename RequestType, typename ResponseType, typename BaseClient_t = BaseClient<>>
class ClientImpl : public BaseClient_t, public RpcInterface<Request<RequestType>>
{
    static_assert(!std::is_const<RequestType>::value, "The type `RequestType` must not be const.");
    static_assert(!std::is_reference<RequestType>::value, "The type `RequestType` must not be a reference.");
    static_assert(!std::is_pointer<RequestType>::value, "The type `RequestType` must not be a pointer.");

    static_assert(!std::is_const<ResponseType>::value, "The type `ResponseType` must not be const.");
    static_assert(!std::is_reference<ResponseType>::value, "The type `ResponseType` must not be a reference.");
    static_assert(!std::is_pointer<ResponseType>::value, "The type `ResponseType` must not be a pointer.");

  public:
    ClientImpl(const capro::ServiceDescription& service,
                  const ClientOptions& clientOptions = ClientOptions());
    ClientImpl(const ClientImpl& other) = delete;
    ClientImpl& operator=(const ClientImpl&) = delete;
    ClientImpl(ClientImpl&& rhs) = default;
    ClientImpl& operator=(ClientImpl&& rhs) = default;
    virtual ~ClientImpl() = default;

    ///
    /// @brief loan Get a sample from loaned shared memory and construct the data with the given arguments.
    /// @param args Arguments used to construct the data.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to
    /// loan.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    template <typename... Args>
    cxx::expected<Request<RequestType>, AllocationError> loan(Args&&... args) noexcept;

    cxx::expected<Response<const ResponseType>, ChunkReceiveResult> take() noexcept;

    ///
    /// @brief send Sends the given sample and then releases its loan.
    /// @param request The request to send.
    ///
    void send(Request<RequestType>&& request) noexcept override;

  protected:
    using BaseClient_t::port;

  private:
    Request<RequestType> convertRequestHeaderToRequest(RequestHeader* const header) noexcept;

    cxx::expected<Request<RequestType>, AllocationError> loanRequest() noexcept;

    using ClientRequestDeleter = SampleDeleter<typename BaseClient_t::PortType>;
    ClientRequestDeleter m_requestDeleter{port()};
};

template <typename RequestType, typename ResponseType>
using Client = ClientImpl<RequestType, ResponseType>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/client.inl"

#endif // IOX_POSH_POPO_TYPED_CLIENT_HPP
