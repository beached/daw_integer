// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

namespace {
	template<typename Integer>
	constexpr bool test_overflowing( ) {
		using value_t = typename Integer::value_type;
		auto const minimum = Integer::min( );
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1 };
		auto const two = Integer{ 2 };

		auto const add_ok = Integer{ 3 }.add_overflowing( Integer{ 4 } );
		auto const add_ov = maximum.add_overflowing( one );
		auto const sub_ok = Integer{ 3 }.sub_overflowing( Integer{ 4 } );
		auto const sub_ov = minimum.sub_overflowing( one );
		auto const mul_ok = Integer{ 3 }.mul_overflowing( Integer{ -4 } );
		auto const mul_ov = maximum.mul_overflowing( two );

		return add_ok.value == value_t{ 7 } and not add_ok.overflowed and
		       add_ov.value == minimum.value( ) and add_ov.overflowed and
		       sub_ok.value == value_t{ -1 } and not sub_ok.overflowed and
		       sub_ov.value == maximum.value( ) and sub_ov.overflowed and
		       mul_ok.value == value_t{ -12 } and not mul_ok.overflowed and
		       mul_ov.value == value_t{ -2 } and mul_ov.overflowed;
	}
} // namespace

static_assert( test_overflowing<daw::i8>( ) );
static_assert( test_overflowing<daw::i16>( ) );
static_assert( test_overflowing<daw::i32>( ) );
static_assert( test_overflowing<daw::i64>( ) );

int main( ) {
	daw_ensure( test_overflowing<daw::i8>( ) );
	daw_ensure( test_overflowing<daw::i16>( ) );
	daw_ensure( test_overflowing<daw::i32>( ) );
	daw_ensure( test_overflowing<daw::i64>( ) );
}
