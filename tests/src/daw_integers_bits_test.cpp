// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

// Tests count_zeros, count_leading_ones, count_trailing_ones, swap_bytes,
// is_power_of_two and next_power_of_two.  8 and 16 bit types are tested
// exhaustively against simple bit by bit references.

#define DAW_DEFAULT_UNSIGNED_CHECKING 0

#include <daw/daw_integer.h>

#include <daw/daw_ensure.h>

#include <cstdint>
#include <optional>

// Usable in constant expressions
static_assert( daw::u32{ 0x1234'5678U }.swap_bytes( ) == 0x7856'3412U );
static_assert( daw::i16{ 0x0102 }.swap_bytes( ) == 0x0201 );
static_assert( daw::i32{ -1 }.count_leading_ones( ) == 32 );
static_assert( daw::u8{ 0b0111U }.count_trailing_ones( ) == 3 );
static_assert( daw::u64{ 1U }.count_zeros( ) == 63 );
static_assert( daw::u32{ 64U }.is_power_of_two( ) );
static_assert( daw::u32{ 65U }.next_power_of_two( ) == 128U );
static_assert( not daw::u32::max( ).try_next_power_of_two( ) );

namespace {
	int overflow_count = 0;

	// References work on the unsigned bit pattern one bit at a time
	template<typename U>
	std::uint32_t ref_count_ones( U u ) {
		auto r = 0U;
		for( auto i = 0U; i < sizeof( U ) * 8U; ++i ) {
			r += ( u >> i ) & 1U;
		}
		return r;
	}

	template<typename U>
	std::uint32_t ref_leading_ones( U u ) {
		auto r = 0U;
		for( auto i = sizeof( U ) * 8U; i-- > 0U and ( ( u >> i ) & 1U ); ) {
			++r;
		}
		return r;
	}

	template<typename U>
	std::uint32_t ref_trailing_ones( U u ) {
		auto r = 0U;
		for( auto i = 0U; i < sizeof( U ) * 8U and ( ( u >> i ) & 1U ); ++i ) {
			++r;
		}
		return r;
	}

	template<typename U>
	U ref_swap_bytes( U u ) {
		auto r = std::uint64_t{ 0 };
		for( auto i = 0U; i < sizeof( U ); ++i ) {
			r = ( r << 8U ) | ( ( static_cast<std::uint64_t>( u ) >> ( 8U * i ) ) &
			                    0xFFU );
		}
		return static_cast<U>( r );
	}

	template<typename U>
	std::optional<std::uint64_t> ref_next_pow2( U u ) {
		auto p = std::uint64_t{ 1 };
		while( p < u ) {
			p *= 2U;
		}
		if( p > static_cast<U>( -1 ) ) {
			return std::nullopt;
		}
		return p;
	}

	template<typename Signed, typename Unsigned>
	void test_exhaustive( ) {
		using s_t = typename Signed::value_type;
		using u_t = typename Unsigned::value_type;
		constexpr auto bits = static_cast<std::uint32_t>( sizeof( u_t ) * 8U );
		for( std::uint64_t n = 0; n <= static_cast<u_t>( -1 ); ++n ) {
			auto const u = static_cast<u_t>( n );
			auto const x = Unsigned( u );
			auto const y = Signed( static_cast<s_t>( u ) );

			daw_ensure( x.count_zeros( ) == bits - ref_count_ones( u ) );
			daw_ensure( y.count_zeros( ) == bits - ref_count_ones( u ) );
			daw_ensure( x.count_leading_ones( ) == ref_leading_ones( u ) );
			daw_ensure( y.count_leading_ones( ) == ref_leading_ones( u ) );
			daw_ensure( x.count_trailing_ones( ) == ref_trailing_ones( u ) );
			daw_ensure( y.count_trailing_ones( ) == ref_trailing_ones( u ) );
			daw_ensure( x.swap_bytes( ) == ref_swap_bytes( u ) );
			daw_ensure( y.swap_bytes( ).as_unsigned( ) == ref_swap_bytes( u ) );
			daw_ensure( x.swap_bytes( ).swap_bytes( ) == x );

			daw_ensure( x.is_power_of_two( ) == ( ref_count_ones( u ) == 1U ) );
			auto const p = ref_next_pow2( u );
			auto const tp = x.try_next_power_of_two( );
			daw_ensure( tp.has_value( ) == p.has_value( ) );
			auto const before = overflow_count;
			if( p ) {
				daw_ensure( *tp == *p );
				daw_ensure( x.next_power_of_two( ) == *p );
				daw_ensure( overflow_count == before );
			} else {
				// Wraps to 0 and reports overflow in checked mode
				daw_ensure( x.next_power_of_two( ) == 0U );
				daw_ensure( overflow_count == before + 1 );
			}
		}
	}

	void test_wide( ) {
		using daw::i64;
		using daw::u32;
		using daw::u64;
		daw_ensure( u64{ 0x0102'0304'0506'0708ULL }.swap_bytes( ) ==
		            0x0807'0605'0403'0201ULL );
		daw_ensure( i64::min( ).swap_bytes( ) == i64{ 0x80 } );
		daw_ensure( u32{ 0xF000'000FU }.count_leading_ones( ) == 4 );
		daw_ensure( u32{ 0xF000'000FU }.count_trailing_ones( ) == 4 );
		daw_ensure( i64{ -1 }.count_trailing_ones( ) == 64 );
		daw_ensure( i64::min( ).count_zeros( ) == 63 );
		daw_ensure( u64{ 0U }.next_power_of_two( ) == 1U );
		daw_ensure( u64{ 1ULL << 63U }.next_power_of_two( ) == 1ULL << 63U );
		daw_ensure( not u64{ ( 1ULL << 63U ) + 1U }.try_next_power_of_two( ) );
		daw_ensure( not u64{ }.is_power_of_two( ) );
		daw_ensure( u64{ 1ULL << 40U }.is_power_of_two( ) );
	}
} // namespace

int main( ) {
	auto handler = []( daw::integers::IntegerErrorType ) { ++overflow_count; };
	daw::integers::register_integer_overflow_handler( handler );

	test_exhaustive<daw::i8, daw::u8>( );
	test_exhaustive<daw::i16, daw::u16>( );
	test_wide( );
}
