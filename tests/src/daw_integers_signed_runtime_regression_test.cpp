// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

static_assert( ( 7_i32 - 2_i32 ) == 5_i32 );

int main( ) {
	bool overflow = false;
	auto handler = [&]( daw::integers::SignedIntegerErrorType error_type ) {
		overflow = error_type == daw::integers::SignedIntegerErrorType::Overflow;
	};
	daw::integers::register_signed_overflow_handler( handler );

	auto result = daw::i32::max( ).div_wrapped( -1_i32 );
	daw_ensure( result == -daw::i32::max( ) );
	daw_ensure( not overflow );

	overflow = false;
	result = daw::i32::min( ).div_wrapped( -1_i32 );
	daw_ensure( result == daw::i32::min( ) );
	daw_ensure( not overflow );

	overflow = false;
	result = ( 1_i32 ).shl_checked( 0_i32 );
	daw_ensure( result == 1_i32 );
	daw_ensure( not overflow );

	overflow = false;
	result = ( 1_i32 ).shr_checked( 0_i32 );
	daw_ensure( result == 1_i32 );
	daw_ensure( not overflow );

	overflow = false;
	auto const sub_result = ( 7_i16 ).sub_wrapped( 2_i16 );
	daw_ensure( sub_result == 5_i16 );
	daw_ensure( not overflow );

	overflow = false;
	auto const mul_result = ( -2_i64 ).mul_checked( 3_i64 );
	daw_ensure( mul_result == -6_i64 );
	daw_ensure( not overflow );
}
