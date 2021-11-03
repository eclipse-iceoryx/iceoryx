# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
# Copyright (c) 2021 by Timo Röhling. All rights reserved.
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

# setup_package_name_and_create_files : this macro which is called from other modules which use iceoryx_hoofs
# sets the variables for package version file,config file used for configuration
# this also creates the config files
include(GNUInstallDirs)

Macro(setup_package_name_and_create_files)
    set(options )
    set(oneValueArgs NAME NAMESPACE PROJECT_PREFIX)
    set(multiValueArgs)
    cmake_parse_arguments(PARAMS "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
    # set variables for library export
    set(PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_NAME}ConfigVersion.cmake" )
    set(PACKAGE_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_NAME}Config.cmake" )
    set(TARGETS_EXPORT_NAME "${PARAMS_NAME}Targets" )
    set(PROJECT_NAMESPACE ${PARAMS_NAMESPACE} )

    set(DESTINATION_BINDIR ${CMAKE_INSTALL_BINDIR})
    set(DESTINATION_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    set(DESTINATION_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/${PARAMS_PROJECT_PREFIX})
    set(DESTINATION_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PARAMS_NAME})

    # create package files
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${PACKAGE_VERSION_FILE}
        COMPATIBILITY AnyNewerVersion
    )
    configure_package_config_file(
    "cmake/Config.cmake.in"
    ${PACKAGE_CONFIG_FILE}
    INSTALL_DESTINATION ${DESTINATION_CONFIGDIR}
    )
endMacro()

# setup_install_directories_and_export_package : this macro route the call for installation
# of target and include directory, package version file , config file

Macro(setup_install_directories_and_export_package)
    set(options)
    set(oneValueArgs INCLUDE_DIRECTORY )
    set(multiValueArgs TARGETS)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
    install_target_directories_and_header(
    TARGETS ${INSTALL_TARGETS}
    INCLUDE_DIRECTORY ${INSTALL_INCLUDE_DIRECTORY}
    )
    install_package_files_and_export()
endMacro()

# install_target_directories_and_header : this macro does the installation
# of target and include directory

Macro(install_target_directories_and_header)
    set(options)
    set(oneValueArgs INCLUDE_DIRECTORY )
    set(multiValueArgs TARGETS)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})
    # target directories
    install(
    TARGETS ${INSTALL_TARGETS}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION ${DESTINATION_BINDIR} COMPONENT bin
    LIBRARY DESTINATION ${DESTINATION_LIBDIR} COMPONENT bin
    ARCHIVE DESTINATION ${DESTINATION_LIBDIR} COMPONENT bin
    )

    # header
    install(
    DIRECTORY ${INSTALL_INCLUDE_DIRECTORY}
    DESTINATION ${DESTINATION_INCLUDEDIR}
    COMPONENT dev
    )
endMacro()

# install_package_files_and_export : this macro does the installation
# of package version and config file and also export the package

Macro(install_package_files_and_export)
   # package files
    install(
    FILES ${PACKAGE_VERSION_FILE} ${PACKAGE_CONFIG_FILE}
    DESTINATION ${DESTINATION_CONFIGDIR}
    )

    # package export
    install(
    EXPORT ${TARGETS_EXPORT_NAME}
    NAMESPACE ${PROJECT_NAMESPACE}::
    DESTINATION ${DESTINATION_CONFIGDIR}
    )
endMacro()
