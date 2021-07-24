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

#ifndef IOX_POSH_POPO_RESPONSE_INL
#define IOX_POSH_POPO_RESPONSE_INL

namespace iox
{
namespace popo
{
template <typename T>
template <typename S, typename>
inline Response<T>::Response(cxx::unique_ptr<T>&& responseUniquePtr, RpcInterface<T>& producer) noexcept
    : Base_t(std::move(responseUniquePtr), producer)
{
}

template <typename T>
template <typename S, typename>
inline Response<T>::Response(cxx::unique_ptr<T>&& responseUniquePtr) noexcept
    : Base_t(std::move(responseUniquePtr))
{
}

template <typename T>
inline ResponseHeader& Response<T>::getResponseHeader() noexcept
{
    return *static_cast<ResponseHeader*>(mepoo::ChunkHeader::fromUserPayload(Base_t::m_members.smartchunkUniquePtr.get())->userHeader());
}

template <typename T>
inline const ResponseHeader& Response<T>::getResponseHeader() const noexcept
{
    return const_cast<Response<T>*>(this)->getResponseHeader();
}

template <typename T>
template <typename S, typename>
inline void Response<T>::send() noexcept
{
    if (Base_t::m_members.smartchunkUniquePtr)
    {
        Base_t::m_members.transmitterRef.get().send(std::move(*this));
    }
    else
    {
        LogError() << "Tried to send empty Response! Might be an already sent or moved Response!";
        errorHandler(Error::kPOSH__SENDING_EMPTY_RESPONSE, nullptr, ErrorLevel::MODERATE);
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_RESPONSE_INL
