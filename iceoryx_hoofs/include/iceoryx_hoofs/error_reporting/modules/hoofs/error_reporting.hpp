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

#ifndef IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERROR_REPORTING_HPP

// Each module (= some unit with its own errors) must provide the following.

// 1. Define the errors of the module
#include "iceoryx_hoofs/error_reporting/modules/hoofs/errors.hpp"

// 2. Include the custom reporting implementation
#include "iceoryx_hoofs/error_reporting/custom/error_reporting.hpp"

// 3. Include the error reporting macro API
#include "iceoryx_hoofs/error_reporting/error_reporting_macros.hpp"

#endif
