// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

static_assert( ( 2_i32 ).pow( 3 ) == 8_i32 );
static_assert( ( 2_i32 ).pow_unchecked( 3 ) == 8_i32 );
static_assert( ( 2_i32 ).pow_checked( 3 ) == 8_i32 );
static_assert( ( 2_i32 ).pow_wrapped( 3 ) == 8_i32 );
static_assert( ( 2_i32 ).pow_saturated( 3 ) == 8_i32 );
static_assert( ( -4_i32 ).pow_saturated( 3 ) == -64_i32 );
static_assert( ( -4_i32 ).pow_saturated( 4 ) == 256_i32 );

int main( ) {
	bool overflow = false;
	auto handler = [&]( daw::integers::SignedIntegerErrorType error_type ) {
		overflow = error_type == daw::integers::SignedIntegerErrorType::Overflow;
	};
	daw::integers::register_signed_overflow_handler( handler );

	auto const result = daw::i32::max( ).pow_checked( 1 );
	daw_ensure( result == daw::i32::max( ) );
	daw_ensure( not overflow );

	overflow = false;
	auto const larger_result = ( 1000_i32 ).pow_checked( 3 );
	daw_ensure( larger_result == 1'000'000'000_i32 );
	daw_ensure( not overflow );
}
