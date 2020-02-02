// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#define ACL_USER_OBJ 0
#define ACL_USER 1
#define ACL_GROUP_OBJ 2
#define ACL_GROUP 3
#define ACL_OTHER 4
#define ACL_READ 5
#define ACL_WRITE 6

using acl_t = int;
using acl_permset_t = int;
using acl_perm_t = int;
