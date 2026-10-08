// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

// Built once per DAW_DEFAULT_UNSIGNED_CHECKING mode (0, 1, 2) by CMake.  The
// default operators must produce the wrapped result in every mode.  Mode 0
// additionally reports each overflow to the handler; modes 1 and 2 never do.

#if not defined( DAW_DEFAULT_UNSIGNED_CHECKING )
#error DAW_DEFAULT_UNSIGNED_CHECKING must be set by the build
#endif

#include <daw/integers/daw_unsigned.h>

#include <daw/daw_ensure.h>

namespace {
	inline constexpr bool reports_overflow = DAW_DEFAULT_UNSIGNED_CHECKING == 0;

	template<typename Integer>
	constexpr bool test_in_range( ) {
		auto r = Integer{ 12U };
		r += Integer{ 3U };
		r -= Integer{ 5U };
		r *= Integer{ 3U };
		r /= Integer{ 4U };
		r %= Integer{ 5U };
		r <<= Integer{ 2U };
		r >>= Integer{ 1U };
		++r;
		--r;
		auto const s = Integer{ 7U } + Integer{ 9U };
		return r == Integer{ 4U } and s == Integer{ 16U };
	}

	template<typename Integer>
	void test_wrapping( int &expected_overflow_count ) {
		auto const minimum = Integer::min( );
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1U };
		auto const two = Integer{ 2U };
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
		daw_ensure( r == maximum - one );
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

		daw_ensure( maximum + one == minimum );
		overflowed( );

		daw_ensure( minimum - one == maximum );
		overflowed( );

		daw_ensure( maximum * two == maximum.sub_wrapped( one ) );
		overflowed( );
	}
} // namespace

static_assert( test_in_range<daw::u8>( ) );
static_assert( test_in_range<daw::u16>( ) );
static_assert( test_in_range<daw::u32>( ) );
static_assert( test_in_range<daw::u64>( ) );

int main( ) {
	auto actual_overflow_count = 0;
	auto overflow_handler =
	  [&]( daw::integers::IntegerErrorType error_type ) {
		  daw_ensure( error_type ==
		              daw::integers::IntegerErrorType::Overflow );
		  ++actual_overflow_count;
	  };
	daw::integers::register_integer_overflow_handler( overflow_handler );

	daw_ensure( test_in_range<daw::u8>( ) );
	daw_ensure( test_in_range<daw::u16>( ) );
	daw_ensure( test_in_range<daw::u32>( ) );
	daw_ensure( test_in_range<daw::u64>( ) );

	auto expected_overflow_count = 0;
	test_wrapping<daw::u8>( expected_overflow_count );
	test_wrapping<daw::u16>( expected_overflow_count );
	test_wrapping<daw::u32>( expected_overflow_count );
	test_wrapping<daw::u64>( expected_overflow_count );
	daw_ensure( actual_overflow_count == expected_overflow_count );
}
