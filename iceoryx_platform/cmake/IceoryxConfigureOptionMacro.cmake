# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

# define macro for option configuration
macro(configure_option)
    set(ONE_VALUE_ARGS NAME DEFAULT_VALUE)
    cmake_parse_arguments(CONFIGURE_OPTION "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if(NOT ${CONFIGURE_OPTION_NAME})
        set(${CONFIGURE_OPTION_NAME} ${CONFIGURE_OPTION_DEFAULT_VALUE})
    endif()
    message(STATUS "[i] ${CONFIGURE_OPTION_NAME}: " ${${CONFIGURE_OPTION_NAME}})
endmacro()
