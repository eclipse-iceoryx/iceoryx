# Copyright (c) 2026 by Functionhx <2994114386@qq.com>. All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Apache Software License 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
# which is available at https://opensource.org/licenses/MIT.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT

#[=======================================================================[.rst:
Findacl
-------

Finds the ACL (Access Control Lists) library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported target, if found:

``acl::libacl``
  The ACL library

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``acl_FOUND``
  True if the ACL library was found
``acl_INCLUDE_DIRS``
  Include directories for ACL headers
``acl_LIBRARIES``
  Libraries to link against

#]=======================================================================]

find_path(
  acl_INCLUDE_DIR
  NAMES sys/acl.h
  DOC "ACL include directory"
)

find_library(
  acl_LIBRARY
  NAMES acl
  DOC "ACL library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(acl
  REQUIRED_VARS acl_LIBRARY acl_INCLUDE_DIR
)

set(acl_INCLUDE_DIRS "${acl_INCLUDE_DIR}")
set(acl_LIBRARIES "${acl_LIBRARY}")

if(acl_FOUND AND NOT TARGET acl::libacl)
  add_library(acl::libacl UNKNOWN IMPORTED)
  set_target_properties(acl::libacl PROPERTIES
    IMPORTED_LOCATION "${acl_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${acl_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(acl_INCLUDE_DIR acl_LIBRARY)
