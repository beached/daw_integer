// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

namespace {
	template<typename Integer>
	constexpr bool test_sign( ) {
		auto const zero = Integer{ 0 };
		auto const one = Integer{ 1 };
		auto const minus_one = Integer{ -1 };
		auto const maximum = Integer::max( );
		auto const minimum = Integer::min( );

		return not zero.is_negative( ) and not zero.is_positive( ) and
		       zero.signum( ) == zero and not one.is_negative( ) and
		       one.is_positive( ) and one.signum( ) == one and
		       minus_one.is_negative( ) and not minus_one.is_positive( ) and
		       minus_one.signum( ) == minus_one and not maximum.is_negative( ) and
		       maximum.is_positive( ) and maximum.signum( ) == one and
		       minimum.is_negative( ) and not minimum.is_positive( ) and
		       minimum.signum( ) == minus_one;
	}

	template<typename Integer>
	void test_sign_runtime( ) {
		daw_ensure( test_sign<Integer>( ) );
	}
} // namespace

static_assert( test_sign<daw::i8>( ) );
static_assert( test_sign<daw::i16>( ) );
static_assert( test_sign<daw::i32>( ) );
static_assert( test_sign<daw::i64>( ) );

int main( ) {
	test_sign_runtime<daw::i8>( );
	test_sign_runtime<daw::i16>( );
	test_sign_runtime<daw::i32>( );
	test_sign_runtime<daw::i64>( );
}
