// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include "daw/daw_integer.h"

#include <daw/traits/daw_traits_is_istream_like.h>
#include <daw/traits/daw_traits_is_ostream_like.h>

#include <cstddef>
#include <limits>

/// Stream operators for signed_integer and unsigned_integer.
/// i8 and u8 are written and read as numbers, not characters.  Reading a value
/// that is out of range sets failbit and stores the nearest limit, matching the
/// builtin integer extractors.  Unsigned extraction also rejects a leading '-'
/// instead of wrapping like the builtin extractors
namespace daw::integers::inline DAW_INTEGER_VER::sint_impl {
	template<typename OStream, std::size_t Bits>
	requires(
	  daw::traits::is_ostream_like_v<OStream, typename OStream::char_type> )
	OStream &operator<<( OStream &os, signed_integer<Bits> v ) {
		// Promote so that i8 is not written as a character
		os << +v.value( );
		return os;
	}

	template<typename IStream, std::size_t Bits>
	requires(
	  daw::traits::is_istream_like_v<IStream, typename IStream::char_type> )
	IStream &operator>>( IStream &is, signed_integer<Bits> &v ) {
		using value_type = typename signed_integer<Bits>::value_type;
		// Leave v unchanged if the stream cannot be read, like the builtins
		if( typename IStream::sentry s( is ); not s ) {
			return is;
		}
		long long tmp = 0;
		is >> tmp;
		if( tmp < daw::lowest_value<value_type> ) {
			v = signed_integer<Bits>::min( );
			is.setstate( IStream::failbit );
		} else if( tmp > daw::max_value<value_type> ) {
			v = signed_integer<Bits>::max( );
			is.setstate( IStream::failbit );
		} else {
			v = signed_integer<Bits>( tmp, signed_integer<Bits>::unchecked );
		}
		return is;
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::sint_impl

namespace daw::integers::inline DAW_INTEGER_VER::uint_impl {
	template<typename OStream, std::size_t Bits>
	requires(
	  daw::traits::is_ostream_like_v<OStream, typename OStream::char_type> )
	OStream &operator<<( OStream &os, unsigned_integer<Bits> v ) {
		// Promote so that u8 is not written as a character
		os << +v.value( );
		return os;
	}

	template<typename IStream, std::size_t Bits>
	requires(
	  daw::traits::is_istream_like_v<IStream, typename IStream::char_type> )
	IStream &operator>>( IStream &is, unsigned_integer<Bits> &v ) {
		using value_type = typename unsigned_integer<Bits>::value_type;
		// Leave v unchanged if the stream cannot be read, like the builtins
		if( typename IStream::sentry s( is ); not s ) {
			return is;
		}
		// The builtin extractors accept "-1" and wrap it, reject it instead
		using traits_type = typename IStream::traits_type;
		if( traits_type::eq_int_type(
		      is.peek( ), traits_type::to_int_type( is.widen( '-' ) ) ) ) {
			v = unsigned_integer<Bits>( );
			is.setstate( IStream::failbit );
			return is;
		}
		unsigned long long tmp = 0;
		is >> tmp;
		if( tmp > daw::max_value<value_type> ) {
			v = unsigned_integer<Bits>::max( );
			is.setstate( IStream::failbit );
		} else {
			v = unsigned_integer<Bits>( tmp, unsigned_integer<Bits>::unchecked );
		}
		return is;
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::uint_impl
