// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include "daw/integers/impl/daw_integer_error_handling.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_attributes.h>
#include <daw/daw_bit_count.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_likely.h>
#include <daw/daw_unreachable.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

/// \brief DAW_DEFAULT_UNSIGNED_CHECKING can be defined to the following
/// 0 - Checked with wrapping defaults if not a named op(e.g. add_wrapped)(for
/// add/sub/mul) 1 - Unchecked 2 - Wrapped for add/sub/mul, unchecked for others
/// As unsigned arithmetic is always modular, 1 and 2 only differ in name.
/// Errors are reported through the same handlers as signed_integer
#if not defined( DAW_DEFAULT_UNSIGNED_CHECKING )
#if defined( DEBUG ) or not defined( NDEBUG )
#define DAW_DEFAULT_UNSIGNED_CHECKING 0
#else
#define DAW_DEFAULT_UNSIGNED_CHECKING 1
#endif
#endif

namespace daw::integers::inline DAW_INTEGER_VER::uint_impl {
	template<typename T>
	concept ValidUIntType = daw::is_integral_v<T> and daw::is_unsigned_v<T> and
	                        not std::is_same_v<T, bool> and
	                        sizeof( T ) <= sizeof( std::uint64_t );

	/// Unsigned types smaller than int promote to signed int, which can overflow
	/// on multiplication.  Do arithmetic in at least unsigned int to keep it
	/// modular.
	template<ValidUIntType T>
	using promoted_t = decltype( T{ } + 0U );

	template<ValidUIntType T>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr promoted_t<T> promote( T value ) {
		return static_cast<promoted_t<T>>( value );
	}

	template<typename Unsigned, std::size_t... Is>
	DAW_ATTRIB_INLINE constexpr Unsigned
	from_bytes_le( unsigned char const *ptr,
	               std::index_sequence<Is...> ) noexcept {
		constexpr auto f = []( unsigned char c, size_t n ) {
			return static_cast<promoted_t<Unsigned>>( c ) << ( 8U * n );
		};
		return static_cast<Unsigned>( ( f( ptr[Is], Is ) | ... ) );
	}

	template<typename Unsigned, std::size_t... Is>
	DAW_ATTRIB_INLINE constexpr Unsigned
	from_bytes_be( unsigned char const *ptr,
	               std::index_sequence<Is...> ) noexcept {
		constexpr auto StartVal = sizeof( Unsigned ) - 1;
		constexpr auto f = []( unsigned char c, size_t n ) {
			return static_cast<promoted_t<Unsigned>>( c ) << ( 8U * n );
		};
		return static_cast<Unsigned>( ( f( ptr[StartVal - Is], Is ) | ... ) );
	}

	template<typename UnsignedInteger, typename Integer>
	inline constexpr bool convertible_unsigned_int =
	  daw::is_integral_v<Integer> and daw::is_unsigned_v<Integer> and
	  not std::is_same_v<Integer, bool> and
	  ( sizeof( Integer ) <= sizeof( UnsignedInteger ) );

	template<ValidUIntType T>
	DAW_ATTRIB_INLINE constexpr bool wrapping_add( T a, T b, T &result ) {
		result = static_cast<T>( promote( a ) + promote( b ) );
		return result < a;
	}

	template<ValidUIntType T>
	DAW_ATTRIB_INLINE constexpr bool wrapping_sub( T a, T b, T &result ) {
		result = static_cast<T>( promote( a ) - promote( b ) );
		return a < b;
	}

	template<ValidUIntType T>
	DAW_ATTRIB_INLINE constexpr bool wrapping_mul( T a, T b, T &result ) {
		if constexpr( sizeof( T ) < sizeof( std::uint64_t ) ) {
			auto const r = static_cast<std::uint64_t>( a ) *
			               static_cast<std::uint64_t>( b );
			result = static_cast<T>( r );
			return r >  daw::max_value<T>;
		} else {
#if defined( __GNUC__ ) or defined( __clang__ )
			return __builtin_mul_overflow( a, b, &result );
#else
			result = static_cast<T>( a * b );
			return a != 0 and result / a != b;
#endif
		}
	}

	template<ValidUIntType T>
	DAW_ATTRIB_INLINE constexpr IntegerErrorType
	wrapping_div( T lhs, T rhs, T &result ) {
		if( rhs == 0 ) {
			result = lhs;
			[[unlikely]] return IntegerErrorType::DivideByZero;
		}
		result = static_cast<T>( lhs / rhs );
		return IntegerErrorType::None;
	}

