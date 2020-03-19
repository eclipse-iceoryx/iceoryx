# Find GIT revisions
execute_process(COMMAND
  git describe --match=None --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE ICEORYX_SHA1
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND
  date -u -R
  OUTPUT_VARIABLE ICEORYX_BUILDDATE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/iceoryx_versions.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/iceoryx/include/iceoryx_versions.hpp" @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/generated/iceoryx/include/iceoryx_versions.hpp
  DESTINATION include/${PREFIX}
  COMPONENT dev)
