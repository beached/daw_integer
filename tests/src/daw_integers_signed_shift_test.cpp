// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

static_assert( ( 3_i32 ).shl_checked( 0_i32 ) == 3_i32 );
static_assert( ( 3_i32 ).shl_checked( 2_i32 ) == 12_i32 );
static_assert( ( 12_i32 ).shr_checked( 0_i32 ) == 12_i32 );
static_assert( ( 12_i32 ).shr_checked( 2_i32 ) == 3_i32 );

int main( ) {
	auto overflow_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++overflow_count;
	};
	daw::integers::register_signed_overflow_handler( overflow_handler );

	auto result = ( 3_i32 ).shl_checked( 0_i32 );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 0 );

	result = ( 3_i32 ).shl_checked( 2_i32 );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 0 );

	result = ( 12_i32 ).shr_checked( 0_i32 );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 0 );

	result = ( 12_i32 ).shr_checked( 2_i32 );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 0 );

	result = ( 3_i32 ).shl_checked( -1_i32 );
	daw_ensure( result == 1_i32 );
	daw_ensure( overflow_count == 1 );

	result = ( -3_i32 ).shl_checked( -1_i32 );
	daw_ensure( result == -2_i32 );
	daw_ensure( overflow_count == 2 );

	result = ( 12_i32 ).shr_checked( -1_i32 );
	daw_ensure( result == 24_i32 );
	daw_ensure( overflow_count == 3 );

	result = ( 3_i32 ).shl_checked( 32_i32 );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 4 );

	result = ( 12_i32 ).shr_checked( 32_i32 );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 5 );

	result = ( 3_i32 ).shl_checked( 40_i32 );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 6 );

	result = ( 12_i32 ).shr_checked( 40_i32 );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 7 );

	result = ( 3_i32 ).shl_checked( -32_i32 );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 8 );

	result = ( 12_i32 ).shr_checked( -32_i32 );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 9 );

	result = ( 3_i32 ).shl_checked( daw::i32::min( ) );
	daw_ensure( result == 3_i32 );
	daw_ensure( overflow_count == 10 );

	result = ( 12_i32 ).shr_checked( daw::i32::min( ) );
	daw_ensure( result == 12_i32 );
	daw_ensure( overflow_count == 11 );
}
