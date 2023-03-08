// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler_interface.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace err
{

/// @brief Defines the default reaction of dynamic error handling.
/// The default reaction is to do nothing apart from logging and termination on panic.
/// As this is common for all error handling of the given platform, this happens in the
/// reporting API before the (polymorphic) custom behavior is invoked.
class DefaultHandler : public ErrorHandlerInterface
{
  public:
    DefaultHandler() = default;
    ~DefaultHandler() override = default;
    DefaultHandler(const DefaultHandler&) = delete;
    DefaultHandler(DefaultHandler&&) = delete;
    DefaultHandler& operator=(const DefaultHandler&) = delete;
    DefaultHandler& operator=(DefaultHandler&&) = delete;

    /// @brief Defines the reaction on panic.
    void panic() override;

    /// @brief Defines the reaction on error.
    /// @param location the location of the error
    /// @param desc error descriptor
    void reportError(ErrorDescriptor desc) override;

    /// @brief Defines the reaction onviolation.
    /// @param location the location of the violation
    /// @param desc error descriptor
    void reportViolation(ErrorDescriptor desc) override;
};

} // namespace err
} // namespace iox

#endif
