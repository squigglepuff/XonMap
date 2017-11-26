cmake_minimum_required(VERSION 2.8)

project(xonmap)

# Versioning.
set(XONMAP_VER_MAJ 1)
set(XONMAP_VER_MIN 0)
set(XONMAP_VER_REV 0)
set(XONMAP_VER_BUILD 00000001)

# Sources for MISC.
FILE(GLOB_RECURSE Hdrs "include/*.h")
set(${MODULE_PREFIX}_HDRS ${${MODULE_PREFIX}_HDRS} ${Hdrs})

FILE(GLOB_RECURSE Srcs "src/*.cpp")
set(${MODULE_PREFIX}_SRCS ${${MODULE_PREFIX}_SRCS} ${Srcs})
###

# Add a search path for the inclusion of headers.
include_directories(${PROJECT_SOURCE_DIR}/include)

add_custom_target(Headers ${${MODULE_PREFIX}_HDRS})
add_executable(${PROJECT_NAME} ${${MODULE_PREFIX}_SRCS})

# Some system info dump.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR WIN32)
    message("Target system is Microsoft Windows.")
    add_definitions(-DQ_OS_WINDOWS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR UNIX)
    message("Target system is Linux (or variant).")
    add_definitions(-DQ_OS_LINUX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR APPLE)
    message("Target system is Apple macOS.")
    add_definitions(-DQ_OS_OSX)
else (CMAKE_SYSTEM_NAME NOT STREQUAL "Windows" AND CMAKE_SYSTEM_NAME NOT STREQUAL "Linux" AND CMAKE_SYSTEM_NAME NOT STREQUAL "Darwin")
    message("Unable to determine target system (hope this works)!")
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR WIN32)

if (DEFINED MSVC_VERSION)
    message("Building using the Microsoft Visual Studio toolchain (Version: ${MSVC_VERSION}).")
elseif (MINGW)
    message("Building using the GNU (MinGW) toolchain.")
elseif (CYGWIN)
    message("Building using GNU (CygWin)toolchain.")
elseif (DEFINED XCODE_VERSION)
    message("Building using the Apple XCode toolchain (Version: ${XCODE_VERSION}).")
elseif (BORLAND)
    message("Building using the Borland toolchain.")
else (NOT DEFINED MSVC_VERSION AND NOT MINGW AND NOT CYGWIN AND NOT XCODE_VERSION AND NOT BORLAND)
    message("Building using an unknown toolchain!")
endif(DEFINED MSVC_VERSION)

message("System Name: ${CMAKE_SYSTEM_NAME}")
message("System CPU: ${CMAKE_SYSTEM_PROCESSOR}")
message("CMake System: ${CMAKE_SYSTEM}")
message("System Version: ${CMAKE_SYSTEM_VERSION}")
message("Build type: ${CMAKE_BUILD_TYPE}")
message("Xonmap Version: ${XONMAP_VER_MAJ}.${XONMAP_VER_MIN} r${XONMAP_VER_REV} (${XONMAP_VER_BUILD})")

# Setup the install step.
