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

#include <climits>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <type_traits>

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

DAW_ATTRIB_NOINLINE daw::i32 test_div( daw::i32 first, daw::i32 inc,
                                       daw::i32 expected ) {
	first /= inc;
	daw_ensure( first == expected );
	return first;
}

DAW_ATTRIB_NOINLINE daw::i64 test_div( daw::i64 first, daw::i64 inc,
                                       daw::i64 expected ) {
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

static_assert( std::is_same_v<decltype( 10_i8 - 3_i16 ), daw::i16> );
static_assert( std::is_same_v<decltype( 10_i64 - 3_i8 ), daw::i64> );
static_assert( 10_i8 - 3_i16 == 7_i16 );
static_assert( 10_i64 - 3_i8 == 7_i64 );
static_assert( 10_i8 + 3_i16 == 13_i16 );
static_assert( 10_i8 * 3_i16 == 30_i16 );
static_assert( 10_i16 / 3_i8 == 3_i16 );

template<typename Integer>
void test_arithmetic_regressions( bool &has_overflow ) {
	static_assert(
	  std::is_same_v<decltype( Integer( 10 ) - Integer( 3 ) ), Integer> );
	daw_ensure( Integer( 10 ) - Integer( 3 ) == Integer( 7 ) );
	daw_ensure( Integer( -10 ) - Integer( -3 ) == Integer( -7 ) );

	daw_ensure( Integer( 7 ) % Integer( 3 ) == Integer( 1 ) );
	daw_ensure( Integer( -7 ) % Integer( 3 ) == Integer( -1 ) );
	auto remainder = Integer( 7 );
	remainder %= Integer( 3 );
	daw_ensure( remainder == Integer( 1 ) );
	daw_ensure( Integer( 7 ).rem_checked( Integer( 3 ) ) == Integer( 1 ) );
	daw_ensure( Integer( 7 ).rem_saturated( Integer( 3 ) ) == Integer( 1 ) );
	daw_ensure( Integer::min( ).rem_saturated( Integer( -1 ) ) == Integer( 0 ) );

	daw_ensure( Integer::max( ).add_wrapped( Integer( 1 ) ) == Integer::min( ) );
	daw_ensure( Integer::min( ).sub_wrapped( Integer( 1 ) ) == Integer::max( ) );
	daw_ensure( Integer::max( ).add_saturated( Integer( 1 ) ) ==
	            Integer::max( ) );
	daw_ensure( Integer::min( ).sub_saturated( Integer( 1 ) ) ==
	            Integer::min( ) );
	daw_ensure( Integer::min( ).mul_saturated( Integer( -1 ) ) ==
	            Integer::max( ) );
	daw_ensure( Integer::min( ).div_saturated( Integer( -1 ) ) ==
	            Integer::max( ) );

	has_overflow = false;
	auto const wrapped_div = Integer::min( ).div_wrapped( Integer( -1 ) );
	daw_ensure( wrapped_div == Integer::min( ) );
	daw_ensure( not has_overflow );
}

template<typename Integer>
void test_bit_regressions( bool &has_overflow ) {
	auto bits = Integer( 0x0F );
	bits &= Integer( 0x03 );
	daw_ensure( bits == Integer( 0x03 ) );
	bits ^= Integer( 0x01 );
	daw_ensure( bits == Integer( 0x02 ) );
	bits |= Integer( 0x08 );
	daw_ensure( bits == Integer( 0x0A ) );

	has_overflow = false;
	daw_ensure( Integer( 1 ).shl_checked( Integer( 0 ) ) == Integer( 1 ) );
	daw_ensure( Integer( 1 ).shr_checked( Integer( 0 ) ) == Integer( 1 ) );
	daw_ensure( not has_overflow );

	constexpr auto bit_count = sizeof( typename Integer::value_type ) * CHAR_BIT;
	has_overflow = false;
	(void)Integer( 1 ).shl_checked( Integer( bit_count ) );
	daw_ensure( has_overflow );
	has_overflow = false;
	(void)Integer( 1 ).shr_checked( Integer( bit_count ) );
	daw_ensure( has_overflow );
	has_overflow = false;
	(void)Integer( 1 ).shl_checked( Integer( -1 ) );
	daw_ensure( has_overflow );
	has_overflow = false;
	(void)Integer( 1 ).shr_checked( Integer( -1 ) );
	daw_ensure( has_overflow );

	auto const value = Integer( 0x53 );
	daw_ensure( value.rotate_left( 0 ) == value );
	daw_ensure( value.rotate_right( 0 ) == value );
	daw_ensure( value.rotate_left( bit_count ) == value );
	daw_ensure( value.rotate_right( bit_count ) == value );
	daw_ensure( value.rotate_left( bit_count + 1 ) == value.rotate_left( 1 ) );
	daw_ensure( value.rotate_right( bit_count + 1 ) == value.rotate_right( 1 ) );

	daw_ensure( Integer( 0 ).count_leading_zeros( ) == bit_count );
	daw_ensure( Integer( 0 ).count_trailing_zeros( ) == bit_count );
	daw_ensure( Integer( 1 ).count_trailing_zeros( ) == 0 );
}

template<typename Integer>
void test_pow_regressions( bool &has_overflow ) {
	daw_ensure( Integer( 7 ).pow_checked( 0 ) == Integer( 1 ) );
	daw_ensure( Integer( 3 ).pow_checked( 2 ) == Integer( 9 ) );
	daw_ensure( Integer( -2 ).pow_checked( 3 ) == Integer( -8 ) );

	has_overflow = false;
	daw_ensure( Integer::max( ).pow_checked( 1 ) == Integer::max( ) );
	daw_ensure( not has_overflow );
}

template<typename Integer>
void test_byte_decoding( ) {
	constexpr auto byte_count = sizeof( typename Integer::value_type );
	unsigned char little_endian[byte_count]{ };
	unsigned char big_endian[byte_count]{ };
	little_endian[byte_count - 1] = 0x80U;
	big_endian[0] = 0x80U;
	daw_ensure( Integer::from_bytes_le( little_endian ) == Integer::min( ) );
	daw_ensure( Integer::from_bytes_be( big_endian ) == Integer::min( ) );

	for( std::size_t n = 0; n < byte_count; ++n ) {
		little_endian[n] = 0xFFU;
		big_endian[n] = 0xFFU;
	}
	daw_ensure( Integer::from_bytes_le( little_endian ) == Integer( -1 ) );
	daw_ensure( Integer::from_bytes_be( big_endian ) == Integer( -1 ) );
}

int main( ) try {
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

	test_arithmetic_regressions<daw::i8>( has_overflow );
	test_arithmetic_regressions<daw::i16>( has_overflow );
	test_arithmetic_regressions<daw::i32>( has_overflow );
	test_arithmetic_regressions<daw::i64>( has_overflow );
	test_bit_regressions<daw::i8>( has_overflow );
	test_bit_regressions<daw::i16>( has_overflow );
	test_bit_regressions<daw::i32>( has_overflow );
	test_bit_regressions<daw::i64>( has_overflow );
	test_pow_regressions<daw::i8>( has_overflow );
	test_pow_regressions<daw::i16>( has_overflow );
	test_pow_regressions<daw::i32>( has_overflow );
	test_pow_regressions<daw::i64>( has_overflow );
	test_byte_decoding<daw::i8>( );
	test_byte_decoding<daw::i16>( );
	test_byte_decoding<daw::i32>( );
	test_byte_decoding<daw::i64>( );

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
	try {
		(void)( 10_i32 ).rem_checked( 0_i32 );
	} catch( daw::integers::signed_integer_div_by_zero_exception const & ) {
		has_exception = true;
	}
	daw_ensure( has_exception );
	has_exception = false;
	daw::integers::register_signed_overflow_handler( );
	try {
		(void)daw::i32::max( ).add_checked( 1_i32 );
	} catch( daw::integers::signed_integer_overflow_exception const & ) {
		has_exception = true;
	}
	daw_ensure( has_exception );

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
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::max( );
		auto i1 = daw::i16::max( );
		i0 += i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::max( );
		auto i1 = daw::i32::max( );
		i0 += i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		auto i1 = daw::i64::max( );
		i0 += i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i8::min( );
		auto i1 = daw::i8::max( );
		daw::do_not_optimize( i0 );
		daw::do_not_optimize( i1 );
		i0 -= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::min( );
		auto i1 = daw::i16::max( );
		i0 -= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::min( );
		auto i1 = daw::i32::max( );
		i0 -= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::min( );
		auto i1 = daw::i16::max( );
		i0 -= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i8::max( );
		auto i1 = daw::i8::max( );
		i0 *= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i16::max( );
		auto i1 = daw::i16::max( );
		i0 *= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i32::max( );
		auto i1 = daw::i32::max( );
		i0 *= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::max( );
		auto i1 = daw::i64::max( );
		i0 *= i1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		i0 *= -1;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		--i0;
		(void)i0;
		daw_ensure( has_overflow );
	}
	{
		has_overflow = false;
		auto i0 = daw::i64::min( );
		(void)i0--;
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
		(void)i0++;
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
		(void)has_exception;
		auto i0 = 5_i32;
		auto i1 = static_cast<daw::i8>( i0 );
		(void)i1;
		daw_ensure( not has_overflow );
		i0 = daw::i32::max( );
		auto i2 = static_cast<daw::i8>( i0 );
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
		(void)i0;
		daw_ensure( not has_overflow );
		i0 = daw::i8::conversion_checked( 255 );
		(void)i0;
		daw_ensure( has_overflow );
		has_overflow = false;
		auto i1 = daw::i32::conversion_checked( 5000 );
		daw::i8 i2 = daw::i8::conversion_checked( i1 );
		(void)i2;
		daw_ensure( has_overflow );
		has_overflow = false;
		i1 = daw::i8( 55 );
		i2 = daw::i8::conversion_checked( i1 );
		(void)i2;
		daw_ensure( not has_overflow );
	}
	{
		constexpr std::uint32_t le_val = 0x0123'4567U;
		constexpr std::uint32_t be_val = 0x6745'2301U;
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
	static_assert( ( -1_i32 ).shl_checked( 1_i32 ) == -2_i32 );

	static_assert( ( -8_i32 ).shr_checked( 1_i32 ) == -4_i32 );

	static_assert( ( -1_i8 ).shl_checked( 7_i8 ) == daw::i8::min( ) );

	static_assert( daw::i8::min( ).shr_checked( 7_i8 ) == -1_i8 );
} catch( ... ) {
	std::cerr << "Unexpected exception thrown\n" << std::flush;
	throw;
}
