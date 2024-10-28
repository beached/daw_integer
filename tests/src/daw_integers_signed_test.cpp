// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_benchmark.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_ensure.h>

#include <initializer_list>

using namespace daw::integers::literals;

#if defined( __cpp_nontype_template_args )
#if __cpp_nontype_template_args >= 201911L
template<daw::i8 num>
struct foo {
	static constexpr daw::i8 value = num;
};
static_assert( foo<40_i8>::value == 40 );
#endif
#endif
static_assert( std::is_trivially_copyable_v<daw::i8> );
static_assert( std::is_trivially_copyable_v<daw::i16> );
static_assert( std::is_trivially_copyable_v<daw::i32> );
static_assert( std::is_trivially_copyable_v<daw::i64> );
static_assert( std::is_trivially_destructible_v<daw::i8> );
static_assert( std::is_trivially_destructible_v<daw::i16> );
static_assert( std::is_trivially_destructible_v<daw::i32> );
static_assert( std::is_trivially_destructible_v<daw::i64> );
static_assert( sizeof( daw::i8 ) == 1 );
static_assert( sizeof( daw::i16 ) == 2 );
static_assert( sizeof( daw::i32 ) == 4 );
static_assert( sizeof( daw::i64 ) == 8 );
static_assert( std::is_standard_layout_v<daw::i8> );
static_assert( std::is_standard_layout_v<daw::i16> );
static_assert( std::is_standard_layout_v<daw::i32> );
static_assert( std::is_standard_layout_v<daw::i64> );

DAW_ATTRIB_NOINLINE int test_plus( std::initializer_list<daw::i32> const &vals,
                                   daw::i32 expected ) {
	auto sum = 0_i32;
	for( auto v : vals ) {
		sum += v;
	}
	daw_ensure( sum == expected );
	return sum.value( );
}

DAW_ATTRIB_NOINLINE daw::i32
test_div( daw::i32 first, daw::i32 inc, daw::i32 expected ) {
	first /= inc;
	daw_ensure( first == expected );
	return first;
}

DAW_ATTRIB_NOINLINE daw::i64
test_div( daw::i64 first, daw::i64 inc, daw::i64 expected ) {
	first /= inc;
	daw_ensure( first == expected );
	return first;
}

template<auto L, auto R>
struct check_eql_t;

template<auto V>
struct check_eql_t<V, V> : std::true_type {};

template<auto L, auto R>
inline constexpr auto check_equal = check_eql_t<L, R>::value;

