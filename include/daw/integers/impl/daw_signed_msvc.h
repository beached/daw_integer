// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#if defined( _MSC_VER ) and not defined( __clang__ )

#include "daw/integers/impl/daw_signed_error_handling.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_attributes.h>
#include <daw/daw_bit_count.h>
#include <daw/daw_consteval.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_is_constant_evaluated.h>
#include <daw/daw_likely.h>
#include <daw/daw_unreachable.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>

namespace daw::integers::sint_impl {
	template<typename T>
	concept ValidIntType = daw::is_integral_v<T> and daw::is_signed_v<T> and
	                       sizeof( T ) <= sizeof( std::int64_t );

	template<ValidIntType T>
	DAW_ATTRIB_INLINE constexpr std::make_unsigned_t<T>
	unsigned_magnitude( T value ) {
		using unsigned_t = std::make_unsigned_t<T>;
		auto const unsigned_value = static_cast<unsigned_t>( value );
		if( value < 0 ) {
			return unsigned_t{ 0 } - unsigned_value;
		}
		return unsigned_value;
	}

	template<typename SignedInteger>
	constexpr bool wrapping_add( SignedInteger a, SignedInteger b,
	                             SignedInteger &result ) {
		static_assert( daw::is_integral_v<SignedInteger> and
		                 daw::is_signed_v<SignedInteger> and
		                 sizeof( SignedInteger ) <= 8U,
		               "Invalid signed integer" );
		using unsigned_t = std::make_unsigned_t<SignedInteger>;
		auto const unsigned_result =
		  static_cast<unsigned_t>( a ) + static_cast<unsigned_t>( b );
		result = static_cast<SignedInteger>( unsigned_result );
		return ( b > 0 and
		         a > ( daw::numeric_limits<SignedInteger>::max( ) - b ) ) or
		       ( b < 0 and
		         a < ( daw::numeric_limits<SignedInteger>::min( ) - b ) );
	}

	template<typename SignedInteger>
	constexpr bool wrapping_sub( SignedInteger a, SignedInteger b,
	                             SignedInteger &result ) {
		static_assert( daw::is_integral_v<SignedInteger> and
		                 daw::is_signed_v<SignedInteger> and
		                 sizeof( SignedInteger ) <= 8U,
		               "Invalid signed integer" );
		using unsigned_t = std::make_unsigned_t<SignedInteger>;
		auto const unsigned_result =
		  static_cast<unsigned_t>( a ) - static_cast<unsigned_t>( b );
		result = static_cast<SignedInteger>( unsigned_result );
		return ( b > 0 and
		         a < ( daw::numeric_limits<SignedInteger>::min( ) + b ) ) or
		       ( b < 0 and
		         a > ( daw::numeric_limits<SignedInteger>::max( ) + b ) );
	}

	template<typename SignedInteger>
	constexpr bool wrapping_mul( SignedInteger a, SignedInteger b,
	                             SignedInteger &result ) noexcept {
		static_assert( daw::is_integral_v<SignedInteger> and
		                 daw::is_signed_v<SignedInteger> and
		                 sizeof( SignedInteger ) <= 8U,
		               "Invalid signed integer" );
		using unsigned_t = std::make_unsigned_t<SignedInteger>;
		auto const unsigned_result = static_cast<unsigned_t>(
		  static_cast<std::uint64_t>( static_cast<unsigned_t>( a ) ) *
		  static_cast<std::uint64_t>( static_cast<unsigned_t>( b ) ) );
		result = static_cast<SignedInteger>( unsigned_result );

		if( a == 0 or b == 0 ) {
			return false;
		}
		if( a == SignedInteger{ -1 } ) {
			return b == daw::numeric_limits<SignedInteger>::min( );
		}
		if( b == SignedInteger{ -1 } ) {
			return a == daw::numeric_limits<SignedInteger>::min( );
		}
		if( a > 0 ) {
			if( b > 0 ) {
				return a > daw::numeric_limits<SignedInteger>::max( ) / b;
			}
			return b < daw::numeric_limits<SignedInteger>::min( ) / a;
		}
		if( b > 0 ) {
			return a < daw::numeric_limits<SignedInteger>::min( ) / b;
		}
		return a < daw::numeric_limits<SignedInteger>::max( ) / b;
	}

