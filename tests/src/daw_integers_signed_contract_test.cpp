// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <daw/integers/daw_signed.h>

#include <concepts>
#include <limits>
#include <type_traits>

namespace {
	template<typename T>
	concept has_is_modulo = requires { std::numeric_limits<T>::is_modulo; };

	template<typename T>
	concept has_traps = requires { std::numeric_limits<T>::traps; };

	template<typename T>
	concept has_signaling_nan =
	  requires { std::numeric_limits<T>::signaling_NaN( ); };

	template<typename T>
	concept const_bitand_assignable =
	  requires( T const lhs, T rhs ) { lhs &= rhs; };

	template<typename T>
	concept const_bitxor_assignable =
	  requires( T const lhs, T rhs ) { lhs ^= rhs; };
} // namespace

static_assert( not noexcept( daw::i8{ 128 } ) );
static_assert( not noexcept( daw::i8{ daw::i16{ 128 } } ) );

static_assert( std::same_as<daw::make_signed_t<daw::i8>, std::int8_t> );
static_assert( std::same_as<daw::make_signed_t<daw::i16>, std::int16_t> );
static_assert( std::same_as<daw::make_signed_t<daw::i32>, std::int32_t> );
static_assert( std::same_as<daw::make_signed_t<daw::i64>, std::int64_t> );

static_assert( has_is_modulo<daw::i32> );
static_assert( has_traps<daw::i32> );
static_assert( has_signaling_nan<daw::i32> );
static_assert(
  std::same_as<decltype( std::numeric_limits<daw::i32>::has_denorm ),
               std::float_denorm_style const> );

static_assert( not const_bitand_assignable<daw::i32> );
static_assert( not const_bitxor_assignable<daw::i32> );

int main( ) {}
