// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

// Tests count_ones, ilog2, unsigned_abs, abs_diff and the std::optional
// returning try_ operations.  8 bit types are tested exhaustively against a
// reference computed with wider builtin integers.

#include <daw/daw_integer.h>

#include <daw/daw_ensure.h>

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

static_assert(
  std::is_same_v<decltype( daw::i32{ }.unsigned_abs( ) ), daw::u32> );
static_assert(
  std::is_same_v<decltype( daw::i64{ }.abs_diff( daw::i64{ } ) ), daw::u64> );
static_assert(
  std::is_same_v<decltype( daw::u16{ }.abs_diff( daw::u16{ } ) ), daw::u16> );
static_assert( std::is_same_v<decltype( daw::i8{ }.try_add( daw::i8{ } ) ),
                              std::optional<daw::i8>> );
static_assert( std::is_same_v<decltype( daw::u8{ }.try_as_signed( ) ),
                              std::optional<daw::i8>> );

// Usable in constant expressions
static_assert( daw::i32{ 7 }.count_ones( ) == 3 );
static_assert( daw::u32{ 1024U }.ilog2( ) == 10 );
static_assert( daw::i8::min( ).unsigned_abs( ) == 128U );
static_assert( not daw::i32::max( ).try_add( daw::i32{ 1 } ) );
static_assert( *daw::u32{ 3U }.try_pow( 4 ) == 81U );
static_assert( not daw::i32::try_from( 1LL << 40 ) );

namespace {
	int handler_count = 0;

	template<typename Integer>
	std::optional<Integer> ref( long long v ) {
		using value_t = typename Integer::value_type;
		if( v < std::numeric_limits<value_t>::min( ) or
		    v > std::numeric_limits<value_t>::max( ) ) {
			return std::nullopt;
		}
		return Integer( static_cast<value_t>( v ) );
	}

	template<typename Integer>
	void ensure_eq( std::optional<Integer> const &actual,
	                std::optional<Integer> const &expected ) {
		daw_ensure( actual.has_value( ) == expected.has_value( ) );
		if( actual ) {
			daw_ensure( *actual == *expected );
		}
	}

	std::uint32_t ref_ilog2( unsigned long long v ) {
		auto r = 0U;
		while( v > 1U ) {
			v /= 2U;
			++r;
		}
		return r;
	}

	long long ref_div_euclid( long long a, long long b ) {
		auto q = a / b;
		if( a % b < 0 ) {
			q = b > 0 ? q - 1 : q + 1;
		}
		return q;
	}

	long long ref_rem_euclid( long long a, long long b ) {
		auto r = a % b;
		if( r < 0 ) {
			r += b > 0 ? b : -b;
		}
		return r;
	}

	void test_i8_exhaustive( ) {
		using daw::i8;
		for( int a = -128; a <= 127; ++a ) {
			auto const x = i8( a );
			auto const ua = static_cast<std::uint8_t>( a );
			daw_ensure( x.count_ones( ) ==
			            static_cast<std::uint32_t>( std::popcount( ua ) ) );
			daw_ensure( x.unsigned_abs( ) == ( a < 0 ? -a : a ) );
			ensure_eq( x.try_negate( ), ref<i8>( -a ) );
			ensure_eq( x.try_abs( ), ref<i8>( a < 0 ? -a : a ) );
			daw_ensure( x.try_as_unsigned( ).has_value( ) == ( a >= 0 ) );
			if( a > 0 ) {
				daw_ensure( x.ilog2( ) == ref_ilog2( static_cast<unsigned>( a ) ) );
				daw_ensure( *x.try_ilog2( ) == x.ilog2( ) );
			} else {
				daw_ensure( not x.try_ilog2( ) );
			}
			for( unsigned e = 0; e <= 8; ++e ) {
				long long p = 1;
				for( unsigned i = 0; i < e; ++i ) {
					p *= a;
				}
				ensure_eq( x.try_pow( e ), ref<i8>( p ) );
			}
			for( int b = -128; b <= 127; ++b ) {
				auto const y = i8( b );
				ensure_eq( x.try_add( y ), ref<i8>( a + b ) );
				ensure_eq( x.try_sub( y ), ref<i8>( a - b ) );
				ensure_eq( x.try_mul( y ), ref<i8>( a * b ) );
				daw_ensure( x.abs_diff( y ) == ( a < b ? b - a : a - b ) );
				if( b == 0 ) {
					daw_ensure( not x.try_div( y ) and not x.try_rem( y ) );
					daw_ensure( not x.try_div_euclid( y ) and
					            not x.try_rem_euclid( y ) );
				} else {
					ensure_eq( x.try_div( y ), ref<i8>( a / b ) );
					// Like Rust, min % -1 is an overflow even though 0 fits
					ensure_eq( x.try_rem( y ), a == -128 and b == -1
					                             ? std::nullopt
					                             : ref<i8>( a % b ) );
					ensure_eq( x.try_div_euclid( y ), ref<i8>( ref_div_euclid( a, b ) ) );
					ensure_eq( x.try_rem_euclid( y ),
					           a == -128 and b == -1
					             ? std::nullopt
					             : ref<i8>( ref_rem_euclid( a, b ) ) );
				}
				if( b >= 0 and b < 8 ) {
					ensure_eq( x.try_shl( y ),
					           std::optional<i8>( i8( static_cast<std::int8_t>(
					             static_cast<std::uint8_t>( ua << b ) ) ) ) );
					ensure_eq( x.try_shr( y ), std::optional<i8>( i8( a >> b ) ) );
				} else {
					daw_ensure( not x.try_shl( y ) and not x.try_shr( y ) );
				}
			}
		}
	}

