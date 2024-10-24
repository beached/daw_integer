# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/daw_integer
#

include( cmake/test_compiler_options.cmake )

add_library( daw_integer_test_lib INTERFACE )
target_link_libraries( daw_integer_test_lib INTERFACE daw::daw-integer )
target_include_directories( daw_integer_test_lib INTERFACE include/ )

add_executable( signed_test_bin src/daw_integers_signed_test.cpp )
target_link_libraries( signed_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_test_bin COMMAND signed_test_bin )

