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

#ifndef IOX_POSH_POPO_REQUEST_HPP
#define IOX_POSH_POPO_REQUEST_HPP

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/internal/popo/smart_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/rpc_header.hpp"


namespace iox
{
namespace popo
{
template <typename T, typename H>
class ResponseInterface;

template <typename T, typename H = cxx::add_const_conditionally_t<mepoo::NoUserHeader, T>>
class Response : public SmartChunk<ResponseInterface, T, H>
{
    using BaseType = SmartChunk<ResponseInterface, T, H>;

    template <typename S, typename TT>
    using ForClientOnly = typename BaseType::template ForProducerOnly<S, TT>;

  public:
    using BaseType::BaseType;

    template <typename S = T, typename = ForClientOnly<S, T>>
    void send() noexcept;

    ResponseHeader* getResponseHeader() noexcept;
    const ResponseHeader* getResponseHeader() const noexcept;

  private:
    template <typename, typename, typename>
    friend class ClientImpl;

    using BaseType::release;

    using BaseType::m_members;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/response.inl"
#endif