int main( ) {
	bool has_overflow = false;
	bool has_div_by_zero = false;
	auto const error_handler =
	  [&]( daw::integers::SignedIntegerErrorType error_type ) {
		  switch( error_type ) {
		  case daw::integers::SignedIntegerErrorType::DivideByZero:
			  has_div_by_zero = true;
			  break;
		  case daw::integers::SignedIntegerErrorType::Overflow:
			  has_overflow = true;
			  break;
		  default:
			  break;
		  }
	  };
	daw::integers::register_signed_overflow_handler( error_handler );
	daw::integers::register_signed_div_by_zero_handler( error_handler );

	test_plus( { 55_i32, 55_i32, 55_i32 }, 165_i32 );
	test_div( 110_i32, 2_i32, 55_i32 );
	test_div( 110_i64, 2_i64, 55_i64 );

	auto y = 10_i8;
	y *= 100_i8;
	daw_ensure( has_overflow );
	has_overflow = false;

	y /= 0_i8;
	daw_ensure( has_div_by_zero );
	has_div_by_zero = false;
	daw::integers::register_signed_div_by_zero_handler( );
	bool has_exception = false;
	try {
		y /= 0_i8;
	} catch( daw::integers::signed_integer_div_by_zero_exception const & ) {
		has_exception = true;
	}
	daw_ensure( has_exception );
	has_exception = false;
	(void)has_exception;

	auto x = y * 2;
	auto xb = x or y;
	daw_ensure( xb );
	auto z = x * y;
	(void)z;
	daw::integers::register_signed_overflow_handler( error_handler );
	daw::integers::register_signed_div_by_zero_handler( error_handler );
	{
		has_overflow = false;
		auto i0 = daw::i8::max( );
		auto i1 = daw::i8::max( );
		i0 += i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::max( );
		auto i1 = daw::i16::max( );
		i0 += i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::max( );
		auto i1 = daw::i32::max( );
		i0 += i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		auto i1 = daw::i64::max( );
		i0 += i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i8::min( );
		auto i1 = daw::i8::max( );
		daw::do_not_optimize( i0 );
		daw::do_not_optimize( i1 );
		i0 -= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::min( );
		auto i1 = daw::i16::max( );
		i0 -= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::min( );
		auto i1 = daw::i32::max( );
		i0 -= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::min( );
		auto i1 = daw::i16::max( );
		i0 -= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i8::max( );
		auto i1 = daw::i8::max( );
		i0 *= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::max( );
		auto i1 = daw::i16::max( );
		i0 *= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::max( );
		auto i1 = daw::i32::max( );
		i0 *= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		auto i1 = daw::i64::max( );
		i0 *= i1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		i0 *= -1;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		--i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		i0--;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		++i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		i0++;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		--i0;
		daw_ensure( has_overflow );
		daw_ensure( i0 == daw::i64::max( ) );
	}
	{
		has_div_by_zero = false;
		auto i0 = daw::i64::min( );
		i0 /= 0;
		daw_ensure( has_div_by_zero );
	}
	{
		has_overflow = false;
		has_div_by_zero = false;
		has_exception = false;
		auto i0 = 5_i32;
		daw::i8 i1 = static_cast<daw::i8>( i0 );
		(void)i1;
		daw_ensure( not has_overflow );
		i0 = daw::i32::max( );
		daw::i8 i2 = static_cast<daw::i8>( i0 );
		static_assert( not std::is_convertible_v<daw::i32, daw::i8> );
		static_assert( std::is_convertible_v<daw::i8, daw::i32> );
		static_assert( std::is_constructible_v<daw::i8, daw::i32> );
		daw_ensure( has_overflow );
		has_overflow = false;
		daw::i64 i3 = i2;
		(void)i3;
		daw_ensure( not has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i8::conversion_checked( 12 );
		daw_ensure( not has_overflow );
		i0 = daw::i8::conversion_checked( 255 );
		daw_ensure( has_overflow );
		has_overflow = false;
		auto i1 = daw::i32::conversion_checked( 5000 );
		daw::i8 i2 = daw::i8::conversion_checked( i1 );
		daw_ensure( has_overflow );
		has_overflow = false;
		i1 = daw::i8( 55 );
		i2 = daw::i8::conversion_checked( i1 );
		daw_ensure( not has_overflow );
	}
	{
		std::uint32_t const le_val = 0x0123'4567U;
		std::uint32_t const be_val = 0x6745'2301U;
		auto const le_bytes = reinterpret_cast<unsigned char const *>( &le_val );
		auto const le_le = daw::i32::from_bytes_le( le_bytes );
		daw_ensure( static_cast<std::uint32_t>( le_le ) == le_val );
		auto const le_be = daw::i32::from_bytes_be( le_bytes );
		daw_ensure( static_cast<std::uint32_t>( le_be ) == be_val );
	}
	{
		auto i0 = 0x10000b3_i32;
		auto i1 = 0xb301_i32;
		auto rot0 = i0.rotate_left( 8 );
		daw_ensure( rot0 == i1 );
	}
	{
		auto i0 = 0xb301_i32;
		auto i1 = 0x10000b3_i32;
		auto rot0 = i0.rotate_right( 8 );
		daw_ensure( rot0 == i1 );
		daw_ensure( i0 and i1 );
		daw_ensure( i0 or i1 );
		auto zero = 0_i32;
		daw_ensure( not( i0 and zero ) );
		daw_ensure( i0 or zero );
	}
	{
		constexpr auto i0 = -1_i8;
		using u8_t = unsigned char;
		constexpr auto u1 = u8_t{ 255 };
		static_assert( i0 < u1 );
		static_assert( i0 <= u1 );
		static_assert( not( u1 < i0 ) );
		static_assert( not( u1 <= i0 ) );
		static_assert( u1 > i0 );
		static_assert( u1 >= i0 );
		static_assert( not( i0 > u1 ) );
		static_assert( not( i0 >= u1 ) );
		static_assert( i0 != u1 );
		static_assert( u1 != i0 );
		static_assert( not( i0 == u1 ) );
		static_assert( not( u1 == i0 ) );
		constexpr unsigned u2 = 55555U;
		static_assert( i0 < u2 );
		static_assert( i0 <= u2 );
		static_assert( not( u2 < i0 ) );
		static_assert( not( u2 <= i0 ) );
		static_assert( u2 > i0 );
		static_assert( u2 >= i0 );
		static_assert( not( i0 > u2 ) );
		static_assert( not( i0 >= u2 ) );
		static_assert( i0 != u2 );
		static_assert( u2 != i0 );
		static_assert( not( i0 == u2 ) );
		static_assert( not( u2 == i0 ) );
	}
	static_assert( daw::i32::max( ).add_saturated( 300_i32 ) ==
	               daw::i32::max( ) );
	static_assert( daw::i32( 300 ).add_saturated( daw::i32::max( ) ) ==
	               daw::i32::max( ) );
	static_assert( daw::i32::min( ).add_saturated( -300_i32 ) ==
	               daw::i32::min( ) );
	static_assert( daw::i32( -300 ).add_saturated( daw::i32::min( ) ) ==
	               daw::i32::min( ) );

	static_assert( daw::i32::min( ).sub_saturated( 300_i32 ) ==
	               daw::i32::min( ) );
	static_assert( daw::i32( 300 ).sub_saturated( daw::i32::min( ) ) ==
	               daw::i32::max( ) );
	static_assert( daw::i32::max( ).sub_saturated( -300_i32 ) ==
	               daw::i32::max( ) );
	static_assert( daw::i32( -300 ).sub_saturated( daw::i32::max( ) ) ==
	               daw::i32::min( ) );

	static_assert( daw::i32::min( ).mul_saturated( -1_i32 ) == daw::i32::max( ) );
	static_assert( daw::i32::max( ).mul_saturated( 2_i32 ) == daw::i32::max( ) );
	static_assert( daw::i32::max( ).mul_saturated( -2_i32 ) == daw::i32::min( ) );

	static_assert( daw::i32::min( ).div_saturated( -1_i32 ) == daw::i32::max( ) );

	static_assert( daw::i8::conversion_unchecked( 0xAAU ).reverse_bits( ) ==
	               daw::i8::conversion_unchecked( 0x55U ) );
	static_assert( daw::i8::conversion_unchecked( 0x80U ).reverse_bits( ) ==
	               daw::i8::conversion_unchecked( 1U ) );

	static_assert( daw::i16::conversion_unchecked( 0xAAAAU ).reverse_bits( ) ==
	               daw::i16::conversion_unchecked( 0x5555U ) );
	static_assert( daw::i16::conversion_unchecked( 0x8000U ).reverse_bits( ) ==
	               daw::i16::conversion_unchecked( 1U ) );

	static_assert(
	  daw::i32::conversion_unchecked( 0xAAAA'AAAAUL ).reverse_bits( ) ==
	  daw::i32::conversion_unchecked( 0x5555'5555UL ) );
	static_assert(
	  daw::i32::conversion_unchecked( 0x8000'0000UL ).reverse_bits( ) ==
	  daw::i32::conversion_unchecked( 1UL ) );

	static_assert( daw::i64::conversion_unchecked( 0xAAAA'AAAA'AAAA'AAAAULL )
	                 .reverse_bits( ) ==
	               daw::i64::conversion_unchecked( 0x5555'5555'5555'5555ULL ) );
	static_assert( daw::i64::conversion_unchecked( 0x8000'0000'0000'0000ULL )
	                 .reverse_bits( ) == daw::i64::conversion_unchecked( 1ULL ) );
}