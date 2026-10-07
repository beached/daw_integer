// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

// Built once per DAW_DEFAULT_SIGNED_CHECKING mode (0, 1, 2) by CMake.  The
// default operators must produce the wrapped result in every mode.  Mode 0
// additionally reports each overflow to the handler; modes 1 and 2 never do.
// min / -1 is not tested as it is intentionally unchecked in modes 1 and 2.

#if not defined( DAW_DEFAULT_SIGNED_CHECKING )
#error DAW_DEFAULT_SIGNED_CHECKING must be set by the build
#endif

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

namespace {
	inline constexpr bool reports_overflow = DAW_DEFAULT_SIGNED_CHECKING == 0;

	template<typename Integer>
	constexpr bool test_in_range( ) {
		auto r = Integer{ 12 };
		r += Integer{ 3 };
		r -= Integer{ 5 };
		r *= Integer{ -3 };
		r /= Integer{ 4 };
		r %= Integer{ 5 };
		r <<= Integer{ 2 };
		r >>= Integer{ 1 };
		++r;
		--r;
		auto const n = -r;
		auto const s = Integer{ 7 } + Integer{ -9 };
		return r == Integer{ -4 } and n == Integer{ 4 } and s == Integer{ -2 };
	}

	template<typename Integer>
	void test_wrapping( int &expected_overflow_count ) {
		auto const minimum = Integer::min( );
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1 };
		auto const two = Integer{ 2 };
		auto const overflowed = [&] {
			if( reports_overflow ) {
				++expected_overflow_count;
			}
		};

		auto r = maximum;
		r += one;
		daw_ensure( r == minimum );
		overflowed( );

		r = minimum;
		r -= one;
		daw_ensure( r == maximum );
		overflowed( );

		r = maximum;
		r *= two;
		daw_ensure( r == Integer{ -2 } );
		overflowed( );

		r = maximum;
		r *= maximum;
		daw_ensure( r == one );
		overflowed( );

		r = maximum;
		++r;
		daw_ensure( r == minimum );
		overflowed( );

		r = minimum;
		--r;
		daw_ensure( r == maximum );
		overflowed( );

		daw_ensure( -minimum == minimum );
		overflowed( );

		daw_ensure( maximum + one == minimum );
		overflowed( );

		daw_ensure( minimum - one == maximum );
		overflowed( );

		daw_ensure( maximum * two == Integer{ -2 } );
		overflowed( );
	}
} // namespace

static_assert( test_in_range<daw::i8>( ) );
static_assert( test_in_range<daw::i16>( ) );
static_assert( test_in_range<daw::i32>( ) );
static_assert( test_in_range<daw::i64>( ) );

int main( ) {
	auto actual_overflow_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++actual_overflow_count;
	};
	daw::integers::register_signed_overflow_handler( overflow_handler );

	daw_ensure( test_in_range<daw::i8>( ) );
	daw_ensure( test_in_range<daw::i16>( ) );
	daw_ensure( test_in_range<daw::i32>( ) );
	daw_ensure( test_in_range<daw::i64>( ) );

	auto expected_overflow_count = 0;
	test_wrapping<daw::i8>( expected_overflow_count );
	test_wrapping<daw::i16>( expected_overflow_count );
	test_wrapping<daw::i32>( expected_overflow_count );
	test_wrapping<daw::i64>( expected_overflow_count );
	daw_ensure( actual_overflow_count == expected_overflow_count );
}
