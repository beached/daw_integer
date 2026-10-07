// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

namespace {
	template<typename Integer>
	constexpr bool test_representable_negate( ) {
		auto const seven = Integer{ 7 };
		auto const minus_seven = Integer{ -7 };
		auto const zero = Integer{ 0 };
		return seven.negate_checked( ) == minus_seven and
		       minus_seven.negate_checked( ) == seven and
		       zero.negate_checked( ) == zero and -seven == minus_seven and
		       Integer::max( ).negate_checked( ) == Integer::min( ) + Integer{ 1 };
	}

	template<typename Integer>
	void test_minimum_negate( int &overflow_count ) {
		auto const minimum = Integer::min( );

		daw_ensure( minimum.negate_checked( ) == minimum );
		++overflow_count;

		daw_ensure( -minimum == minimum );
		++overflow_count;

		daw_ensure( minimum.negate_wrapped( ) == minimum );
		daw_ensure( minimum.negate_saturated( ) == Integer::max( ) );
	}
} // namespace

static_assert( test_representable_negate<daw::i8>( ) );
static_assert( test_representable_negate<daw::i16>( ) );
static_assert( test_representable_negate<daw::i32>( ) );
static_assert( test_representable_negate<daw::i64>( ) );

int main( ) {
	auto actual_overflow_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++actual_overflow_count;
	};
	daw::integers::register_signed_overflow_handler( overflow_handler );

	auto expected_overflow_count = 0;
	test_minimum_negate<daw::i8>( expected_overflow_count );
	test_minimum_negate<daw::i16>( expected_overflow_count );
	test_minimum_negate<daw::i32>( expected_overflow_count );
	test_minimum_negate<daw::i64>( expected_overflow_count );
	daw_ensure( actual_overflow_count == expected_overflow_count );
}
