// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#define DAW_DEFAULT_UNSIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>
#include <daw/integers/daw_unsigned.h>

#include <daw/daw_ensure.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

using namespace daw::integers::literals;

static_assert( std::is_trivially_copyable_v<daw::u8> );
static_assert( std::is_trivially_copyable_v<daw::u16> );
static_assert( std::is_trivially_copyable_v<daw::u32> );
static_assert( std::is_trivially_copyable_v<daw::u64> );
static_assert( std::is_standard_layout_v<daw::u64> );
static_assert( sizeof( daw::u8 ) == 1 );
static_assert( sizeof( daw::u16 ) == 2 );
static_assert( sizeof( daw::u32 ) == 4 );
static_assert( sizeof( daw::u64 ) == 8 );

static_assert(
  std::is_same_v<decltype( daw::integers::uint_impl::unsigned_integer(
                   std::uint16_t{ 1 } ) ),
                 daw::u16> );
static_assert( std::is_same_v<decltype( 1_u8 + 1_u32 ), daw::u32> );
static_assert( std::is_same_v<decltype( 1_u64 * 1_u16 ), daw::u64> );
static_assert(
  std::is_same_v<decltype( 1_u16 + std::uint8_t{ 1 } ), daw::u16> );
static_assert( std::is_same_v<decltype( 1_u16 + 1U ), daw::u32> );

// Mixing signed builtins into arithmetic is not allowed, use 1U
template<typename L, typename R>
concept can_add = requires( L l, R r ) { l + r; };
static_assert( can_add<daw::u32, unsigned> );
static_assert( not can_add<daw::u32, int> );
static_assert( not can_add<int, daw::u32> );

static_assert( std::numeric_limits<daw::u8>::digits == 8 );
static_assert( not std::numeric_limits<daw::u32>::is_signed );
static_assert( std::numeric_limits<daw::u16>::max( ) == 65535U );
static_assert( std::numeric_limits<daw::u16>::min( ) == 0U );
static_assert( std::is_same_v<daw::make_signed_t<daw::u32>, std::int32_t> );

namespace {
	int overflow_count = 0;
	int div_by_zero_count = 0;

	template<typename Integer>
	constexpr bool test_basic( ) {
		using value_t = typename Integer::value_type;
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1U };
		auto const two = Integer{ 2U };

		{
			auto u16_test = daw::u16{ };
			auto uint16_value = std::uint16_t{42};
			u16_test = uint16_value;
			daw_ensure( u16_test == 42 );
		}

		bool r = true;
		r &= Integer{ 3U } + Integer{ 4U } == 7U;
		r &= Integer{ 9U } - Integer{ 4U } == 5U;
		r &= Integer{ 3U } * Integer{ 4U } == 12U;
		r &= Integer{ 13U } / Integer{ 4U } == 3U;
		r &= Integer{ 13U } % Integer{ 4U } == 1U;
		r &= ( one << Integer{ 3U } ) == 8U;
		r &= ( Integer{ 16U } >> Integer{ 3U } ) == 2U;
		r &= ( Integer{ 0b1100U } | Integer{ 0b0011U } ) == 0b1111U;
		r &= ( Integer{ 0b1100U } & Integer{ 0b0110U } ) == 0b0100U;
		r &= ( Integer{ 0b1100U } ^ Integer{ 0b0110U } ) == 0b1010U;
		r &= ~Integer{ } == maximum;

		// Comparisons with builtin types use the safe int compare functions
		r &= Integer{ 1U } == 1;
		r &= Integer{ 1U } > -1;
		r &= -1 < Integer{ 0U };
		r &= maximum > Integer{ };

		auto x = Integer{ 5U };
		r &= x++ == 5U and x == 6U;
		r &= x-- == 6U and x == 5U;
		r &= ++x == 6U;
		r &= --x == 5U;

