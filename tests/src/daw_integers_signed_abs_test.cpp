// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

namespace {
	template<typename Integer>
	constexpr bool test_representable_abs( ) {
		auto const zero = Integer{ 0 };
		auto const positive = Integer{ 7 };
		auto const negative = Integer{ -7 };
		auto const maximum = Integer::max( );

		return zero.abs( ) == zero and zero.abs_checked( ) == zero and
		       zero.abs_wrapped( ) == zero and zero.abs_saturated( ) == zero and
		       positive.abs( ) == positive and
		       positive.abs_checked( ) == positive and
		       positive.abs_wrapped( ) == positive and
		       positive.abs_saturated( ) == positive and
		       negative.abs( ) == positive and
		       negative.abs_checked( ) == positive and
		       negative.abs_wrapped( ) == positive and
		       negative.abs_saturated( ) == positive and
		       maximum.abs( ) == maximum and maximum.abs_checked( ) == maximum and
		       maximum.abs_wrapped( ) == maximum and
		       maximum.abs_saturated( ) == maximum;
	}

	template<typename Integer>
	void test_minimum_abs( int &overflow_count ) {
		auto const minimum = Integer::min( );
		auto const maximum = Integer::max( );

		daw_ensure( minimum.abs_wrapped( ) == minimum );
		daw_ensure( minimum.abs_saturated( ) == maximum );

		auto const checked = minimum.abs_checked( );
		daw_ensure( checked == minimum );
		++overflow_count;

		auto const debug_checked = minimum.abs( );
		daw_ensure( debug_checked == minimum );
		++overflow_count;
	}
} // namespace

static_assert( test_representable_abs<daw::i8>( ) );
static_assert( test_representable_abs<daw::i16>( ) );
static_assert( test_representable_abs<daw::i32>( ) );
static_assert( test_representable_abs<daw::i64>( ) );

static_assert( daw::i8::min( ).abs_wrapped( ) == daw::i8::min( ) );
static_assert( daw::i16::min( ).abs_wrapped( ) == daw::i16::min( ) );
static_assert( daw::i32::min( ).abs_wrapped( ) == daw::i32::min( ) );
static_assert( daw::i64::min( ).abs_wrapped( ) == daw::i64::min( ) );

static_assert( daw::i8::min( ).abs_saturated( ) == daw::i8::max( ) );
static_assert( daw::i16::min( ).abs_saturated( ) == daw::i16::max( ) );
static_assert( daw::i32::min( ).abs_saturated( ) == daw::i32::max( ) );
static_assert( daw::i64::min( ).abs_saturated( ) == daw::i64::max( ) );

int main( ) {
	auto actual_overflow_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++actual_overflow_count;
	};
	daw::integers::register_signed_overflow_handler( overflow_handler );

	auto expected_overflow_count = 0;
	test_minimum_abs<daw::i8>( expected_overflow_count );
	test_minimum_abs<daw::i16>( expected_overflow_count );
	test_minimum_abs<daw::i32>( expected_overflow_count );
	test_minimum_abs<daw::i64>( expected_overflow_count );
	daw_ensure( actual_overflow_count == expected_overflow_count );
}