	template<typename SignedInteger>
	DAW_ATTRIB_INLINE constexpr SignedIntegerErrorType
	wrapping_div( SignedInteger lhs, SignedInteger rhs, SignedInteger &result ) {
		if( rhs == 0 ) {
			result = lhs;
			[[unlikely]] return SignedIntegerErrorType::DivideByZero;
		}
		if( lhs == min_value<SignedInteger> and rhs == SignedInteger{ -1 } ) {
			[[unlikely]] result = min_value<SignedInteger>;
			return SignedIntegerErrorType::Overflow;
		}
		result = lhs / rhs;
		return SignedIntegerErrorType::None;
	}

	template<typename SignedInteger>
	DAW_ATTRIB_INLINE constexpr SignedIntegerErrorType
	wrapping_rem( SignedInteger lhs, SignedInteger rhs, SignedInteger &result ) {
		if( rhs == 0 ) {
			result = lhs;
			[[unlikely]] return SignedIntegerErrorType::DivideByZero;
		}
		if( lhs == min_value<SignedInteger> and rhs == SignedInteger{ -1 } ) {
			[[unlikely]] result = SignedInteger{ };
			return SignedIntegerErrorType::Overflow;
		}
		result = lhs % rhs;
		return SignedIntegerErrorType::None;
	}

	inline constexpr struct checked_div_t {
		explicit checked_div_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			T result;
			SignedIntegerErrorType r = wrapping_div( lhs, rhs, result );
			switch( r ) {
			case SignedIntegerErrorType::None:
				return result;
				break;
			case SignedIntegerErrorType::Overflow:
				on_signed_integer_overflow( );
				return result;
				break;
			case SignedIntegerErrorType::DivideByZero:
				on_signed_integer_div_by_zero( );
				return result;
			}
			DAW_UNREACHABLE( );
		}
	} checked_div{ };

	inline constexpr struct checked_rem_t {
		explicit checked_rem_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			DAW_IF_CONSTEVAL {
				return lhs % rhs;
			}
			else {
				if( DAW_UNLIKELY( rhs == 0 ) ) {
					on_signed_integer_div_by_zero( );
					return lhs;
				}
				if( lhs == daw::numeric_limits<T>::min( ) and rhs == T{ -1 } ) {
					on_signed_integer_overflow( );
					return T{ };
				}
				return lhs % rhs;
			}
		}
	} checked_rem{ };

	inline constexpr struct checked_shl_t {
		explicit checked_shl_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( rhs == 0 ) {
				return lhs;
			}
			auto const count = unsigned_magnitude( rhs );
			if( DAW_UNLIKELY( count >= daw::bit_count_v<T> ) ) {
				on_signed_integer_overflow( );
				return lhs;
			}
			if( rhs < 0 ) {
				on_signed_integer_overflow( );
				return static_cast<T>( lhs >> count );
			}
			return static_cast<T>( lhs << count );
		}
	} checked_shl{ };

	inline constexpr struct checked_shr_t {
		explicit checked_shr_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( rhs == 0 ) {
				return lhs;
			}
			auto const count = unsigned_magnitude( rhs );
			if( DAW_UNLIKELY( count >= daw::bit_count_v<T> ) ) {
				on_signed_integer_overflow( );
				return lhs;
			}
			if( rhs < 0 ) {
				on_signed_integer_overflow( );
				return static_cast<T>( lhs << count );
			}
			return static_cast<T>( lhs >> count );
		}
	} checked_shr{ };
} // namespace daw::integers::sint_impl
#endif
