// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace {
	// Reference rotate done on the unsigned representation.  n is taken modulo
	// the bit width, matching Rust's rotate_left/rotate_right.
	template<typename Integer>
	constexpr Integer reference_rotate( Integer v, std::size_t n, bool left ) {
		using value_t = typename Integer::value_type;
		using unsigned_t = std::make_unsigned_t<value_t>;
		constexpr std::size_t bits = sizeof( value_t ) * 8U;
		n %= bits;
		auto const u = static_cast<unsigned_t>( v.value( ) );
		if( n == 0 ) {
			return v;
		}
		auto const r =
		  left ? static_cast<unsigned_t>( ( u << n ) | ( u >> ( bits - n ) ) )
		       : static_cast<unsigned_t>( ( u >> n ) | ( u << ( bits - n ) ) );
		return Integer( static_cast<value_t>( r ) );
	}

	template<typename Integer>
	void test_rotate( ) {
		using value_t = typename Integer::value_type;
		constexpr std::size_t bits = sizeof( value_t ) * 8U;
		Integer const values[] = { Integer{ 0 },  Integer{ 1 },   Integer{ -1 },
		                           Integer{ -2 }, Integer{ 0x5A }, Integer::min( ),
		                           Integer::max( ) };
		for( auto v : values ) {
			for( std::size_t n = 0; n <= 2 * bits; ++n ) {
				daw_ensure( v.rotate_left( n ) == reference_rotate( v, n, true ) );
				daw_ensure( v.rotate_right( n ) == reference_rotate( v, n, false ) );
			}
		}
	}
} // namespace

int main( ) {
	auto handler_count = 0;
	auto handler = [&]( daw::integers::SignedIntegerErrorType ) {
		++handler_count;
	};
	daw::integers::register_signed_overflow_handler( handler );

	daw_ensure( daw::i8{ 1 }.rotate_right( 1 ) == daw::i8::min( ) );
	daw_ensure( daw::i32::min( ).rotate_right( 1 ) == daw::i32{ 1 << 30 } );
	daw_ensure( daw::i32::min( ).rotate_left( 1 ) == daw::i32{ 1 } );

	test_rotate<daw::i8>( );
	test_rotate<daw::i16>( );
	test_rotate<daw::i32>( );
	test_rotate<daw::i64>( );
	daw_ensure( handler_count == 0 );
}
