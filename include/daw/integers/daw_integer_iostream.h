// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/daw_integer.h"

#include <cstddef>
#include <istream>
#include <limits>
#include <ostream>

/// Stream operators for signed_integer and unsigned_integer.
/// i8 and u8 are written and read as numbers, not characters.  Reading a value
/// that is out of range sets failbit and stores the nearest limit, matching the
/// builtin integer extractors.  Unsigned extraction also rejects a leading '-'
/// instead of wrapping like the builtin extractors
namespace daw::integers {
	template<typename CharT, typename Traits, std::size_t Bits>
	std::basic_ostream<CharT, Traits> &
	operator<<( std::basic_ostream<CharT, Traits> &os, signed_integer<Bits> v ) {
		// Promote so that i8 is not written as a character
		return os << +v.value( );
	}

	template<typename CharT, typename Traits, std::size_t Bits>
	std::basic_ostream<CharT, Traits> &
	operator<<( std::basic_ostream<CharT, Traits> &os,
	            unsigned_integer<Bits> v ) {
		// Promote so that u8 is not written as a character
		return os << +v.value( );
	}

	template<typename CharT, typename Traits, std::size_t Bits>
	std::basic_istream<CharT, Traits> &
	operator>>( std::basic_istream<CharT, Traits> &is, signed_integer<Bits> &v ) {
		using value_type = typename signed_integer<Bits>::value_type;
		// Leave v unchanged if the stream cannot be read, like the builtins
		if( typename std::basic_istream<CharT, Traits>::sentry s( is ); not s ) {
			return is;
		}
		long long tmp = 0;
		is >> tmp;
		if( tmp < std::numeric_limits<value_type>::min( ) ) {
			v = signed_integer<Bits>::min( );
			is.setstate( std::ios_base::failbit );
		} else if( tmp > std::numeric_limits<value_type>::max( ) ) {
			v = signed_integer<Bits>::max( );
			is.setstate( std::ios_base::failbit );
		} else {
			v = signed_integer<Bits>( tmp, signed_integer<Bits>::unchecked );
		}
		return is;
	}

	template<typename CharT, typename Traits, std::size_t Bits>
	std::basic_istream<CharT, Traits> &
	operator>>( std::basic_istream<CharT, Traits> &is,
	            unsigned_integer<Bits> &v ) {
		using value_type = typename unsigned_integer<Bits>::value_type;
		// Leave v unchanged if the stream cannot be read, like the builtins
		if( typename std::basic_istream<CharT, Traits>::sentry s( is ); not s ) {
			return is;
		}
		// The builtin extractors accept "-1" and wrap it, reject it instead
		if( Traits::eq_int_type( is.peek( ),
		                         Traits::to_int_type( is.widen( '-' ) ) ) ) {
			v = unsigned_integer<Bits>( );
			is.setstate( std::ios_base::failbit );
			return is;
		}
		unsigned long long tmp = 0;
		is >> tmp;
		if( tmp > std::numeric_limits<value_type>::max( ) ) {
			v = unsigned_integer<Bits>::max( );
			is.setstate( std::ios_base::failbit );
		} else {
			v = unsigned_integer<Bits>( tmp, unsigned_integer<Bits>::unchecked );
		}
		return is;
	}
} // namespace daw::integers