		// Wrapping/saturating/overflowing
		r &= maximum.add_wrapped( one ) == 0U;
		r &= maximum.add_saturated( one ) == maximum;
		r &= maximum.add_unchecked( two ) == 1U;
		r &= Integer{ }.sub_wrapped( one ) == maximum;
		r &= Integer{ }.sub_saturated( one ) == 0U;
		r &= Integer{ }.sub_unchecked( one ) == maximum;
		r &= maximum.mul_wrapped( two ) == maximum - one;
		r &= maximum.mul_saturated( two ) == maximum;
		r &= maximum.mul_unchecked( maximum ) == 1U;

		auto const add_ok = Integer{ 3U }.add_overflowing( Integer{ 4U } );
		auto const add_ov = maximum.add_overflowing( one );
		auto const sub_ok = Integer{ 4U }.sub_overflowing( Integer{ 3U } );
		auto const sub_ov = Integer{ 3U }.sub_overflowing( Integer{ 4U } );
		auto const mul_ok = Integer{ 3U }.mul_overflowing( Integer{ 4U } );
		auto const mul_ov = maximum.mul_overflowing( two );
		r &= add_ok.value == value_t{ 7 } and not add_ok.overflowed;
		r &= add_ov.value == value_t{ 0 } and add_ov.overflowed;
		r &= sub_ok.value == value_t{ 1 } and not sub_ok.overflowed;
		r &= sub_ov.value == maximum.value( ) and sub_ov.overflowed;
		r &= mul_ok.value == value_t{ 12 } and not mul_ok.overflowed;
		r &= mul_ov.value == value_t( maximum.value( ) - 1U ) and mul_ov.overflowed;

		auto const div_ok = Integer{ 7U }.div_overflowing( two );
		auto const div_z = Integer{ 7U }.div_overflowing( Integer{ } );
		auto const rem_ok = Integer{ 7U }.rem_overflowing( two );
		auto const rem_z = Integer{ 7U }.rem_overflowing( Integer{ } );
		using err_t = daw::integers::IntegerErrorType;
		r &= div_ok.value == value_t{ 3 } and div_ok.error == err_t::None;
		r &= div_z.error == err_t::DivideByZero;
		r &= rem_ok.value == value_t{ 1 } and rem_ok.error == err_t::None;
		r &= rem_z.error == err_t::DivideByZero;

		r &= Integer{ 7U }.div_euclid( two ) == 3U;
		r &= Integer{ 7U }.rem_euclid( two ) == 1U;
		r &= Integer{ 7U }.div_saturated( two ) == 3U;
		r &= Integer{ 7U }.rem_wrapped( two ) == 1U;

		r &= Integer{ }.negate_unchecked( ) == 0U;
		r &= one.negate_unchecked( ) == maximum;
		r &= maximum.negate_unchecked( ) == one;
		r &= -Integer{ } == 0U;
		r &= -one == maximum;
		r &= -maximum == one;
		r &= -Integer{ 5U } == value_t( -value_t{ 5 } );
		r &= -( -Integer{ 5U } ) == 5U;

		r &= Integer{ 3U }.pow( 3 ) == 27U;
		r &= two.pow_wrapped( sizeof( value_t ) * 8U ) == 0U;
		r &= two.pow_saturated( sizeof( value_t ) * 8U ) == maximum;
		r &= Integer{ 5U }.pow( 0 ) == 1U;

		r &= one.shl_overflowing( sizeof( value_t ) * 8U + 1U ) == 2U;
		r &=
		  maximum.shr_overflowing( Integer( sizeof( value_t ) * 8U ) ) == maximum;
		r &= one.shl_unchecked( Integer{ 2U } ) == 4U;
		r &= Integer{ 8U }.shr_checked( Integer{ 2U } ) == 2U;

		r &= one.rotate_right( 1 ) == Integer( maximum.value( ) / 2U + 1U );
		r &= one.rotate_left( sizeof( value_t ) * 8U ) == one;

