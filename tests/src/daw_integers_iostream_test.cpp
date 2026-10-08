// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#include <daw/integers/daw_integer_iostream.h>

#include <daw/daw_ensure.h>

#include <iomanip>
#include <sstream>
#include <string>

namespace {
	template<typename T>
	std::string to_str( T v ) {
		auto ss = std::ostringstream{ };
		ss << v;
		return ss.str( );
	}

	template<typename T>
	bool read( std::string const &s, T &v ) {
		auto ss = std::istringstream( s );
		return static_cast<bool>( ss >> v );
	}

	void test_output( ) {
		using namespace daw::integers::literals;
		daw_ensure( to_str( 42_i32 ) == "42" );
		daw_ensure( to_str( -42_i32 ) == "-42" );
		daw_ensure( to_str( daw::i64::min( ) ) == "-9223372036854775808" );
		daw_ensure( to_str( daw::u64::max( ) ) == "18446744073709551615" );
		// 8 bit types are numbers, not characters
		daw_ensure( to_str( 65_i8 ) == "65" );
		daw_ensure( to_str( daw::i8::min( ) ) == "-128" );
		daw_ensure( to_str( 200_u8 ) == "200" );
		// Stream flags are respected
		auto ss = std::ostringstream{ };
		ss << std::hex << std::showbase << 255_u32 << ' ' << std::dec
		   << std::setw( 4 ) << 7_i16;
		daw_ensure( ss.str( ) == "0xff    7" );
		auto ws = std::wostringstream{ };
		ws << 42_u16;
		daw_ensure( ws.str( ) == L"42" );
	}

	void test_input( ) {
		auto i8 = daw::i8{ 5 };
		daw_ensure( read( "  -128", i8 ) and i8 == -128 );
		daw_ensure( read( "127", i8 ) and i8 == 127 );
		// Out of range sets failbit and clamps, like the builtins
		daw_ensure( not read( "128", i8 ) and i8 == daw::i8::max( ) );
		daw_ensure( not read( "-129", i8 ) and i8 == daw::i8::min( ) );
		auto i64 = daw::i64{ };
		daw_ensure( not read( "99999999999999999999", i64 ) and
		            i64 == daw::i64::max( ) );
		auto i32 = daw::i32{ };
		daw_ensure( not read( "99999999999999999999", i32 ) and
		            i32 == daw::i32::max( ) );
		daw_ensure( not read( "-99999999999999999999", i32 ) and
		            i32 == daw::i32::min( ) );

		auto u8 = daw::u8{ };
		daw_ensure( read( "255", u8 ) and u8 == 255U );
		daw_ensure( not read( "256", u8 ) and u8 == daw::u8::max( ) );
		// Negative values are rejected rather than wrapped
		daw_ensure( not read( "-1", u8 ) and u8 == 0U );
		auto u64 = daw::u64{ };
		daw_ensure( read( "18446744073709551615", u64 ) and u64 == daw::u64::max( ) );
		daw_ensure( not read( " -1", u64 ) and u64 == 0U );

		// Not a number fails and stores 0, like the builtins
		auto u32 = daw::u32{ 9U };
		daw_ensure( not read( "abc", u32 ) and u32 == 0U );

		// An unreadable stream leaves the value unchanged
		auto empty = daw::i32{ 9 };
		daw_ensure( not read( "", empty ) and empty == 9 );

		// Several values and stream flags
		auto ss = std::istringstream( "1 2 ff" );
		auto a = daw::u16{ };
		auto b = daw::i16{ };
		auto c = daw::u32{ };
		ss >> a >> b >> std::hex >> c;
		daw_ensure( ss and a == 1U and b == 2 and c == 255U );
	}
} // namespace

int main( ) {
	test_output( );
	test_input( );
}
