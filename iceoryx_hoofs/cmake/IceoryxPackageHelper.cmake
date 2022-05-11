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
        "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
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

Macro(iox_set_rpath)
    set(arguments TARGET RPATH )
    cmake_parse_arguments(IOX "" "" "${arguments}" ${ARGN} )

    if ( LINUX OR UNIX )
        set_target_properties(
            ${IOX_TARGET}
            PROPERTIES
            BUILD_RPATH ${IOX_RPATH}
            INSTALL_RPATH ${IOX_RPATH}
        )
    elseif( APPLE )
        set_target_properties(
                    ${IOX_TARGET}
                    PROPERTIES
                    BUILD_RPATH ${IOX_RPATH}
                    INSTALL_RPATH ${IOX_RPATH}
                )
    endif( LINUX OR UNIX )
endMacro()

Macro(iox_set_file_language)
    set(switches USE_C_LANGUAGE)
    set(multiArguments FILES)
    cmake_parse_arguments(IOX "${switches}" "" "${multiArguments}" ${ARGN} )

    if ( IOX_USE_C_LANGUAGE )
        set_source_files_properties(${IOX_FILES} PROPERTIES LANGUAGE C)
        set_target_properties(
            ${IOX_TARGET}
          PROPERTIES
            C_STANDARD_REQUIRED ON
            C_STANDARD 11
        )
    else()
        set_source_files_properties(${IOX_FILES} PROPERTIES LANGUAGE CXX)
        set_target_properties(
            ${IOX_TARGET}
          PROPERTIES
            CXX_STANDARD_REQUIRED ON
            CXX_STANDARD ${ICEORYX_CXX_STANDARD}
        )
    endif()
endMacro()

Macro(iox_add_executable)
    set(switches USE_C_LANGUAGE)
    set(arguments TARGET STACK_SIZE)
    set(multiArguments FILES LIBS INCLUDE_DIRECTORY LIBS_QNX)
    cmake_parse_arguments(IOX "${switches}" "${arguments}" "${multiArguments}" ${ARGN} )

    add_executable(${IOX_TARGET} ${IOX_FILES})
    target_include_directories(${IOX_TARGET} PRIVATE ${IOX_INCLUDE_DIRECTORY})
    target_link_libraries(${IOX_TARGET} PRIVATE ${IOX_LIBS})

    if ( QNX )
        target_link_libraries(${IOX_TARGET} PRIVATE ${IOX_LIBS_QNX})
    endif()

    set(IOX_WARNINGS ${ICEORYX_WARNINGS})

    if ( IOX_USE_C_LANGUAGE )
        iox_set_file_language( USE_C_LANGUAGE FILES ${IOX_FILES} )
    else()
        iox_set_file_language( FILES ${IOX_FILES} )
    endif()

    if ( IOX_USE_C_LANGUAGE )
        if("-Wno-noexcept-type" IN_LIST IOX_WARNINGS)
            list(REMOVE_ITEM IOX_WARNINGS "-Wno-noexcept-type")
        endif()
    endif()

    target_compile_options(${IOX_TARGET} PRIVATE ${IOX_WARNINGS} ${ICEORYX_SANITIZER})

    if ( IOX_STACK_SIZE )
        if(WIN32)
            target_link_options(single_process BEFORE PRIVATE /STACK:${IOX_STACK_SIZE})
        else()
            target_link_options(single_process BEFORE PRIVATE -Wl,-z,stack-size=${IOX_STACK_SIZE})
        endif()
    endif()

    set_target_properties( ${IOX_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON )

    install( TARGETS ${IOX_TARGET} RUNTIME DESTINATION bin)
endMacro()

Macro(iox_add_library)
    set(switches USE_C_LANGUAGE NO_EXPORT NO_PACKAGE_SETUP NO_FIND_PACKAGE_SUPPORT)
    set(arguments TARGET NAMESPACE PROJECT_PREFIX)
    set(multiArguments RPATH FILES PUBLIC_LIBS PRIVATE_LIBS BUILD_INTERFACE
        INSTALL_INTERFACE ADDITIONAL_EXPORT_TARGETS
        PUBLIC_LIBS_LINUX PRIVATE_LIBS_LINUX PUBLIC_LIBS_QNX PRIVATE_LIBS_QNX
        PUBLIC_LIBS_UNIX PRIVATE_LIBS_UNIX PUBLIC_LIBS_WIN32 PRIVATE_LIBS_WIN32
        PUBLIC_LIBS_APPLE PRIVATE_LIBS_APPLE)
    cmake_parse_arguments(IOX "${switches}" "${arguments}" "${multiArguments}" ${ARGN} )

    if ( NOT IOX_NO_PACKAGE_SETUP )
        message("setup ${IOX_TARGET}")
        setup_package_name_and_create_files(
            NAME ${IOX_TARGET}
            NAMESPACE ${IOX_NAMESPACE}
            PROJECT_PREFIX ${IOX_PROJECT_PREFIX}
        )
    endif()

    if ( NOT IOX_NO_FIND_PACKAGE_SUPPORT )
        ########## find_package in source tree ##########
        set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake
            CACHE FILEPATH
            "${PROJECT_NAME}Config.cmake to make find_package(${PROJECT_NAME}) work in source tree!"
            FORCE
        )
    endif()

    add_library( ${IOX_TARGET} ${IOX_FILES} )

    if ( IOX_NAMESPACE )
        add_library( ${IOX_NAMESPACE}::${IOX_TARGET} ALIAS ${IOX_TARGET})
    endif()

    if ( IOX_USE_C_LANGUAGE )
        iox_set_file_language( USE_C_LANGUAGE FILES ${IOX_FILES} )
    else()
        iox_set_file_language( FILES ${IOX_FILES} )
    endif()

    if ( IOX_USE_C_LANGUAGE )
        if("-Wno-noexcept-type" IN_LIST IOX_WARNINGS)
            list(REMOVE_ITEM IOX_WARNINGS "-Wno-noexcept-type")
        endif()
    endif()

    target_compile_options(${IOX_TARGET} PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})
    target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS} PRIVATE ${IOX_PRIVATE_LIBS})

    if ( LINUX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_LINUX} PRIVATE ${IOX_PRIVATE_LIBS_LINUX})
    elseif ( QNX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_QNX} PRIVATE ${IOX_PRIVATE_LIBS_QNX})
    elseif ( UNIX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_UNIX} PRIVATE ${IOX_PRIVATE_LIBS_UNIX})
    elseif ( WIN32 )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_WIN32} PRIVATE ${IOX_PRIVATE_LIBS_WIN32})
    elseif ( APPLE )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_APPLE} PRIVATE ${IOX_PRIVATE_LIBS_APPLE})
    endif ( LINUX )

    iox_set_rpath( TARGET ${IOX_TARGET} RPATH ${IOX_RPATH} )

    foreach(INTERFACE ${IOX_BUILD_INTERFACE})
        target_include_directories(${IOX_TARGET}
            PUBLIC
            $<BUILD_INTERFACE:${INTERFACE}>
        )
    endforeach()

    foreach(INTERFACE ${IOX_INSTALL_INTERFACE})
        target_include_directories(${IOX_TARGET}
            PUBLIC
            $<INSTALL_INTERFACE:${INTERFACE}>
        )
    endforeach()

    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        DESTINATION share/doc/${IOX_TARGET}
        COMPONENT dev
    )

    if ( NOT IOX_NO_EXPORT )
        setup_install_directories_and_export_package(
            TARGETS ${IOX_TARGET} ${IOX_ADDITIONAL_EXPORT_TARGETS}
            INCLUDE_DIRECTORY include/
        )
    endif()
endMacro()
