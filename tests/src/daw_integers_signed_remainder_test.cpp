// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

static_assert( ( 5_i32 % 2_i32 ) == 1_i32 );
static_assert( ( 5_i32 ).rem_checked( 2_i32 ) == 1_i32 );
static_assert( ( 5_i32 ).rem_saturated( 2_i32 ) == 1_i32 );
static_assert( daw::i32::min( ).rem_saturated( -1_i32 ) == 0_i32 );

int main( ) {
	auto value = 5_i32;
	value %= 2_i32;
	daw_ensure( value == 1_i32 );

	bool divide_by_zero = false;
	auto divide_by_zero_handler =
	  [&]( daw::integers::SignedIntegerErrorType error_type ) {
		  divide_by_zero =
		    error_type == daw::integers::SignedIntegerErrorType::DivideByZero;
	  };
	daw::integers::register_signed_div_by_zero_handler( divide_by_zero_handler );

	(void)( 5_i32 ).rem_checked( 0_i32 );
	daw_ensure( divide_by_zero );

	bool overflow = false;
	auto overflow_handler =
	  [&]( daw::integers::SignedIntegerErrorType error_type ) {
		  overflow = error_type == daw::integers::SignedIntegerErrorType::Overflow;
	  };
	daw::integers::register_signed_overflow_handler( overflow_handler );

	auto const overflow_result = daw::i32::min( ).rem_checked( -1_i32 );
	daw_ensure( overflow );
	daw_ensure( overflow_result == 0_i32 );
}
