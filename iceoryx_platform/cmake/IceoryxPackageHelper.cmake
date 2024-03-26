# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
# Copyright (c) 2021 by Timo Röhling. All rights reserved.
# Copyright (c) 2023 by NXP. All rights reserved.
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

function(set_global)
    set(arguments VAR )
    set(multiValueArgs VALUE)
    cmake_parse_arguments(IOX "" "${arguments}" "${multiValueArgs}" ${ARGN} )

    set(${IOX_VAR} ${IOX_VALUE} CACHE INTERNAL "${IOX_VAR}")
endfunction()

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
    set(multiValueArgs TARGETS INCLUDE_DIRECTORIES)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
    install_target_directories_and_header(
    TARGETS ${INSTALL_TARGETS}
    INCLUDE_DIRECTORIES ${INSTALL_INCLUDE_DIRECTORIES}
    )
    install_package_files_and_export()
endMacro()

# install_target_directories_and_header : this macro does the installation
# of target and include directory

Macro(install_target_directories_and_header)
    set(options)
    set(multiValueArgs TARGETS INCLUDE_DIRECTORIES)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})
    # target directories
    install(
    TARGETS ${INSTALL_TARGETS}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION ${DESTINATION_BINDIR} COMPONENT bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin
    )

    # header
    install(
    DIRECTORY ${INSTALL_INCLUDE_DIRECTORIES}
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
    set(switches IS_EXECUTABLE)
    set(arguments TARGET)
    cmake_parse_arguments(IOX "${switches}" "${arguments}" "" ${ARGN} )

    if( APPLE )
        if ( IOX_IS_EXECUTABLE )
            set(IOX_RPATH_PREFIX "@executable_path")
        else()
            set(IOX_RPATH_PREFIX "@loader_path")
        endif()
    elseif ( LINUX OR UNIX )
        set(IOX_RPATH_PREFIX "\$ORIGIN")
    endif()

    set_property(
        TARGET ${IOX_TARGET}
        PROPERTY INSTALL_RPATH
        "${IOX_RPATH_PREFIX}/../${CMAKE_INSTALL_LIBDIR}"
    )

    set_property(
        TARGET ${IOX_TARGET}
        PROPERTY BUILD_RPATH
            # @todo iox-#1287 implement rpath auto detection to have no dependency on posh at this level.
            "${IOX_RPATH_PREFIX}/../iceoryx_hoofs"
            "${IOX_RPATH_PREFIX}/../iceoryx_posh"
            "${IOX_RPATH_PREFIX}/../iceoryx_platform"
            "${IOX_RPATH_PREFIX}/../iceoryx_binding_c"
            # @todo iox-#1287 END

            # @todo iox-#1287 to be compatible with our current iceoryx_meta structure where we have build/posh build/hoofs build/binding_c
            "${IOX_RPATH_PREFIX}/../hoofs"
            "${IOX_RPATH_PREFIX}/../posh"
            "${IOX_RPATH_PREFIX}/../platform"
            "${IOX_RPATH_PREFIX}/../binding_c"
            # @todo iox-#1287 to be compatible with our current iceoryx_meta structure where the examples are again in a subfolder, build/iceoryx_examples/example_name
            "${IOX_RPATH_PREFIX}/../../hoofs"
            "${IOX_RPATH_PREFIX}/../../posh"
            "${IOX_RPATH_PREFIX}/../../platform"
            "${IOX_RPATH_PREFIX}/../../binding_c"
            # @todo iox-#1287 iox-roudi is stored directly in build, despite it should be stored in iceoryx_posh, adjust paths so that this works too
            "${IOX_RPATH_PREFIX}/hoofs"
            "${IOX_RPATH_PREFIX}/posh"
            "${IOX_RPATH_PREFIX}/platform"
            "${IOX_RPATH_PREFIX}/binding_c"
            # @todo iox-#1287 END
    )
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
    set(switches USE_C_LANGUAGE PLACE_IN_BUILD_ROOT)
    set(arguments TARGET STACK_SIZE)
    set(multiArguments FILES LIBS INCLUDE_DIRECTORIES LIBS_QNX LIBS_LINUX LIBS_UNIX LIBS_WIN32 LIBS_APPLE
        BUILD_INTERFACE INSTALL_INTERFACE)
    cmake_parse_arguments(IOX "${switches}" "${arguments}" "${multiArguments}" ${ARGN} )

    add_executable(${IOX_TARGET} ${IOX_FILES})
    target_include_directories(${IOX_TARGET} PRIVATE ${IOX_INCLUDE_DIRECTORIES})
    target_link_libraries(${IOX_TARGET} ${IOX_LIBS})

    if ( QNX )
        target_link_libraries(${IOX_TARGET} ${IOX_LIBS_QNX})
    elseif ( LINUX )
        target_link_libraries(${IOX_TARGET} ${IOX_LIBS_LINUX})
    elseif ( APPLE )
        target_link_libraries(${IOX_TARGET} ${IOX_LIBS_APPLE})
    elseif ( WIN32 )
        target_link_libraries(${IOX_TARGET} ${IOX_LIBS_WIN32})
    elseif ( UNIX )
        target_link_libraries(${IOX_TARGET} ${IOX_LIBS_UNIX})
    endif()

    # @todo iox-#1287 lasting fix for rpath without implicit posh dependencies and auto lib detection
    ### iox_set_rpath( IS_EXECUTABLE TARGET ${IOX_TARGET} )

    if ( IOX_PLACE_IN_BUILD_ROOT )
        set_target_properties(${IOX_TARGET} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
    endif()

    if ( IOX_USE_C_LANGUAGE )
        iox_set_file_language( USE_C_LANGUAGE FILES ${IOX_FILES} )
    else()
        iox_set_file_language( FILES ${IOX_FILES} )
    endif()

    if ( IOX_USE_C_LANGUAGE )
        target_compile_options(${IOX_TARGET} PRIVATE ${ICEORYX_C_FLAGS} ${ICEORYX_C_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${ICEORYX_GRCOV_FLAGS})
    else()
        target_compile_options(${IOX_TARGET} PRIVATE ${ICEORYX_CXX_FLAGS} ${ICEORYX_CXX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${ICEORYX_GRCOV_FLAGS})
    endif()

    if ( IOX_STACK_SIZE )
        if(APPLE)
            # @todo iox-#1287 not yet supported
        elseif(WIN32)
            if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
                target_link_options(${IOX_TARGET} BEFORE PRIVATE /STACK:${IOX_STACK_SIZE})
            elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                target_link_options(${IOX_TARGET} BEFORE PRIVATE -Wl,--stack,${IOX_STACK_SIZE})
            else()
            endif()
        elseif(QNX OR LINUX OR UNIX)
            target_link_options(${IOX_TARGET} BEFORE PRIVATE -Wl,-z,stack-size=${IOX_STACK_SIZE})
        else()
        endif()
    endif()

    set_target_properties( ${IOX_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON )

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

    install( TARGETS ${IOX_TARGET} RUNTIME DESTINATION bin)
endMacro()

Macro(iox_add_library)
    set(switches USE_C_LANGUAGE NO_EXPORT NO_PACKAGE_SETUP NO_FIND_PACKAGE_SUPPORT STATIC)
    set(arguments TARGET NAMESPACE PROJECT_PREFIX)
    set(multiArguments FILES PUBLIC_LIBS PRIVATE_LIBS BUILD_INTERFACE
        INSTALL_INTERFACE ADDITIONAL_EXPORT_TARGETS
        PUBLIC_INCLUDES PRIVATE_INCLUDES
        PUBLIC_LIBS_LINUX PRIVATE_LIBS_LINUX PUBLIC_LIBS_QNX PRIVATE_LIBS_QNX
        PUBLIC_LIBS_UNIX PRIVATE_LIBS_UNIX PUBLIC_LIBS_WIN32 PRIVATE_LIBS_WIN32
        PUBLIC_LIBS_APPLE PRIVATE_LIBS_APPLE PUBLIC_LIBS_FREERTOS PRIVATE_LIBS_FREERTOS
        EXPORT_INCLUDE_DIRS)
    cmake_parse_arguments(IOX "${switches}" "${arguments}" "${multiArguments}" ${ARGN} )

    if ( NOT IOX_NO_PACKAGE_SETUP )
        setup_package_name_and_create_files(
            NAME ${IOX_TARGET}
            NAMESPACE ${IOX_NAMESPACE}
            PROJECT_PREFIX ${IOX_PROJECT_PREFIX}
        )
    endif()

    if ( NOT IOX_NO_FIND_PACKAGE_SUPPORT )
        ########## find_package in source tree ##########
        set(${IOX_TARGET}_DIR ${PROJECT_SOURCE_DIR}/cmake
            CACHE FILEPATH
            "${IOX_TARGET}Config.cmake to make find_package(${IO_TARGET}) work in source tree!"
            FORCE
        )
    endif()

    if ( IOX_STATIC )
        add_library( ${IOX_TARGET} STATIC ${IOX_FILES} )
    else()
        add_library( ${IOX_TARGET} ${IOX_FILES} )
    endif()

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

    set_target_properties(${IOX_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    set_target_properties( ${IOX_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON )

    if ( IOX_USE_C_LANGUAGE )
        target_compile_options(${IOX_TARGET} PRIVATE ${ICEORYX_C_FLAGS} ${ICEORYX_C_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${ICEORYX_GRCOV_FLAGS})
    else()
        target_compile_options(${IOX_TARGET} PRIVATE ${ICEORYX_CXX_FLAGS} ${ICEORYX_CXX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS} ${ICEORYX_GRCOV_FLAGS})
    endif()
    target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS} PRIVATE ${IOX_PRIVATE_LIBS})
    target_include_directories(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_INCLUDES} PRIVATE ${IOX_PRIVATE_INCLUDES})

    if ( LINUX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_LINUX} PRIVATE ${IOX_PRIVATE_LIBS_LINUX})
    elseif ( APPLE )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_APPLE} PRIVATE ${IOX_PRIVATE_LIBS_APPLE})
    elseif ( QNX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_QNX} PRIVATE ${IOX_PRIVATE_LIBS_QNX})
    elseif ( UNIX )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_UNIX} PRIVATE ${IOX_PRIVATE_LIBS_UNIX})
    elseif ( WIN32 )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_WIN32} PRIVATE ${IOX_PRIVATE_LIBS_WIN32})
    elseif ( FREERTOS )
        target_link_libraries(${IOX_TARGET} PUBLIC ${IOX_PUBLIC_LIBS_FREERTOS} PRIVATE ${IOX_PRIVATE_LIBS_FREERTOS})
        # PIC can cause NULL function pointers on bare metal since there is no dynamic linker...
        set_target_properties( ${IOX_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE OFF )
    endif ( LINUX )

    # @todo iox-#1287 lasting fix for rpath without implicit posh dependencies and auto lib detection
    ### iox_set_rpath( TARGET ${IOX_TARGET} )

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

    if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE )
        install(
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
            DESTINATION share/doc/${IOX_TARGET}
            COMPONENT dev
        )
    endif()

    if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/3rd_party_licenses )
        install(
          DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/3rd_party_licenses
          DESTINATION share/doc/${IOX_TARGET}
          COMPONENT dev)
    endif()

    if ( NOT IOX_NO_EXPORT )
        setup_install_directories_and_export_package(
            TARGETS ${IOX_TARGET} ${IOX_ADDITIONAL_EXPORT_TARGETS}
            INCLUDE_DIRECTORIES ${IOX_EXPORT_INCLUDE_DIRS}
        )
    endif()

    unset(IOX_NO_PACKAGE_SETUP)
    unset(IOX_NO_FIND_PACKAGE_SUPPORT)
endMacro()
