// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "impl/daw_signed_error_handling.h"
#include "impl/daw_signed_impl.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_attributes.h>
#include <daw/daw_consteval.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_cxmath.h>
#include <daw/daw_endian.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_likely.h>
#include <daw/traits/daw_traits_is_one_of.h>

#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <exception>
#include <limits>
#include <type_traits>

namespace daw::integers {
	template<std::size_t /*Bits*/>
	struct signed_integer;

	namespace sint_impl {
		template<std::size_t /*Bits*/>
		struct signed_integer_type;

		template<>
		struct signed_integer_type<8> {
			using type = std::int8_t;
		};

		template<>
		struct signed_integer_type<16> {
			using type = std::int16_t;
		};

		template<>
		struct signed_integer_type<32> {
			using type = std::int32_t;
		};

		template<>
		struct signed_integer_type<64> {
			using type = std::int64_t;
		};

		template<std::size_t Bits>
		using signed_integer_type_t = typename signed_integer_type<Bits>::type;

		template<typename, typename = void>
		inline constexpr bool is_signed_integral_v = false;

		template<typename T>
		inline constexpr bool is_signed_integral_v<
		  T, std::enable_if_t<daw::is_integral_v<T> and daw::is_signed_v<T>>> =
		  true;

		template<
		  typename Lhs, typename Rhs,
		  std::enable_if_t<is_signed_integral_v<Lhs> and is_signed_integral_v<Rhs>,
		                   std::nullptr_t> = nullptr>
		using int_result_t =
		  typename std::conditional<( sizeof( Lhs ) >= sizeof( Rhs ) ),
		                            signed_integer<sizeof( Lhs ) * 8>,
		                            signed_integer<sizeof( Rhs ) * 8>>::type;
	} // namespace sint_impl

	using i8 = signed_integer<8>;
	using i16 = signed_integer<16>;
	using i32 = signed_integer<32>;
	using i64 = signed_integer<64>;

	/// @brief Signed Integer type with overflow checked/wrapping/saturated
	/// operations
	template<std::size_t Bits>
	/**
	 * @brief Constructs a signed_integer from an integer type.
	 * @tparam I The type of the input parameter.
	 * @param v The value*/
	struct [[DAW_PREF_NAME( i8 ), DAW_PREF_NAME( i16 ), DAW_PREF_NAME( i32 ),
	         DAW_PREF_NAME( i64 )]] signed_integer {
		using SignedInteger = typename sint_impl::signed_integer_type<Bits>::type;
		static_assert( daw::is_integral_v<SignedInteger> and
		                 daw::is_signed_v<SignedInteger>,
		               "Only signed integer types are supported" );
		using value_type = SignedInteger;
		using reference = value_type &;
		using const_reference = value_type const &;

		/// @brief Returns the maximum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL signed_integer max( ) noexcept {
			return signed_integer( daw::numeric_limits<value_type>::max( ) );
		}

