// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

#include <limits>

namespace {
	template<typename Integer>
	constexpr bool test_numeric_limits( ) {
		using limits = std::numeric_limits<Integer>;
		using value_limits = std::numeric_limits<typename Integer::value_type>;
		auto const zero = Integer{ 0 };
		bool const is_specialized = limits::is_specialized;
		bool const is_signed = limits::is_signed;
		bool const is_integer = limits::is_integer;
		bool const is_exact = limits::is_exact;
		bool const is_bounded = limits::is_bounded;
		// Default operations are checked/unchecked, not wrapping
		bool const is_not_modulo = not limits::is_modulo;
		bool const digits = limits::digits == value_limits::digits;
		bool const digits10 = limits::digits10 == value_limits::digits10;
		bool const radix = limits::radix == value_limits::radix;
		bool const min = limits::min( ) == Integer::min( );
		bool const max = limits::max( ) == Integer::max( );
		bool const lowest = limits::lowest( ) == Integer::min( );
		bool const epsilon = limits::epsilon( ) == zero;
		bool const round_error = limits::round_error( ) == zero;
		bool const infinity = limits::infinity( ) == zero;
		bool const quiet_NaN = limits::quiet_NaN( ) == zero;
		bool const signaling_NaN = limits::signaling_NaN( ) == zero;
		bool const denorm_min = limits::denorm_min( ) == zero;
		return is_specialized and is_signed and is_integer and is_exact and
		       is_bounded and is_not_modulo and digits and digits10 and radix and
		       min and max and lowest and epsilon and round_error and infinity and
		       quiet_NaN and signaling_NaN and denorm_min;
	}
} // namespace

static_assert( test_numeric_limits<daw::i8>( ) );
static_assert( test_numeric_limits<daw::i16>( ) );
static_assert( test_numeric_limits<daw::i32>( ) );
static_assert( test_numeric_limits<daw::i64>( ) );

int main( ) {}
