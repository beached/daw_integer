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

add_executable( signed_abs_test_bin src/daw_integers_signed_abs_test.cpp )
target_link_libraries( signed_abs_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_abs_test_bin COMMAND signed_abs_test_bin )

add_executable( signed_euclidean_test_bin
                src/daw_integers_signed_euclidean_test.cpp )
target_link_libraries( signed_euclidean_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_euclidean_test_bin COMMAND signed_euclidean_test_bin )

add_executable( signed_div_rem_overflowing_test_bin
                src/daw_integers_signed_div_rem_overflowing_test.cpp )
target_link_libraries( signed_div_rem_overflowing_test_bin
                       PRIVATE daw_integer_test_lib )
add_test( NAME signed_div_rem_overflowing_test_bin
          COMMAND signed_div_rem_overflowing_test_bin )

add_executable( signed_negate_test_bin src/daw_integers_signed_negate_test.cpp )
target_link_libraries( signed_negate_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_negate_test_bin COMMAND signed_negate_test_bin )

add_executable( signed_unchecked_test_bin src/daw_integers_signed_unchecked_test.cpp )
target_link_libraries( signed_unchecked_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_unchecked_test_bin COMMAND signed_unchecked_test_bin )

add_executable( signed_rotate_test_bin src/daw_integers_signed_rotate_test.cpp )
target_link_libraries( signed_rotate_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_rotate_test_bin COMMAND signed_rotate_test_bin )

add_executable( signed_arith_overflowing_test_bin src/daw_integers_signed_arith_overflowing_test.cpp )
target_link_libraries( signed_arith_overflowing_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_arith_overflowing_test_bin COMMAND signed_arith_overflowing_test_bin )

add_executable( signed_shift_ops_test_bin src/daw_integers_signed_shift_ops_test.cpp )
target_link_libraries( signed_shift_ops_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_shift_ops_test_bin COMMAND signed_shift_ops_test_bin )

add_executable( signed_numeric_limits_test_bin src/daw_integers_signed_numeric_limits_test.cpp )
target_link_libraries( signed_numeric_limits_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_numeric_limits_test_bin COMMAND signed_numeric_limits_test_bin )

add_executable( signed_constexpr_checked_test_bin src/daw_integers_signed_constexpr_checked_test.cpp )
target_link_libraries( signed_constexpr_checked_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME signed_constexpr_checked_test_bin COMMAND signed_constexpr_checked_test_bin )

# Same source built in each DAW_DEFAULT_SIGNED_CHECKING mode
foreach( mode 0 1 2 )
  add_executable( signed_checking_mode_${mode}_test_bin
                  src/daw_integers_signed_checking_modes_test.cpp )
  target_link_libraries( signed_checking_mode_${mode}_test_bin
                         PRIVATE daw_integer_test_lib )
  target_compile_definitions( signed_checking_mode_${mode}_test_bin
                              PRIVATE DAW_DEFAULT_SIGNED_CHECKING=${mode} )
  add_test( NAME signed_checking_mode_${mode}_test_bin
            COMMAND signed_checking_mode_${mode}_test_bin )
endforeach( )

add_executable( unsigned_test_bin src/daw_integers_unsigned_test.cpp )
target_link_libraries( unsigned_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME unsigned_test_bin COMMAND unsigned_test_bin )

# Same source built in each DAW_DEFAULT_UNSIGNED_CHECKING mode
foreach( mode 0 1 2 )
  add_executable( unsigned_checking_mode_${mode}_test_bin
                  src/daw_integers_unsigned_checking_modes_test.cpp )
  target_link_libraries( unsigned_checking_mode_${mode}_test_bin
                         PRIVATE daw_integer_test_lib )
  target_compile_definitions( unsigned_checking_mode_${mode}_test_bin
                              PRIVATE DAW_DEFAULT_UNSIGNED_CHECKING=${mode} )
  add_test( NAME unsigned_checking_mode_${mode}_test_bin
            COMMAND unsigned_checking_mode_${mode}_test_bin )
endforeach( )

add_executable( sign_conversion_test_bin src/daw_integers_sign_conversion_test.cpp )
target_link_libraries( sign_conversion_test_bin PRIVATE daw_integer_test_lib )
add_test( NAME sign_conversion_test_bin COMMAND sign_conversion_test_bin )