		/// @brief Returns the minimum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL signed_integer min( ) noexcept {
			return signed_integer( daw::numeric_limits<value_type>::min( ) );
		}

		struct private_t {
			value_type value{ };
		} m_private{ };

		explicit signed_integer( ) = default;

		// Construct from an integer type and ensure value_type is large enough
		template<typename I,
		         std::enable_if_t<daw::is_integral_v<I>, std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr explicit signed_integer( I v ) noexcept
		  : m_private{ static_cast<value_type>( v ) } {
			if constexpr( not sint_impl::convertible_signed_int<value_type, I> ) {
				if( DAW_UNLIKELY( not daw::in_range<value_type>( v ) ) ) {
					DAW_UNLIKELY_BRANCH
					on_signed_integer_overflow( );
				}
			}
		}

		/// @brief Creates an integer from a bytes object using little-endian byte
		/// order.
		/// @param ptr A byte array representing the integer in little-endian byte
		/// order
		/// @return The signed_integer represented by the bytes in little-endian
		/// order.
		[[nodiscard]] static constexpr signed_integer
		from_bytes_le( unsigned char const *ptr ) noexcept {
			return signed_integer(
			  daw::integers::sint_impl::from_bytes_le<value_type>(
			    ptr, std::make_index_sequence<sizeof( value_type )>{ } ) );
		}

		/// @brief `from_bytes_be` function that creates an integer from a bytes
		/// object using big-endian byte order.
		/// @param ptr A byte array representing the integer in big-endian byte
		/// order
		/// @return The signed_integer represented by the bytes in big-endian order.
		[[nodiscard]] static constexpr signed_integer
		from_bytes_be( unsigned char const *ptr ) noexcept {
			return signed_integer(
			  daw::integers::sint_impl::from_bytes_be<value_type>(
			    ptr, std::make_index_sequence<sizeof( value_type )>{ } ) );
		}

		/// @brief `conversion_checked` provides safe type conversion operations
		/// with overflow and underflow checks.
		/// @param other Integer to convert to signed_integer
		/// @returns A signed_integer with value of other
		template<typename I,
		         std::enable_if_t<daw::is_integral_v<I> and daw::is_signed_v<I>,
		                          std::nullptr_t> = nullptr>
		[[nodiscard]] static constexpr signed_integer
		conversion_checked( I other ) {
			if( DAW_UNLIKELY( sizeof( I ) > sizeof( value_type ) ) and
			    DAW_UNLIKELY( not daw::in_range<value_type>( other ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
			}
			return signed_integer( static_cast<value_type>( other ) );
		}

		/// @brief `conversion_checked` provides safe type conversion operations
		/// with overflow and underflow checks.
		/// @param other signed_integer of different size to convert
		/// @returns A signed_integer with value of other
		template<std::size_t I>
		[[nodiscard]] static constexpr signed_integer
		conversion_checked( signed_integer<I> other ) {
			return signed_integer( conversion_checked( other.value( ) ) );
		}

		/// @brief Performs an unchecked conversion between signed integer types.
		/// @tparam I The type of the signed integer to be converted.
		/// @param other The signed integer value to convert.
		/// @return The converted signed integer value.
		template<std::size_t I>
		[[nodiscard]] static constexpr signed_integer
		conversion_unchecked( signed_integer<I> other ) {
			return signed_integer( static_cast<value_type>( other.value( ) ) );
		}

		/// @brief Converts another numeric type to a signed integer without bounds
		/// checking.
		/// @tparam I The type of the input parameter to be converted.
		/// @param other The input value to be converted to a signed integer.
		/// @return A `signed_integer` representing the converted value.
		template<typename I,
		         std::enable_if_t<daw::is_integral_v<I>, std::nullptr_t> = nullptr>
		static constexpr signed_integer conversion_unchecked( I other ) {
			return signed_integer( static_cast<value_type>( other ) );
		}

		/// @brief Construct a signed_integer from another that has a larger range
		/// @tparam I The type of the input parameter to be converted.
		/// @param other The input value to be constructed from
		template<std::size_t I,
		         std::enable_if_t<( I > Bits ), std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE explicit constexpr signed_integer(
		  signed_integer<I> other ) noexcept
		  : m_private{ static_cast<value_type>( other.value( ) ) } {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			if( not daw::in_range<value_type>( other.value( ) ) ) {
				on_signed_integer_overflow( );
			}
#endif
		}

		// @brief Construct a signed_integer from another signed_integer of smaller
		// range.  No checks are needed
		template<std::size_t I, std::enable_if_t<( I / 8 <= sizeof( value_type ) ),
		                                         std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer(
		  signed_integer<I> other ) noexcept
		  : m_private{ static_cast<value_type>( other.value( ) ) } {}

		/// @brief Allow conversion to an arithmetic type
		template<typename Arithmetic,
		         std::enable_if_t<daw::is_arithmetic_v<Arithmetic>,
		                          std::nullptr_t> = nullptr>
		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator Arithmetic( ) const noexcept {
			return static_cast<Arithmetic>( value( ) );
		}

		/// @brief Allow conversion to signed_integer types that are larger in range
		template<
		  std::size_t I,
		  std::enable_if_t<sint_impl::convertible_signed_int<
		                     sint_impl::signed_integer_type_t<I>, value_type>,
		                   std::nullptr_t> = nullptr>
		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator signed_integer<I>( ) const noexcept {
			return signed_integer<I>( value( ) );
		}

		/// @brief Access to underlying value
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr value_type
		value( ) const noexcept {
			return m_private.value;
		}

		/// @brief Negate the value performing checks in debug mode
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		operator-( ) const {
			return signed_integer( sint_impl::debug_checked_neg( value( ) ) );
		}

		// @brief Returns the negated value of the signed_integer, ensuring it is
		// within valid range.
		// @return The negated signed_integer value, ensuring it does not overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		negate_checked( ) const {
			return signed_integer( sint_impl::checked_neg( value( ) ) );
		}

		// @brief Negates the current signed_integer without performing any
		// validation checks.
		// @return A new signed_integer instance with the negated value of the
		// current instance.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		negate_unchecked( ) const {
			return signed_integer( -value( ) );
		}

		// @brief Negates the current signed_integer, wrapping when overflow happens
		// @return A new signed_integer instance with the negated value of the
		// current instance.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		negate_wrapped( ) const {
			return mul_wrapped( signed_integer( -1 ) );
		}

		// @brief Negates the current signed_integer, saturating when overflow
		// happens
		// @return A new signed_integer instance with the negated value of the
		// current instance.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		negate_saturated( ) const {
			return mul_saturated( signed_integer( -1 ) );
		}

		/// @brief Computes the bitwise not and returns as a signed integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		operator~( ) const {
			return signed_integer( static_cast<value_type>( ~value( ) ) );
		}

		/// @brief Add signed_integer rhs to self.  Checked in debug mode
		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator+=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_add( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief increment current value.  Checked in debug mode
		DAW_ATTRIB_INLINE constexpr signed_integer &operator++( ) {
			m_private.value =
			  sint_impl::debug_checked_add( value( ), value_type{ 1 } );
			return *this;
		}

		/// @brief increment current value and return previous value.  Checked in
		/// debug mode
		DAW_ATTRIB_INLINE constexpr signed_integer operator++( int ) {
			auto result = *this;
			operator++( );
			return result;
		}

		/// @brief add rhs to current value and return a new signed_integer.
		/// Addition is checked and calls error handler on overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		add_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_add( value( ), rhs.value( ) ) );
		}

		/// @brief add rhs to current value and return a new signed_integer.
		/// Addition is wrapped on overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		add_wrapped( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::wrapped_add( value( ), rhs.value( ) ) );
		}

		/// @brief add rhs to current value and return a new signed_integer. No
		/// overflow checking is performed
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		add_unchecked( signed_integer const &rhs ) const {
			return value( ) + rhs.value( );
		}

		/// @brief saturated addition of rhs and current value and return a new
		/// signed_integer.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		add_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_add( value( ), rhs.value( ) ) );
		}

		/// @brief Add rhs to this and return a ref this this.  Checked while in
		/// debug
		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator+=( I rhs ) {
			return *this += signed_integer( rhs );
		}

		/// @brief Subtract rhs to this and return a ref this this.  Checked while
		/// in debug
		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator-=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_sub( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief Subtract rhs to this and return a ref this this.  Checked while
		/// in debug
		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator-=( I rhs ) {
			return *this -= signed_integer( rhs );
		}

		/// @brief Subtract rhs from this and return a new signed_integer.  Checked
		/// for overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		sub_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_sub( value( ), rhs.value( ) ) );
		}

		/// @brief Subtract rhs from this and return a new signed_integer.  On
		/// overflow value is wrapped
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		sub_wrapped( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::wrapped_sub( value( ), rhs.value( ) ) );
		}

		/// @brief Subtract rhs from this and return a new signed_integer.  No
		/// overflow checking.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		sub_unchecked( signed_integer const &rhs ) const {
			return value( ) - rhs.value( );
		}

		/// @brief Subtract rhs from this and return a new signed_integer.  On
		/// overflow value is saturated.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		sub_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_sub( value( ), rhs.value( ) ) );
		}

		/// @brief prefix increment return a ref to self. Checked in debug
		/// modes.
		DAW_ATTRIB_INLINE constexpr signed_integer &operator--( ) {
			m_private.value =
			  sint_impl::debug_checked_sub( value( ), value_type{ 1 } );
			return *this;
		}

		/// @brief postfix increment return a ref to self. Checked in debug
		/// modes.
		DAW_ATTRIB_INLINE constexpr signed_integer operator--( int ) {
			auto result = *this;
			operator--( );
			return result;
		}

		/// @brief Multiple self with rhs and store the product in this.  Checked in
		/// debug modes
		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator*=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_mul( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief Multiple self with rhs and store the product in this.  Checked in
		/// debug modes
		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator*=( I rhs ) {
			return *this *= signed_integer( rhs );
		}

		/// @brief Perform checked multiplication with rhs and return a new
		/// signed_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		mul_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_mul( value( ), rhs.value( ) ) );
		}

		/// @brief Perform wrapped multiplication with rhs and return a new
		/// signed_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		mul_wrapped( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::wrapped_mul( value( ), rhs.value( ) ) );
		}

		/// @brief Perform unchecked multiplication with rhs and return a new
		/// signed_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		mul_uncheck( signed_integer const &rhs ) const {
			return value( ) * rhs.value( );
		}

		/// @brief Perform saturated multiplication with rhs and return a new
		/// signed_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		mul_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_mul( value( ), rhs.value( ) ) );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator/=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_div( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator/=( I rhs ) {
			return *this /= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_unchecked( signed_integer const &rhs ) const {
			return value( ) / rhs.value( );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_wrapped( signed_integer const &rhs ) const {
			if( value( ) == max( ) and rhs.value( ) == value_type{ -1 } ) {
				return min( );
			}
			return signed_integer(
			  sint_impl::debug_checked_div( value( ), rhs.value( ) ) );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator%=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_rem( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator%=( I rhs ) {
			return *this %= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_rem( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_unchecked( signed_integer const &rhs ) const {
			return value( ) % rhs.value( );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_saturated( signed_integer const &rhs ) const {
			if( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) {
				return 0;
			}
			return sint_impl::debug_checked_div( value( ), rhs.value( ) );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator<<=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_shl( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator<<=( I rhs ) {
			return *this <<= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_shl( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_unchecked( signed_integer const &rhs ) const {
			return value( ) << rhs.value( );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_overflowing( signed_integer n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			} else if( n == 0 ) {
				return *this;
			}
			n &= sizeof( value_type ) * CHAR_BIT - 1;
			return signed_integer( value( ) << n.value( ) );
		}

		template<typename I,
		         std::enable_if_t<daw::is_integral_v<I>, std::nullptr_t> = nullptr>
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_overflowing( I n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			} else if( n == 0 ) {
				return *this;
			}
			n &= sizeof( value_type ) * CHAR_BIT - 1;
			return signed_integer( value( ) << n );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator>>=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_shr( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator>>=( I rhs ) {
			return *this >>= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_shr( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_unchecked( signed_integer const &rhs ) const {
			return value( ) >> rhs.value( );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_overflowing( signed_integer n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			} else if( n == 0 ) {
				return *this;
			}
			n &= sizeof( value_type ) * CHAR_BIT - 1;
			return signed_integer( value( ) >> n.value( ) );
		}

		template<typename I,
		         std::enable_if_t<daw::is_integral_v<I>, std::nullptr_t> = nullptr>
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_overflowing( I n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			} else if( n == 0 ) {
				return *this;
			}
			n &= sizeof( value_type ) * CHAR_BIT - 1;
			return signed_integer( value( ) >> n );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rotate_left( std::size_t n ) const {
			return shl_overflowing( n ) |
			       shr_overflowing( sizeof( value_type ) * CHAR_BIT - n );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rotate_right( std::size_t n ) const {
			return shr_overflowing( n ) |
			       shl_overflowing( sizeof( value_type ) * CHAR_BIT - n );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator|=( signed_integer const &rhs ) noexcept {
			m_private.value |= rhs.value( );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator|=( I rhs ) {
			m_private.value |= static_cast<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator&=( signed_integer const &rhs ) const noexcept {
			m_private.value &= rhs.value( );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator&=( I rhs ) {
			m_private.value &= static_cast<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator^=( signed_integer const &rhs ) const noexcept {
			m_private.value ^= rhs.value( );
			return *this;
		}

		template<typename I,
		         std::enable_if_t<sint_impl::convertible_signed_int<value_type, I>,
		                          std::nullptr_t> = nullptr>
		DAW_ATTRIB_INLINE constexpr signed_integer &operator^=( I rhs ) {
			m_private.value ^= static_cast<value_type>( rhs );
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator bool( ) const noexcept {
			return static_cast<bool>( value( ) );
		}

		// Logical without short circuit
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		And( signed_integer const &rhs ) const noexcept {
			return static_cast<bool>( *this ) and static_cast<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		Or( signed_integer const &rhs ) const noexcept {
			return static_cast<bool>( *this ) or static_cast<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		reverse_bits( ) const noexcept {
			return signed_integer( daw::cxmath::to_signed(
			  daw::integers::reverse_bits( daw::cxmath::to_unsigned( value( ) ) ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_leading_zeros( ) const noexcept {
			return daw::cxmath::count_leading_zeroes(
			  daw::cxmath::to_unsigned( value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_trailing_zeros( ) const noexcept {
			return daw::cxmath::count_trailing_zeros(
			  daw::cxmath::to_unsigned( value( ) ) );
		}
	};

	template<typename I,
	         typename U = std::enable_if_t<
	           daw::traits::is_one_of_v<daw::make_signed_t<I>, std::int8_t,
	                                    std::int16_t, std::int32_t, std::int64_t>,
	           daw::make_signed_t<I>>>
	signed_integer( I ) -> signed_integer<sizeof( U ) * 8>;

	// Addition
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result += result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result += result_t( rhs );
		return result;
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs );
		result += result_t( rhs.value( ) );
		return result;
	}

	// Subtraction
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs.value( ) );
		result += result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs.value( ) );
		result -= result_t( rhs );
		return result;
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs );
		result -= result_t( rhs.value( ) );
		return result;
	}

	// Multiplication
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs.value( ) );
		result *= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result *= result_t( rhs );
		return result;
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs );
		result *= result_t( rhs.value( ) );
		return result;
	}

	// Division
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result /= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result /= result_t( rhs );
		return result;
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs );
		result /= result_t( rhs.value( ) );
		return result;
	}

	// Remainder
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result %= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result %= result_t( rhs );
		return result;
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs );
		result %= result_t( rhs.value( ) );
		return result;
	}

	// Shift Left
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) <<= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) <<= result_t( rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs ) <<= result_t( rhs.value( ) );
	}

	// Shift Right
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) >>= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) >>= result_t( rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs ) >>= result_t( rhs.value( ) );
	}

	// Bitwise Or
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) |= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) |= result_t( rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs ) |= result_t( rhs.value( ) );
	}

	// Bitwise And
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) &= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) &= result_t( rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs ) &= result_t( rhs.value( ) );
	}

	// Bitwise Xor
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) ^= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) ^= result_t( rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( Lhs lhs, signed_integer<Rhs> rhs ) {
		using lhs_t = Lhs;
		using rhs_t = sint_impl::signed_integer_type_t<Rhs>;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs ) ^= result_t( rhs.value( ) );
	}

	// Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator==( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator==( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator==( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_equal( lhs, rhs.value( ) );
	}

	// Not Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator!=( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_not_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator!=( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_not_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_not_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator!=( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_not_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_not_equal( lhs, rhs.value( ) );
	}

	// Less Than
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator<( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_less( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_less( lhs.value( ), rhs ) ) {
		return daw::cmp_less( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_less( lhs, rhs.value( ) ) ) {
		return daw::cmp_less( lhs, rhs.value( ) );
	}

	// Less Than or Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator<=( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_less_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<=( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_less_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_less_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<=( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_less_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_less_equal( lhs, rhs.value( ) );
	}

	// Greater Than
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator>( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_greater( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_greater( lhs.value( ), rhs ) ) {
		return daw::cmp_greater( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_greater( lhs, rhs.value( ) ) ) {
		return daw::cmp_greater( lhs, rhs.value( ) );
	}

	// Less Than or Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator>=( signed_integer<Lhs> lhs, signed_integer<Rhs> rhs ) {
		return daw::cmp_greater_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>=( signed_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_greater_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_greater_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>=( Lhs &&lhs, signed_integer<Rhs> rhs )
	  -> decltype( daw::cmp_greater_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_greater_equal( lhs, rhs.value( ) );
	}

	namespace literals {
		[[nodiscard]] DAW_CONSTEVAL signed_integer<8>
		operator""_i8( unsigned long long v ) noexcept {
			using int_t = std::int8_t;
			if( not daw::in_range<int_t>( v ) ) {
				on_signed_integer_overflow( );
			}
			return signed_integer<8>( static_cast<int_t>( v ) );
		}

		[[nodiscard]] DAW_CONSTEVAL signed_integer<16>
		operator""_i16( unsigned long long v ) noexcept {
			using int_t = std::int16_t;
			if( not daw::in_range<int_t>( v ) ) {
				on_signed_integer_overflow( );
			}
			return signed_integer<16>( static_cast<int_t>( v ) );
		}

		[[nodiscard]] DAW_CONSTEVAL signed_integer<32>
		operator""_i32( unsigned long long v ) noexcept {
			using int_t = std::int32_t;
			if( not daw::in_range<int_t>( v ) ) {
				on_signed_integer_overflow( );
			}
			return signed_integer<32>( static_cast<int_t>( v ) );
		}

		[[nodiscard]] DAW_CONSTEVAL signed_integer<64>
		operator""_i64( unsigned long long v ) noexcept {
			using int_t = std::int64_t;
			if( not daw::in_range<int_t>( v ) ) {
				on_signed_integer_overflow( );
			}
			return signed_integer<64>( static_cast<int_t>( v ) );
		}
	} // namespace literals
} // namespace daw::integers

namespace daw {
	using daw::integers::i16;
	using daw::integers::i32;
	using daw::integers::i64;
	using daw::integers::i8;

	template<>
	struct make_unsigned<daw::integers::i8> {
		using type = std::uint8_t;
	};

	template<>
	struct make_unsigned<daw::integers::i16> {
		using type = std::uint16_t;
	};

	template<>
	struct make_unsigned<daw::integers::i32> {
		using type = std::uint32_t;
	};

	template<>
	struct make_unsigned<daw::integers::i64> {
		using type = std::uint64_t;
	};

	template<>
	struct make_signed<daw::integers::i8> {
		using type = std::uint8_t;
	};

	template<>
	struct make_signed<daw::integers::i16> {
		using type = std::uint16_t;
	};

	template<>
	struct make_signed<daw::integers::i32> {
		using type = std::uint32_t;
	};

	template<>
	struct make_signed<daw::integers::i64> {
		using type = std::uint64_t;
	};
} // namespace daw

namespace std {
	template<std::size_t Bits>
	struct numeric_limits<daw::integers::signed_integer<Bits>> {
		static constexpr bool is_specialized = true;
		static constexpr bool is_signed = true;
		static constexpr bool is_integer = true;
		static constexpr bool is_exact = true;
		static constexpr bool has_infinity = false;
		static constexpr bool has_quiet_NaN = false;
		static constexpr bool has_signaling_NaN = false;
		static constexpr bool has_denorm = false;
		static constexpr bool has_denorm_loss = false;
		static constexpr std::float_round_style round_style =
		  std::round_toward_zero;
		static constexpr bool is_iec559 = false;
		static constexpr bool is_bounded = true;
		// Cannot reasonably guess as it's imp defined for signed
		// static constexpr bool is_modulo = true;
		static constexpr int digits = Bits - 1;

		static constexpr int digits10 = digits * 3 / 10;
		static constexpr int max_digits10 = 0;
		static constexpr int radix = 2;
		static constexpr int min_exponent = 0;
		static constexpr int min_exponent10 = 0;
		static constexpr int max_exponent = 0;
		static constexpr int max_exponent10 = 0;
		// Cannot reasonably define
		// static constexpr bool traps = true;
		static constexpr bool tinyness_before = false;

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		min( ) noexcept {
			return daw::integers::signed_integer<Bits>::min( );
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		max( ) noexcept {
			return daw::integers::signed_integer<Bits>::max( );
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		lowest( ) noexcept {
			return daw::integers::signed_integer<Bits>::min( );
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		epsilon( ) noexcept {
			return 0;
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		round_error( ) noexcept {
			return 0;
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		infinity( ) noexcept {
			return 0;
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		quiet_NaN( ) noexcept {
			return 0;
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		signalling_NaN( ) noexcept {
			return 0;
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		denorm_min( ) noexcept {
			return 0;
		}
	};
} // namespace std
