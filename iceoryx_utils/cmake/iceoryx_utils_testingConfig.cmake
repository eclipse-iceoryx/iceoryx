#
########## dummyConfig.cmake to be able to use find_package with the source tree ##########
#

if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FOUND_PRINTED)
    message(STATUS "The package '${CMAKE_FIND_PACKAGE_NAME}' is used in source code version.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND_PRINTED true CACHE INTERNAL "")
endif()
