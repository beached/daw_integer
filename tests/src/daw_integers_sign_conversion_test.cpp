// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#include <daw/daw_integer.h>

#include <daw/daw_ensure.h>

#include <type_traits>

static_assert( std::is_same_v<decltype( daw::i8{ }.as_unsigned( ) ), daw::u8> );
static_assert( std::is_same_v<decltype( daw::i64{ }.as_unsigned( ) ), daw::u64> );
static_assert( std::is_same_v<decltype( daw::u16{ }.as_signed( ) ), daw::i16> );
static_assert( std::is_same_v<decltype( daw::u32{ }.as_signed( ) ), daw::i32> );

namespace {
	template<typename Signed, typename Unsigned>
	constexpr bool test_casts( ) {
		bool r = true;
		r &= Signed{ 0 }.as_unsigned( ) == Unsigned{ 0U };
		r &= Signed{ 42 }.as_unsigned( ) == Unsigned{ 42U };
		r &= Unsigned{ 42U }.as_signed( ) == Signed{ 42 };
		r &= Unsigned{ 0U }.as_signed( ) == Signed{ 0 };
		// Out of range values keep the two's complement bits, like static_cast
		r &= Signed{ -1 }.as_unsigned( ) == Unsigned::max( );
		r &= Unsigned::max( ).as_signed( ) == Signed{ -1 };
		r &= Signed::min( ).as_unsigned( ) ==
		     Unsigned( Unsigned::max( ).value( ) / 2U + 1U );
		r &= Signed::min( ).as_unsigned( ).as_signed( ) == Signed::min( );
		r &= Signed::max( ).as_unsigned( ).as_signed( ) == Signed::max( );
		r &= Unsigned::max( ).as_signed( ).as_unsigned( ) == Unsigned::max( );
		return r;
	}

	template<typename Signed, typename Unsigned>
	constexpr bool test_exact_in_range( ) {
		bool r = true;
		r &= Signed{ 0 }.as_exact_unsigned( ) == Unsigned{ 0U };
		r &= Signed{ 42 }.as_exact_unsigned( ) == Unsigned{ 42U };
		r &= Unsigned{ 42U }.as_exact_signed( ) == Signed{ 42 };
		r &= Signed::max( ).as_exact_unsigned( ).as_exact_signed( ) ==
		     Signed::max( );
		return r;
	}

	template<typename Signed, typename Unsigned>
	void test_exact_out_of_range( int &error_count ) {
		auto const before = error_count;
		// Out of range values report overflow and keep the two's complement bits
		daw_ensure( Signed{ -1 }.as_exact_unsigned( ) == Unsigned::max( ) );
		daw_ensure( error_count == before + 1 );
		daw_ensure( Signed::min( ).as_exact_unsigned( ) ==
		            Signed::min( ).as_unsigned( ) );
		daw_ensure( error_count == before + 2 );
		daw_ensure( Unsigned::max( ).as_exact_signed( ) == Signed{ -1 } );
		daw_ensure( error_count == before + 3 );
		auto const first_too_big = Signed::max( ).as_unsigned( ) + Unsigned{ 1U };
		daw_ensure( first_too_big.as_exact_signed( ) == Signed::min( ) );
		daw_ensure( error_count == before + 4 );
	}
} // namespace

static_assert( test_casts<daw::i8, daw::u8>( ) );
static_assert( test_casts<daw::i16, daw::u16>( ) );
static_assert( test_casts<daw::i32, daw::u32>( ) );
static_assert( test_casts<daw::i64, daw::u64>( ) );
static_assert( test_exact_in_range<daw::i8, daw::u8>( ) );
static_assert( test_exact_in_range<daw::i16, daw::u16>( ) );
static_assert( test_exact_in_range<daw::i32, daw::u32>( ) );
static_assert( test_exact_in_range<daw::i64, daw::u64>( ) );

int main( ) {
	auto error_count = 0;
	auto handler = [&]( daw::integers::IntegerErrorType ) { ++error_count; };
	daw::integers::register_integer_overflow_handler( handler );
	daw::integers::register_integer_div_by_zero_handler( handler );

	daw_ensure( test_casts<daw::i8, daw::u8>( ) );
	daw_ensure( test_casts<daw::i16, daw::u16>( ) );
	daw_ensure( test_casts<daw::i32, daw::u32>( ) );
	daw_ensure( test_casts<daw::i64, daw::u64>( ) );
	daw_ensure( error_count == 0 );

	test_exact_out_of_range<daw::i8, daw::u8>( error_count );
	test_exact_out_of_range<daw::i16, daw::u16>( error_count );
	test_exact_out_of_range<daw::i32, daw::u32>( error_count );
	test_exact_out_of_range<daw::i64, daw::u64>( error_count );
}