	void test_u8_exhaustive( ) {
		using daw::u8;
		for( unsigned a = 0; a <= 255; ++a ) {
			auto const x = u8( a );
			daw_ensure( x.count_ones( ) ==
			            static_cast<std::uint32_t>( std::popcount( a ) ) );
			daw_ensure( x.try_as_signed( ).has_value( ) == ( a <= 127 ) );
			if( a > 0 ) {
				daw_ensure( x.ilog2( ) == ref_ilog2( a ) );
				daw_ensure( *x.try_ilog2( ) == x.ilog2( ) );
			} else {
				daw_ensure( not x.try_ilog2( ) );
			}
			for( unsigned e = 0; e <= 8; ++e ) {
				unsigned long long p = 1;
				for( unsigned i = 0; i < e; ++i ) {
					p *= a;
				}
				ensure_eq( x.try_pow( e ),
				           p > 255U ? std::nullopt
				                    : ref<u8>( static_cast<long long>( p ) ) );
			}
			for( unsigned b = 0; b <= 255; ++b ) {
				auto const y = u8( b );
				auto const sa = static_cast<long long>( a );
				auto const sb = static_cast<long long>( b );
				ensure_eq( x.try_add( y ), ref<u8>( sa + sb ) );
				ensure_eq( x.try_sub( y ), ref<u8>( sa - sb ) );
				ensure_eq( x.try_mul( y ), ref<u8>( sa * sb ) );
				daw_ensure( x.abs_diff( y ) == ( a < b ? b - a : a - b ) );
				if( b == 0 ) {
					daw_ensure( not x.try_div( y ) and not x.try_rem( y ) );
					daw_ensure( not x.try_div_euclid( y ) and
					            not x.try_rem_euclid( y ) );
				} else {
					ensure_eq( x.try_div( y ), ref<u8>( sa / sb ) );
					ensure_eq( x.try_rem( y ), ref<u8>( sa % sb ) );
					ensure_eq( x.try_div_euclid( y ), ref<u8>( sa / sb ) );
					ensure_eq( x.try_rem_euclid( y ), ref<u8>( sa % sb ) );
				}
				if( b < 8 ) {
					ensure_eq( x.try_shl( y ), ref<u8>( ( a << b ) & 0xFFU ) );
					ensure_eq( x.try_shr( y ), ref<u8>( a >> b ) );
				} else {
					daw_ensure( not x.try_shl( y ) and not x.try_shr( y ) );
				}
			}
		}
	}

	void test_64_bit( ) {
		using daw::i64;
		using daw::u64;
		daw_ensure( i64::min( ).unsigned_abs( ) == 0x8000'0000'0000'0000ULL );
		daw_ensure( i64::min( ).abs_diff( i64::max( ) ) == u64::max( ) );
		daw_ensure( i64{ -1 }.count_ones( ) == 64 );
		daw_ensure( u64::max( ).count_ones( ) == 64 );
		daw_ensure( u64::max( ).ilog2( ) == 63 );
		daw_ensure( i64::max( ).ilog2( ) == 62 );
		daw_ensure( u64{ 0U }.abs_diff( u64::max( ) ) == u64::max( ) );
		daw_ensure( not i64::max( ).try_mul( i64{ 2 } ) );
		daw_ensure( *i64{ 3 }.try_pow( 39 ) == 4052555153018976267LL );
		daw_ensure( not i64{ 3 }.try_pow( 40 ) );
		daw_ensure( *i64{ -2 }.try_pow( 63 ) == i64::min( ) );
		daw_ensure( *u64{ 3U }.try_pow( 40 ) == 12157665459056928801ULL );
		daw_ensure( not u64{ 3U }.try_pow( 41 ) );
		daw_ensure( not i64::min( ).try_div( i64{ -1 } ) );
		daw_ensure( not i64::min( ).try_negate( ) );
		daw_ensure( *i64{ 1 }.try_shl( i64{ 63 } ) == i64::min( ) );
		daw_ensure( not i64{ 1 }.try_shl( i64{ 64 } ) );
		daw_ensure( not u64{ 1U }.try_shr( u64{ 64U } ) );
		daw_ensure( *daw::i32::try_from( -5 ) == -5 );
		daw_ensure( not daw::i32::try_from( 3'000'000'000U ) );
		daw_ensure( *daw::u32::try_from( 3'000'000'000U ) == 3'000'000'000U );
		daw_ensure( not daw::u32::try_from( -1 ) );
		daw_ensure( not daw::u8::try_from( 256 ) );
	}

	void test_ilog2_reports( ) {
		auto const before = handler_count;
		daw_ensure( daw::i32{ 0 }.ilog2( ) == 0 );
		daw_ensure( daw::i32{ -4 }.ilog2( ) == 0 );
		daw_ensure( daw::u32{ 0U }.ilog2( ) == 0 );
		daw_ensure( handler_count == before + 3 );
	}
} // namespace

int main( ) {
	auto handler = []( daw::integers::IntegerErrorType ) { ++handler_count; };
	daw::integers::register_integer_overflow_handler( handler );
	daw::integers::register_integer_div_by_zero_handler( handler );

	test_i8_exhaustive( );
	test_u8_exhaustive( );
	test_64_bit( );
	// None of the try_ operations, abs_diff or unsigned_abs report errors
	daw_ensure( handler_count == 0 );
	test_ilog2_reports( );
}