	template<ValidUIntType T>
	DAW_ATTRIB_INLINE constexpr IntegerErrorType
	wrapping_rem( T lhs, T rhs, T &result ) {
		if( rhs == 0 ) {
			result = lhs;
			[[unlikely]] return IntegerErrorType::DivideByZero;
		}
		result = static_cast<T>( lhs % rhs );
		return IntegerErrorType::None;
	}

#if defined( DAW_HAS_CPP23_STATIC_CALL_OP ) and DAW_HAS_CLANG_VER_GTE( 17, 0 )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++23-extensions"
#endif

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_add( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return result;
		}
	} checked_add{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return static_cast<T>( promote( lhs ) + promote( rhs ) );
		}
	} wrapped_add{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_sub( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return result;
		}
	} checked_sub{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return static_cast<T>( promote( lhs ) - promote( rhs ) );
		}
	} wrapped_sub{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_mul( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return result;
		}
	} checked_mul{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return static_cast<T>( promote( lhs ) * promote( rhs ) );
		}
	} wrapped_mul{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( auto result = T{ };
			    DAW_LIKELY( not wrapping_add( lhs, rhs, result ) ) ) {
				DAW_LIKELY_BRANCH
				return result;
			}
			return  daw::max_value<T>;
		}
	} sat_add{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( DAW_LIKELY( lhs >= rhs ) ) {
				DAW_LIKELY_BRANCH
				return static_cast<T>( lhs - rhs );
			}
			return T{ };
		}
	} sat_sub{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( auto result = T{ };
			    DAW_LIKELY( not wrapping_mul( lhs, rhs, result ) ) ) {
				DAW_LIKELY_BRANCH
				return result;
			}
			return  daw::max_value<T>;
		}
	} sat_mul{ };

	inline constexpr struct checked_div_t {
		explicit checked_div_t( ) = default;

		template<ValidUIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( DAW_UNLIKELY( rhs == 0 ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_div_by_zero( );
				return lhs;
			}
			return static_cast<T>( lhs / rhs );
		}
	} checked_div{ };

	inline constexpr struct checked_rem_t {
		explicit checked_rem_t( ) = default;

		template<ValidUIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( DAW_UNLIKELY( rhs == 0 ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_div_by_zero( );
				return lhs;
			}
			return static_cast<T>( lhs % rhs );
		}
	} checked_rem{ };

	inline constexpr struct checked_shl_t {
		explicit checked_shl_t( ) = default;

		template<ValidUIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( DAW_UNLIKELY( rhs >= daw::bit_count_v<T> ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
				return lhs;
			}
			return static_cast<T>( promote( lhs ) << rhs );
		}
	} checked_shl{ };

	inline constexpr struct checked_shr_t {
		explicit checked_shr_t( ) = default;

		template<ValidUIntType T>
		DAW_ATTRIB_INLINE constexpr T operator( )( T lhs, T rhs ) const {
			if( DAW_UNLIKELY( rhs >= daw::bit_count_v<T> ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
				return lhs;
			}
			return static_cast<T>( promote( lhs ) >> rhs );
		}
	} checked_shr{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_add( lhs, rhs );
#else
			return wrapped_add( lhs, rhs );
#endif
		}
	} debug_checked_add{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_sub( lhs, rhs );
#else
			return wrapped_sub( lhs, rhs );
#endif
		}
	} debug_checked_sub{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_mul( lhs, rhs );
#else
			return wrapped_mul( lhs, rhs );
#endif
		}
	} debug_checked_mul{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_div( lhs, rhs );
#else
			return static_cast<T>( lhs / rhs );
#endif
		}
	} debug_checked_div{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_rem( lhs, rhs );
#else
			return static_cast<T>( lhs % rhs );
#endif
		}
	} debug_checked_rem{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_shl( lhs, rhs );
#else
			return static_cast<T>( promote( lhs ) << rhs );
#endif
		}
	} debug_checked_shl{ };

	inline constexpr struct {
		template<ValidUIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			return checked_shr( lhs, rhs );
#else
			return static_cast<T>( promote( lhs ) >> rhs );
#endif
		}
	} debug_checked_shr{ };

#if defined( DAW_HAS_CPP23_STATIC_CALL_OP ) and DAW_HAS_CLANG_VER_GTE( 17, 0 )
#pragma clang diagnostic pop
#endif
} // namespace daw::integers::inline DAW_INTEGER_VER::uint_impl
