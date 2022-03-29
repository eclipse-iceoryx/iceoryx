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
#ifndef IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL
#define IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL

#include "iceoryx_hoofs/error_handling/error_handler.hpp"

namespace iox
{
template <typename Error>
inline void errorHandler(const Error error, const ErrorLevel level) noexcept
{
    ErrorHandler::handler(
        static_cast<typename std::underlying_type<Error>::type>(error), asStringLiteral(error), level);
}

template <typename Error>
auto errorToStringIndex(Error error) noexcept
{
    return static_cast<typename std::underlying_type<Error>::type>(error)
           - static_cast<typename std::underlying_type<Error>::type>(Error::NO_ERROR) - 1;
}

} // namespace iox

#endif // IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL
