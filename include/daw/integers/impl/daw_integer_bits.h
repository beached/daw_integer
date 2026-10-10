// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include <daw/daw_attributes.h>

#include <cstddef>
#include <type_traits>
#include <utility>

/// Bit helpers shared by signed_integer and unsigned_integer
namespace daw::integers::inline DAW_INTEGER_VER::int_impl {
	template<typename Unsigned, std::size_t... Is>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr Unsigned
	swap_bytes_impl( Unsigned value, std::index_sequence<Is...> ) noexcept {
		using promoted_t = decltype( value + 0U );
		constexpr auto last = sizeof( Unsigned ) - 1U;
		// Move byte Is to byte last - Is
		return static_cast<Unsigned>(
		  ( ( ( ( static_cast<promoted_t>( value ) >> ( 8U * Is ) ) & 0xFFU )
		      << ( 8U * ( last - Is ) ) ) |
		    ... ) );
	}

	/// @brief Reverses the byte order of an unsigned integer.  std::byteswap
	/// is C++23, this works in C++20
	template<typename Unsigned>
	requires( std::is_unsigned_v<Unsigned> ) //
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr Unsigned
	swap_bytes( Unsigned value ) noexcept {
		return swap_bytes_impl(
		  value, std::make_index_sequence<sizeof( Unsigned )>{ } );
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::int_impl
