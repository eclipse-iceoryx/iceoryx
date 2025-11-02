# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#
########## dummyConfig.cmake to be able to use find_package with the source tree ##########
#

if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FOUND_PRINTED)
    message(STATUS "The package '${CMAKE_FIND_PACKAGE_NAME}' is used in source code version.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND_PRINTED true CACHE INTERNAL "")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
