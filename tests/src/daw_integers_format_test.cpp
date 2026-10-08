// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#include <daw/integers/daw_integer_format.h>

#include <daw/daw_ensure.h>

#include <format>
#include <string>

int main( ) {
	using namespace daw::integers::literals;
	daw_ensure( std::format( "{}", 42_i32 ) == "42" );
	daw_ensure( std::format( "{}", -42_i32 ) == "-42" );
	daw_ensure( std::format( "{}", daw::i64::min( ) ) == "-9223372036854775808" );
	daw_ensure( std::format( "{}", daw::u64::max( ) ) == "18446744073709551615" );
	// 8 bit types are numbers, not characters
	daw_ensure( std::format( "{}", 65_i8 ) == "65" );
	daw_ensure( std::format( "{}", 200_u8 ) == "200" );
	daw_ensure( std::format( "{}", daw::i8::min( ) ) == "-128" );
	// The underlying format specs are supported
	daw_ensure( std::format( "{:#x}", 255_u32 ) == "0xff" );
	daw_ensure( std::format( "{:08b}", 5_u8 ) == "00000101" );
	daw_ensure( std::format( "{:>5}", 7_i16 ) == "    7" );
	daw_ensure( std::format( "{:+}", 7_i64 ) == "+7" );
	daw_ensure( std::format( "{:c}", 65_i32 ) == "A" );
	daw_ensure( std::format( L"{}", 42_u16 ) == L"42" );
}
