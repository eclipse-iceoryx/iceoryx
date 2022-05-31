# Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

cmake_minimum_required(VERSION 3.16)

if(LINUX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/linux/)
elseif(QNX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/qnx/)
elseif(APPLE)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/mac/)
elseif(WIN32)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/win/)
elseif(UNIX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/unix/)
else()
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/unix/)
    message(WARNING "Could not detect supported platform, but I'm feeling lucky today. Maybe its Unix." )
endif()

set(ICEORYX_PLATFORM ${ICEORYX_PLATFORM} CACHE PATH "" FORCE)

# this must be recurse since every platform has potentially different cpp files
file ( GLOB_RECURSE ICEORYX_PLATFORM_FILES
    ${ICEORYX_PLATFORM}/source/*.cpp
)

iox_add_library(
    NO_EXPORT
    NO_PACKAGE_SETUP
    NO_FIND_PACKAGE_SUPPORT
    TARGET                      iceoryx_platform
    NAMESPACE                   iceoryx_hoofs
    BUILD_INTERFACE             ${ICEORYX_PLATFORM}/include/
    INSTALL_INTERFACE           include/${PREFIX}
    PUBLIC_LIBS_LINUX           rt pthread
    PUBLIC_LIBS_UNIX            rt pthread
    PRIVATE_LIBS_QNX            socket
    FILES                       ${ICEORYX_PLATFORM_FILES}
)

install(
    DIRECTORY ${ICEORYX_PLATFORM}/include/
    DESTINATION include/${PREFIX}
    COMPONENT dev
)
