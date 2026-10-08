// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#if defined( __clang__ ) or defined( __GNUC__ )

#include "daw/integers/impl/daw_signed_error_handling.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_attributes.h>
#include <daw/daw_bit_count.h>
#include <daw/daw_consteval.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_is_constant_evaluated.h>
#include <daw/daw_likely.h>
#include <daw/daw_unreachable.h>
#include <daw/traits/daw_traits_is_one_of.h>

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
	requires( daw::traits::is_one_of_v<SignedInteger, signed char, short> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_add( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		static_assert( sizeof( int ) >= sizeof( SignedInteger ) );
		int const r2 = a + b;
		bool const r = not daw::in_range<SignedInteger>( r2 );
		result = static_cast<SignedInteger>( r2 );
		return r;
	}

	template<typename SignedInteger>
	requires( daw::traits::is_one_of_v<SignedInteger, int, long, long long> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_add( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		return __builtin_add_overflow( a, b, &result );
	}

	template<typename SignedInteger>
	requires( daw::traits::is_one_of_v<SignedInteger, signed char, short> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_sub( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		static_assert( sizeof( int ) >= sizeof( SignedInteger ) );
		int const r2 = a - b;
		bool const r = not daw::in_range<SignedInteger>( r2 );
		result = static_cast<SignedInteger>( r2 );
		return r;
	}

	template<typename SignedInteger>
	requires( daw::traits::is_one_of_v<SignedInteger, int, long, long long> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_sub( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		return __builtin_sub_overflow( a, b, &result );
	}

	template<typename SignedInteger>
	requires( daw::traits::is_one_of_v<SignedInteger, signed char, short> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_mul( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		static_assert( sizeof( int ) >= sizeof( SignedInteger ) );
		int const r2 = a * b;
		bool const r = not daw::in_range<SignedInteger>( r2 );
		result = static_cast<SignedInteger>( r2 );
		return r;
	}

	template<typename SignedInteger>
	requires( daw::traits::is_one_of_v<SignedInteger, int, long, long long> )
	DAW_ATTRIB_INLINE constexpr bool
	wrapping_mul( SignedInteger a, SignedInteger b, SignedInteger &result ) {
		return __builtin_mul_overflow( a, b, &result );
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
		result = static_cast<SignedInteger>( lhs / rhs );
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
		result = static_cast<SignedInteger>( lhs % rhs );
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
				return static_cast<T>( lhs % rhs );
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
				return static_cast<T>( lhs % rhs );
			}
		}
	} checked_rem{ };

	inline constexpr struct checked_shl_t {
		explicit checked_shl_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			// A negative rhs becomes a large unsigned value, so one compare
			// covers both negative and too large shift counts
			if( DAW_UNLIKELY( static_cast<std::make_unsigned_t<T>>( rhs ) >=
			                  daw::bit_count_v<T> ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
				auto const count = unsigned_magnitude( rhs );
				if( rhs < T{ } and count < daw::bit_count_v<T> ) {
					return static_cast<T>( lhs >> count );
				}
				return lhs;
			}
			return static_cast<T>( lhs << rhs );
		}
	} checked_shl{ };

	inline constexpr struct checked_shr_t {
		explicit checked_shr_t( ) = default;

		template<ValidIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			// A negative rhs becomes a large unsigned value, so one compare
			// covers both negative and too large shift counts
			if( DAW_UNLIKELY( static_cast<std::make_unsigned_t<T>>( rhs ) >=
			                  daw::bit_count_v<T> ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
				auto const count = unsigned_magnitude( rhs );
				if( rhs < T{ } and count < daw::bit_count_v<T> ) {
					return static_cast<T>( lhs << count );
				}
				return lhs;
			}
			return static_cast<T>( lhs >> rhs );
		}
	} checked_shr{ };
} // namespace daw::integers::sint_impl
#endif
