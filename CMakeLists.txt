cmake_minimum_required(VERSION 3.20)

project(hydrogen)

# If CMake configured a missing make program (e.g. /usr/bin/gmake not present),
# try to find a usable 'make' and set CMAKE_MAKE_PROGRAM so builds work.
if(NOT CMAKE_MAKE_PROGRAM OR CMAKE_MAKE_PROGRAM STREQUAL "CMAKE_MAKE_PROGRAM-NOTFOUND")
  find_program(MAKE_PROG make)
  if(MAKE_PROG)
    message(STATUS "Found make: ${MAKE_PROG} — setting CMAKE_MAKE_PROGRAM fallback")
    set(CMAKE_MAKE_PROGRAM "${MAKE_PROG}" CACHE FILEPATH "Make program" FORCE)
  endif()
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(hydro src/main.cpp)