		r &= Integer{ }.count_leading_zeros( ) == sizeof( value_t ) * 8U;
		r &= one.count_leading_zeros( ) == sizeof( value_t ) * 8U - 1U;
		r &= maximum.count_leading_zeros( ) == 0U;
		r &= Integer{ }.count_trailing_zeros( ) == sizeof( value_t ) * 8U;
		r &= Integer{ 8U }.count_trailing_zeros( ) == 3U;
		r &= one.reverse_bits( ) == Integer( maximum.value( ) / 2U + 1U );

		r &= static_cast<bool>( one ) and not static_cast<bool>( Integer{ } );
		r &= one.And( two ) and not one.And( Integer{ } );
		r &= one.Or( Integer{ } ) and not Integer{ }.Or( Integer{ } );
		return r;
	}

	template<typename Integer>
	void test_checked( ) {
		auto const maximum = Integer::max( );
		auto const one = Integer{ 1U };
		auto const zero = Integer{ };
		constexpr auto bits = sizeof( typename Integer::value_type ) * 8U;

		auto const check_overflow = [&]( auto &&f ) {
			auto const before = overflow_count;
			(void)f( );
			daw_ensure( overflow_count == before + 1 );
		};
		auto const check_div_by_zero = [&]( auto &&f ) {
			auto const before = div_by_zero_count;
			(void)f( );
			daw_ensure( div_by_zero_count == before + 1 );
		};

		check_overflow( [&] {
			return maximum.add_checked( one );
		} );
		check_overflow( [&] {
			return zero.sub_checked( one );
		} );
		check_overflow( [&] {
			return maximum.mul_checked( Integer{ 2U } );
		} );
		check_overflow( [&] {
			return one.shl_checked( Integer( bits ) );
		} );
		check_overflow( [&] {
			return one.shr_checked( Integer( bits ) );
		} );
		check_overflow( [&] {
			return one.shl_overflowing( -1 );
		} );
		check_overflow( [&] {
			return Integer{ 2U }.pow_checked( bits );
		} );
		check_overflow( [&] {
			return maximum + one;
		} );
		check_overflow( [&] {
			return zero - one;
		} );
		check_overflow( [&] {
			return maximum * Integer{ 2U };
		} );
		check_overflow( [&] {
			return one << Integer( bits );
		} );
		check_overflow( [&] {
			auto x = zero;
			return --x;
		} );
		check_overflow( [&] {
			auto x = maximum;
			return ++x;
		} );
		check_overflow( [] {
			return Integer( -1 );
		} );
		check_overflow( [] {
			return Integer::conversion_checked( -1 );
		} );

		check_div_by_zero( [&] {
			return one / zero;
		} );
		check_div_by_zero( [&] {
			return one % zero;
		} );
		check_div_by_zero( [&] {
			return one.div_checked( zero );
		} );
		check_div_by_zero( [&] {
			return one.rem_checked( zero );
		} );
		check_div_by_zero( [&] {
			return one.div_euclid_checked( zero );
		} );
		check_div_by_zero( [&] {
			return one.rem_euclid_checked( zero );
		} );

		auto const before = overflow_count;
		(void)maximum.add_wrapped( one );
		(void)maximum.add_saturated( one );
		(void)maximum.add_overflowing( one );
		(void)zero.sub_wrapped( one );
		(void)zero.sub_saturated( one );
		(void)maximum.mul_wrapped( maximum );
		(void)maximum.mul_saturated( maximum );
		(void)one.negate_unchecked( );
		(void)-one;
		(void)Integer{ 2U }.pow_wrapped( bits );
		(void)Integer{ 2U }.pow_saturated( bits );
		(void)one.shl_overflowing( bits );
		(void)Integer::conversion_unchecked( -1 );
		daw_ensure( overflow_count == before );
	}

	void test_conversions( ) {
		auto const before = overflow_count;
		// Widening is implicit and never checked
		daw::u64 const w = daw::u8{ 200U };
		daw_ensure( w == 200U );
		daw_ensure( static_cast<daw::u64>( 200_u8 ) == 200U );
		daw_ensure( daw::u8::conversion_checked( 255_u32 ) == 255U );
		daw_ensure( daw::u8::conversion_unchecked( 256_u32 ) == 0U );
		daw_ensure( static_cast<std::uint16_t>( 300_u16 ) == 300U );
		daw_ensure( overflow_count == before );

		// Narrowing is explicit and checked
		daw_ensure( daw::u8( 256_u32 ) == 0U );
		daw_ensure( overflow_count == before + 1 );
		daw_ensure( daw::u8::conversion_checked( 256_u32 ) == 0U );
		daw_ensure( overflow_count == before + 2 );
		daw_ensure( daw::u16( 70000 ) == 70000U - 65536U );
		daw_ensure( overflow_count == before + 3 );
	}

	void test_bytes( ) {
		unsigned char const bytes[] = {
		  0x01, 0x02, 0x03, 0x84, 0x05, 0x06, 0x07, 0x88 };
		daw_ensure( daw::u8::from_bytes_le( bytes ) == 0x01U );
		daw_ensure( daw::u16::from_bytes_le( bytes ) == 0x0201U );
		daw_ensure( daw::u16::from_bytes_be( bytes ) == 0x0102U );
		daw_ensure( daw::u32::from_bytes_le( bytes ) == 0x8403'0201U );
		daw_ensure( daw::u32::from_bytes_be( bytes ) == 0x0102'0384U );
		daw_ensure( daw::u64::from_bytes_le( bytes ) == 0x8807'0605'8403'0201ULL );
		daw_ensure( daw::u64::from_bytes_be( bytes ) == 0x0102'0384'0506'0788ULL );
	}

	// signed_integer and unsigned_integer report through the same handlers
	void test_shared_handlers( ) {
		auto const overflows = overflow_count;
		auto const div_by_zeros = div_by_zero_count;
		(void)daw::i32::max( ).add_checked( daw::i32{ 1 } );
		(void)daw::u32::max( ).add_checked( daw::u32{ 1U } );
		(void)daw::i32{ 1 }.div_checked( daw::i32{ } );
		(void)daw::u32{ 1U }.div_checked( daw::u32{ } );
		daw_ensure( overflow_count == overflows + 2 );
		daw_ensure( div_by_zero_count == div_by_zeros + 2 );
	}

	void test_compound_builtin( ) {
		auto x = 10_u32;
		x += 5U;
		x -= std::uint8_t{ 3 };
		x *= 2U;
		x /= 3U;
		x %= 5U;
		x <<= 4U;
		x >>= 2U;
		x |= 1U;
		x &= 0xFU;
		x ^= 2U;
		daw_ensure( x == 15U );
	}
} // namespace

static_assert( test_basic<daw::u8>( ) );
static_assert( test_basic<daw::u16>( ) );
static_assert( test_basic<daw::u32>( ) );
static_assert( test_basic<daw::u64>( ) );

int main( ) {
	auto overflow_handler = []( daw::integers::IntegerErrorType e ) {
		daw_ensure( e == daw::integers::IntegerErrorType::Overflow );
		++overflow_count;
	};
	auto div_by_zero_handler = []( daw::integers::IntegerErrorType e ) {
		daw_ensure( e == daw::integers::IntegerErrorType::DivideByZero );
		++div_by_zero_count;
	};
	daw::integers::register_integer_overflow_handler( overflow_handler );
	daw::integers::register_integer_div_by_zero_handler( div_by_zero_handler );

	daw_ensure( test_basic<daw::u8>( ) );
	daw_ensure( test_basic<daw::u16>( ) );
	daw_ensure( test_basic<daw::u32>( ) );
	daw_ensure( test_basic<daw::u64>( ) );
	daw_ensure( overflow_count == 0 and div_by_zero_count == 0 );

	test_checked<daw::u8>( );
	test_checked<daw::u16>( );
	test_checked<daw::u32>( );
	test_checked<daw::u64>( );
	test_conversions( );
	test_bytes( );
	test_compound_builtin( );
	test_shared_handlers( );
}
