// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

// shl_unchecked/shr_unchecked and both overloads of shl_overflowing/
// shr_overflowing.  The overflowing variants mask the shift count to the bit
// width (like Rust's overflowing_shl) and never report lost bits.
namespace {
	template<typename Integer>
	void test_shift_ops( int &expected_overflow_count ) {
		using value_t = typename Integer::value_type;
		constexpr int bits = static_cast<int>( sizeof( value_t ) * 8U );

		daw_ensure( Integer{ 3 }.shl_unchecked( Integer{ 2 } ) == Integer{ 12 } );
		daw_ensure( Integer{ 12 }.shr_unchecked( Integer{ 2 } ) == Integer{ 3 } );
		daw_ensure( Integer{ -8 }.shr_unchecked( Integer{ 1 } ) == Integer{ -4 } );

		// signed_integer shift count
		daw_ensure( Integer{ 3 }.shl_overflowing( Integer{ 2 } ) == Integer{ 12 } );
		daw_ensure( Integer{ 12 }.shr_overflowing( Integer{ 2 } ) == Integer{ 3 } );
		daw_ensure( Integer{ 3 }.shl_overflowing( Integer{ bits + 1 } ) ==
		            Integer{ 6 } );
		daw_ensure( Integer{ 12 }.shr_overflowing( Integer{ bits + 1 } ) ==
		            Integer{ 6 } );
		// Bits shifted out are discarded, not reported
		daw_ensure( Integer::max( ).shl_overflowing( Integer{ 1 } ) ==
		            Integer{ -2 } );

		// raw integer shift count
		daw_ensure( Integer{ 3 }.shl_overflowing( 2 ) == Integer{ 12 } );
		daw_ensure( Integer{ 12 }.shr_overflowing( 2 ) == Integer{ 3 } );
		daw_ensure( Integer{ 3 }.shl_overflowing( bits + 1 ) == Integer{ 6 } );
		daw_ensure( Integer::max( ).shl_overflowing( 1 ) == Integer{ -2 } );
		daw_ensure( Integer{ 1 }.shl_overflowing( bits - 1 ) == Integer::min( ) );

		// Negative counts are reported and leave the value unchanged
		daw_ensure( Integer{ 3 }.shl_overflowing( Integer{ -1 } ) == Integer{ 3 } );
		daw_ensure( Integer{ 3 }.shr_overflowing( -1 ) == Integer{ 3 } );
		expected_overflow_count += 2;
	}
} // namespace

int main( ) {
	auto actual_overflow_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++actual_overflow_count;
	};
	daw::integers::register_signed_overflow_handler( overflow_handler );

	auto expected_overflow_count = 0;
	test_shift_ops<daw::i8>( expected_overflow_count );
	test_shift_ops<daw::i16>( expected_overflow_count );
	test_shift_ops<daw::i32>( expected_overflow_count );
	test_shift_ops<daw::i64>( expected_overflow_count );
	daw_ensure( actual_overflow_count == expected_overflow_count );
}
