# This file is part of CMake-argp.
#
# CMake-argp is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with this program. If not, see
#
#  http://www.gnu.org/licenses/
#
#
# Copyright (c)
#   2016 Alexander Haase <ahaase@alexhaase.de>
#

include(FindPackageHandleStandardArgs)
include(CheckFunctionExists)


# Check if argp is included in libc:
set(CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET TRUE)

check_function_exists("argp_parse" ARGP_IN_LIBC)
if (ARGP_IN_LIBC)
	set(ARGP_LIBRARIES "c" CACHE STRING "ARGP libraries.")

elseif (NOT ARGP_IN_LIBC)
	unset(ARGP_IN_LIBC CACHE)

	find_library(ARGP_LIB "argp")
	find_path(ARGP_INCLUDE_DIR argp.h PATH_SUFFIXES include)
	mark_as_advanced(ARGP_LIB)
	if (ARGP_LIB)
		set(CMAKE_REQUIRED_LIBRARIES "${ARGP_LIB}")
		check_function_exists("argp_parse" ARGP_EXTERNAL)
		if (ARGP_EXTERNAL)
			set(ARGP_LIBRARIES "${ARGP_LIB}" CACHE STRING "ARGP libraries.")
		endif ()
	endif ()
endif ()

set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_SAVE})


find_package_handle_standard_args(argp
	FOUND_VAR ARGP_FOUND
	REQUIRED_VARS ARGP_LIBRARIES
)



MESSAGE(STATUS "Find Header Directory for argp: " ${ARGP_INCLUDE_DIR})
MESSAGE(STATUS "Find Dynamic Library for arpg:  " ${ARGP_LIBRARIES})

