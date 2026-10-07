// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

// The *_unchecked operations must never call the overflow handler.  For i8 and
// i16 the arithmetic is done in int, so an overflowing result is truncated
// (wraps) rather than being undefined.
namespace {
	template<typename Integer>
	void test_unchecked_no_handler( ) {
		auto const minimum = Integer::min( );
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1 };
		auto const two = Integer{ 2 };

		daw_ensure( maximum.add_unchecked( one ) == minimum );
		daw_ensure( minimum.sub_unchecked( one ) == maximum );
		daw_ensure( maximum.mul_unchecked( two ) == Integer{ -2 } );
		daw_ensure( minimum.div_unchecked( Integer{ -1 } ) == minimum );
		daw_ensure( minimum.negate_unchecked( ) == minimum );
	}

	template<typename Integer>
	constexpr bool test_unchecked_in_range( ) {
		auto const a = Integer{ 12 };
		auto const b = Integer{ -5 };
		return a.add_unchecked( b ) == Integer{ 7 } and
		       a.sub_unchecked( b ) == Integer{ 17 } and
		       a.mul_unchecked( b ) == Integer{ -60 } and
		       a.div_unchecked( b ) == Integer{ -2 } and
		       a.rem_unchecked( b ) == Integer{ 2 } and
		       a.negate_unchecked( ) == Integer{ -12 };
	}
} // namespace

static_assert( test_unchecked_in_range<daw::i8>( ) );
static_assert( test_unchecked_in_range<daw::i16>( ) );
static_assert( test_unchecked_in_range<daw::i32>( ) );
static_assert( test_unchecked_in_range<daw::i64>( ) );

int main( ) {
	auto handler_count = 0;
	auto handler = [&]( daw::integers::SignedIntegerErrorType ) {
		++handler_count;
	};
	daw::integers::register_signed_overflow_handler( handler );
	daw::integers::register_signed_div_by_zero_handler( handler );

	test_unchecked_no_handler<daw::i8>( );
	test_unchecked_no_handler<daw::i16>( );
	daw_ensure( handler_count == 0 );
}
