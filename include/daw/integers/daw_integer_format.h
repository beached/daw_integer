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

#include <cstddef>
#include <format>

/// std::format support for signed_integer and unsigned_integer.  All of the
/// format specs of the underlying value_type are supported, e.g. "{:#x}" or
/// "{:>8}".  i8 and u8 are formatted as numbers, not characters
namespace std {
	template<std::size_t Bits, typename CharT>
	struct formatter<daw::integers::signed_integer<Bits>, CharT>
	  : formatter<typename daw::integers::signed_integer<Bits>::value_type,
	              CharT> {
		template<typename FormatContext>
		auto format( daw::integers::signed_integer<Bits> v,
		             FormatContext &ctx ) const {
			return formatter<typename daw::integers::signed_integer<Bits>::value_type,
			                 CharT>::format( v.value( ), ctx );
		}
	};

	template<std::size_t Bits, typename CharT>
	struct formatter<daw::integers::unsigned_integer<Bits>, CharT>
	  : formatter<typename daw::integers::unsigned_integer<Bits>::value_type,
	              CharT> {
		template<typename FormatContext>
		auto format( daw::integers::unsigned_integer<Bits> v,
		             FormatContext &ctx ) const {
			return formatter<
			  typename daw::integers::unsigned_integer<Bits>::value_type,
			  CharT>::format( v.value( ), ctx );
		}
	};
} // namespace std
