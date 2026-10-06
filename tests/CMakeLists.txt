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

add_executable( signed_test_bin src/daw_integers_signed_test.cpp )
target_link_libraries( signed_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_test_bin COMMAND signed_test_bin )

add_executable( signed_contract_test_bin
                src/daw_integers_signed_contract_test.cpp )
target_link_libraries( signed_contract_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_contract_test_bin COMMAND signed_contract_test_bin )

add_executable( signed_pow_test_bin src/daw_integers_signed_pow_test.cpp )
target_link_libraries( signed_pow_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_pow_test_bin COMMAND signed_pow_test_bin )

add_executable( signed_remainder_test_bin
                src/daw_integers_signed_remainder_test.cpp )
target_link_libraries( signed_remainder_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_remainder_test_bin COMMAND signed_remainder_test_bin )

add_executable( signed_runtime_regression_test_bin
                src/daw_integers_signed_runtime_regression_test.cpp )
target_link_libraries( signed_runtime_regression_test_bin
                       PRIVATE daw_integer_test_lib )
add_test( NAME signed_runtime_regression_test_bin
          COMMAND signed_runtime_regression_test_bin )

add_executable( signed_shift_test_bin
                src/daw_integers_signed_shift_test.cpp )
target_link_libraries( signed_shift_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_shift_test_bin COMMAND signed_shift_test_bin )

add_executable( signed_sign_test_bin src/daw_integers_signed_sign_test.cpp )
target_link_libraries( signed_sign_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_sign_test_bin COMMAND signed_sign_test_bin )
