// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2024 by Michael Bentley <mikebentley15@gmail.com>. All rights reserved.
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

#ifndef IOX_BINDING_C_C2CPP_BINDING_H
#define IOX_BINDING_C_C2CPP_BINDING_H

#ifdef __cplusplus

#include <cstdint>

#define IOX_C_CLASS class

#else

#include <stdbool.h>
#include <stdint.h>

#define IOX_C_CLASS struct

#endif

#endif
