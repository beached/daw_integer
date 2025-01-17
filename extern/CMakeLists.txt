# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/daw_json_link
#

# This prevents tests running on dependencies
option( DAW_ENABLE_TESTING "Build unit tests" OFF )
if( DAW_ENABLE_TESTING )
    set( DAW_ENABLE_TESTING OFF )
    set( DAW_EXTERN_USE_LATEST ON )
endif()

include( FetchContent )

if( DAW_EXTERN_USE_LATEST )
    # Use the latest header_libraries and not release
    FetchContent_Declare(
            daw_header_libraries
            GIT_REPOSITORY https://github.com/beached/header_libraries.git
            GIT_TAG v2
    )
    message( STATUS "Using latest header_libraries" )
else()
    FetchContent_Declare(
            daw_header_libraries
            GIT_REPOSITORY https://github.com/beached/header_libraries.git
						GIT_SHALLOW 1
            GIT_TAG master
    )
endif()

FetchContent_MakeAvailable( daw_header_libraries )

