// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

#include <type_traits>

// A checked operation that overflows during constant evaluation must not
// produce a value; it must fail to be a constant expression.
namespace {
	template<auto>
	struct constant {};

	template<typename Integer>
	concept constexpr_add_max_one = requires {
		typename constant<Integer::max( ).add_checked( Integer{ 1 } )>;
	};

	template<typename Integer>
	concept constexpr_sub_min_one = requires {
		typename constant<Integer::min( ).sub_checked( Integer{ 1 } )>;
	};

	template<typename Integer>
	concept constexpr_mul_max_two = requires {
		typename constant<Integer::max( ).mul_checked( Integer{ 2 } )>;
	};

	template<typename Integer>
	concept constexpr_neg_min = requires {
		typename constant<Integer::min( ).negate_checked( )>;
	};

	template<typename Integer>
	concept constexpr_add_in_range = requires {
		typename constant<Integer{ 1 }.add_checked( Integer{ 1 } )>;
	};

	template<typename Integer>
	constexpr bool test_constexpr_checked( ) {
		return constexpr_add_in_range<Integer> and
		       not constexpr_add_max_one<Integer> and
		       not constexpr_sub_min_one<Integer> and
		       not constexpr_mul_max_two<Integer> and
		       not constexpr_neg_min<Integer>;
	}
} // namespace

static_assert( test_constexpr_checked<daw::i8>( ) );
static_assert( test_constexpr_checked<daw::i16>( ) );
static_assert( test_constexpr_checked<daw::i32>( ) );
static_assert( test_constexpr_checked<daw::i64>( ) );

int main( ) {}
