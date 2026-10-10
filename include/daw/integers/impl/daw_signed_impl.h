// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include "daw/integers/impl/daw_signed_clanggcc.h"
#include "daw/integers/impl/daw_signed_msvc.h"

#include <daw/daw_as.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_integer_reverse.h>
#include <daw/daw_is_constant_evaluated.h>
#include <daw/daw_traits.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace daw::integers::inline DAW_INTEGER_VER::sint_impl {
	template<typename T>
	using next_wider_or_unsigned_t =
	  std::conditional_t<sizeof( T ) >= 8, make_unsigned_t<T>,
	                     daw::next_wider_t<T>>;

	template<typename T>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr next_wider_or_unsigned_t<T>
	as_next_wider_or_unsigned( T value ) {
		return static_cast<next_wider_or_unsigned_t<T>>( value );
	}

	template<typename Unsigned, std::size_t... Is>
	DAW_ATTRIB_INLINE constexpr Unsigned
	from_bytes_le( unsigned char const *ptr,
	               std::index_sequence<Is...> ) noexcept {
		constexpr auto f = []( unsigned char c, size_t n ) {
			return static_cast<Unsigned>( c ) << ( 8U * n );
		};
		auto result = static_cast<Unsigned>( ( f( ptr[Is], Is ) | ... ) );
		return result;
	}

	template<typename Unsigned, std::size_t... Is>
	DAW_ATTRIB_INLINE constexpr Unsigned
	from_bytes_be( unsigned char const *ptr,
	               std::index_sequence<Is...> ) noexcept {
		constexpr auto StartVal = sizeof( Unsigned ) - 1;
		constexpr auto f = []( unsigned char c, size_t n ) {
			return static_cast<Unsigned>( c ) << ( 8U * n );
		};
		auto result =
		  static_cast<Unsigned>( ( f( ptr[StartVal - Is], Is ) | ... ) );
		return result;
	}

	template<typename Lhs, typename Rhs>
	concept SizeFits = sizeof( Lhs ) <= sizeof( Rhs );

	template<typename SignedInteger, typename Integer>
	inline constexpr bool convertible_signed_int =
	  daw::is_integral_v<Integer> and daw::is_signed_v<Integer> and
	  ( sizeof( Integer ) <= sizeof( SignedInteger ) );
#if defined( DAW_HAS_CPP23_STATIC_CALL_OP ) and DAW_HAS_CLANG_VER_GTE( 17, 0 )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++23-extensions"
#endif

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_add( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
			}
			return result;
		}
	} checked_add{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			(void)wrapping_add( lhs, rhs, result );
			return result;
		}
	} wrapped_add{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_sub( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
			}
			return result;
		}
	} checked_sub{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			(void)wrapping_sub( lhs, rhs, result );
			return result;
		}
	} wrapped_sub{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			if( DAW_UNLIKELY( wrapping_mul( lhs, rhs, result ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
			}
			return result;
		}
	} checked_mul{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( auto result = T{ };
			    DAW_LIKELY( not wrapping_add( lhs, rhs, result ) ) ) {
				DAW_LIKELY_BRANCH
				return result;
			}
			if( rhs < 0 ) {
				return daw::lowest_value<T>;
			}
			return daw::max_value<T>;
		}
	} sat_add{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( auto result = T{ };
			    DAW_LIKELY( not wrapping_sub( lhs, rhs, result ) ) ) {
				DAW_LIKELY_BRANCH
				return result;
			}
			if( rhs < 0 ) {
				return daw::max_value<T>;
			}
			return daw::lowest_value<T>;
		}
	} sat_sub{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( auto result = T{ };
			    DAW_LIKELY( not wrapping_mul( lhs, rhs, result ) ) ) {
				DAW_LIKELY_BRANCH
				return result;
			}
			if( daw::signbit( lhs ) == daw::signbit( rhs ) ) {
				return daw::max_value<T>;
			}
			return daw::lowest_value<T>;
		}
	} sat_mul{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			auto result = T{ };
			(void)wrapping_mul( lhs, rhs, result );
			return result;
		}
	} wrapped_mul{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_add( lhs, rhs );
#elif DAW_DEFAULT_SIGNED_CHECKING == 2
			return wrapped_add( lhs, rhs );
#else
			return as<T>( sint_impl::as_next_wider_or_unsigned( lhs ) +
			              sint_impl::as_next_wider_or_unsigned( rhs ) );
#endif
		}
	} debug_checked_add{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_sub( lhs, rhs );
#elif DAW_DEFAULT_SIGNED_CHECKING == 2
			return wrapped_sub( lhs, rhs );
#else
			return as<T>( sint_impl::as_next_wider_or_unsigned( lhs ) -
			              sint_impl::as_next_wider_or_unsigned( rhs ) );
#endif
		}
	} debug_checked_sub{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_mul( lhs, rhs );
#elif DAW_DEFAULT_SIGNED_CHECKING == 2
			return wrapped_mul( lhs, rhs );
#else
			return as<T>( sint_impl::as_next_wider_or_unsigned( lhs ) *
			              sint_impl::as_next_wider_or_unsigned( rhs ) );
#endif
		}
	} debug_checked_mul{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_div( lhs, rhs );
#else
			return static_cast<T>( lhs / rhs );
#endif
		}
	} debug_checked_div{ };

	inline constexpr struct {
		template<ValidIntType T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			assert( rhs != 0 );
			if( DAW_UNLIKELY( lhs == daw::lowest_value<T> and rhs == T{ -1 } ) ) {
				DAW_UNLIKELY_BRANCH
				return daw::max_value<T>;
			}
			return sint_impl::debug_checked_div( lhs, rhs );
		}
	} sat_div{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_rem( lhs, rhs );
#else
			return static_cast<T>( lhs % rhs );
#endif
		}
	} debug_checked_rem{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
			if( lhs == lowest_value<T> ) {
				[[unlikely]] on_signed_integer_overflow( );
				return lhs;
			}
			return static_cast<T>( -lhs );
		}
	} checked_neg{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_neg( lhs );
#else
			return static_cast<T>( -as_unsigned( lhs ) );
#endif
		}
	} debug_checked_neg{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_shl( lhs, rhs );
#else
			return static_cast<T>( lhs << rhs );
#endif
		}
	} debug_checked_shl{ };

	inline constexpr struct {
		template<SizeFits<std::int64_t> T>
		DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr T
		operator( )( T lhs, T rhs ) DAW_CPP23_STATIC_CALL_OP_CONST {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			return checked_shr( lhs, rhs );
#else
			return static_cast<T>( lhs >> rhs );
#endif
		}
	} debug_checked_shr{ };

#if defined( DAW_HAS_CPP23_STATIC_CALL_OP ) and DAW_HAS_CLANG_VER_GTE( 17, 0 )
#pragma clang diagnostic pop
#endif
} // namespace daw::integers::inline DAW_INTEGER_VER::sint_impl
