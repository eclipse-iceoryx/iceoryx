# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

set(CC arm-linux-gnueabihf)
set(CMAKE_LIBRARY_ARCHITECTURE ${CC})
set(TOOLCHAIN_PREFIX ${CC})
set(CMAKE_SYSROOT "/home/kenny/work/image/debian-11.3-iot-armhf-2022-05-10/rootfs/")

# specify the cross compiler
SET(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# compiler tools
find_program(CMAKE_AR ${TOOLCHAIN_PREFIX}-gcc-ar)
find_program(CMAKE_GDB ${TOOLCHAIN_PREFIX}-gdb)
find_program(CMAKE_LD ${TOOLCHAIN_PREFIX}-ld)
find_program(CMAKE_LINKER ${TOOLCHAIN_PREFIX}-ld)
find_program(CMAKE_NM ${TOOLCHAIN_PREFIX}-gcc-nm)
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}-objcopy)
find_program(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}-objdump)
find_program(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}-gcc-ranlib)
find_program(CMAKE_STRIP ${TOOLCHAIN_PREFIX}-strip)

# Linker
set (COMMON_LINKER_FLAGS "-Wl,-rpath-link,${CMAKE_SYSROOT}/lib/arm-linux-gnueabihf:${CMAKE_SYSROOT}/lib:${CMAKE_SYSROOT}/usr/lib -Wl,-L${CMAKE_SYSROOT}/lib")
set (CMAKE_EXE_LINKER_FLAGS "${COMMON_LINKER_FLAGS} -Wl,--start-group -ldl -lrt" CACHE STRING "EXE LD Flags" FORCE)
set (CMAKE_SHARED_LINKER_FLAGS "${COMMON_LINKER_FLAGS}" CACHE STRING "MODULE LD Flags" FORCE)
set (CMAKE_MODULE_LINKER_FLAGS "${COMMON_LINKER_FLAGS}" CACHE STRING "MODULE LD Flags" FORCE)
