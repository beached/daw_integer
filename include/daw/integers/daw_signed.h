// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include "daw/integers/impl/daw_integer_bits.h"
#include "daw/integers/impl/daw_integer_fwd.h"
#include "daw/integers/impl/daw_signed_error_handling.h"
#include "daw/integers/impl/daw_signed_impl.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_as.h>
#include <daw/daw_attributes.h>
#include <daw/daw_bit_count.h>
#include <daw/daw_consteval.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_cxmath.h>
#include <daw/daw_endian.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_likely.h>
#include <daw/traits/daw_traits_is_one_of.h>

#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstdint>
#include <exception>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace daw::integers::inline DAW_INTEGER_VER::sint_impl {
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
	using signed_integer_type_t = signed_integer_type<Bits>::type;

	template<typename T>
	inline constexpr bool is_signed_integral_v =
	  daw::is_integral_v<T> and daw::is_signed_v<T>;

	template<typename T>
	concept SignedIntegral = is_signed_integral_v<T>;

	template<SignedIntegral Lhs, SignedIntegral Rhs>
	using int_result_t = std::conditional_t<( sizeof( Lhs ) >= sizeof( Rhs ) ),
	                                        signed_integer<sizeof( Lhs ) * 8>,
	                                        signed_integer<sizeof( Rhs ) * 8>>;

	template<std::size_t Bits>
	DAW_ATTRIB_FLATINLINE constexpr signed_integer<Bits>
	pow_impl( signed_integer<Bits> base, unsigned exp, auto &&multiplier ) {
		auto result = signed_integer<Bits>{ 1 }; // Initialize the result to 1

		while( exp != 0 ) {        // Loop until the exponent becomes zero
			if( ( exp & 1 ) == 1 ) { // If the least significant bit of exp is set
				result =
				  multiplier( result, base ); // Multiply the result by the current base
			}
			exp /= 2U;
			if( exp != 0 ) {
				base = multiplier( base, base );
			}
		}
		return result; // Return the final result
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::sint_impl

namespace daw::integers::inline DAW_INTEGER_VER {
	using i8 = signed_integer<8>;
	using i16 = signed_integer<16>;
	using i32 = signed_integer<32>;
	using i64 = signed_integer<64>;

	/// @brief Signed Integer type with overflow checked/wrapping/saturated
	/// operations
	template<std::size_t Bits>
	struct [[DAW_PREF_NAME( i8 ), DAW_PREF_NAME( i16 ), DAW_PREF_NAME( i32 ),
	         DAW_PREF_NAME( i64 )]] signed_integer {
		using SignedInteger = sint_impl::signed_integer_type<Bits>::type;
		static_assert( std::is_integral_v<SignedInteger> and
		                 std::is_signed_v<SignedInteger>,
		               "Only signed integer types are supported" );
		using value_type = SignedInteger;
		using reference = value_type &;
		using const_reference = value_type const &;

		/// @brief Returns the maximum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL signed_integer max( ) noexcept {
			return signed_integer( daw::max_value<value_type> );
		}

		/// @brief Returns the minimum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL signed_integer min( ) noexcept {
			return signed_integer( daw::min_value<value_type> );
		}

		/// @brief Returns the lowest value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL signed_integer lowest( ) noexcept {
			return signed_integer( daw::lowest_value<value_type> );
		}

		struct private_t {
			value_type value{ };
		} m_private{ };

		explicit signed_integer( ) = default;

		// Construct from an integer type and ensure value_type is large enough
		template<typename I>
		requires daw::is_integral_v<I> //
		DAW_ATTRIB_INLINE constexpr explicit signed_integer( I v )
		  : m_private{ as<value_type>( v ) } {
			if constexpr( not sint_impl::convertible_signed_int<value_type, I> ) {
				if( DAW_UNLIKELY( not daw::in_range<value_type>( v ) ) ) {
					DAW_UNLIKELY_BRANCH
					on_signed_integer_overflow( );
				}
			}
		}

		static constexpr struct unchecked_t {
			explicit unchecked_t( ) = default;
		} unchecked{ };

		template<typename I>
		requires daw::is_integral_v<I> //
		DAW_ATTRIB_INLINE constexpr explicit signed_integer( I v, unchecked_t )
		  : m_private{ as<value_type>( v ) } {}

		/// @brief Creates an integer from a bytes object using little-endian byte
		/// order.
		/// @param ptr A byte array representing the integer in little-endian byte
		/// order
		/// @return The signed_integer represented by the bytes in little-endian
		/// order.
		[[nodiscard]] static constexpr signed_integer
		from_bytes_le( unsigned char const *ptr ) noexcept {
			return signed_integer{
			  daw::integers::sint_impl::from_bytes_le<value_type>(
			    ptr, std::make_index_sequence<sizeof( value_type )>{ } ) };
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
		template<typename I>
		requires( daw::is_integral_v<I> and daw::is_signed_v<I> ) //
		[[nodiscard]] static constexpr signed_integer
		conversion_checked( I other ) {
			if( DAW_UNLIKELY( sizeof( I ) > sizeof( value_type ) ) and
			    DAW_UNLIKELY( not daw::in_range<value_type>( other ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_signed_integer_overflow( );
			}
			return signed_integer( as<value_type>( other ) );
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
			return signed_integer( as<value_type>( other.value( ) ) );
		}

		/// @brief Converts another numeric type to a signed integer without bounds
		/// checking.
		/// @tparam I The type of the input parameter to be converted.
		/// @param other The input value to be converted to a signed integer.
		/// @return A `signed_integer` representing the converted value.
		template<typename I>
		requires daw::is_integral_v<I> //
		static constexpr signed_integer conversion_unchecked( I other ) {
			return signed_integer( as<value_type>( other ) );
		}

		/// @brief Construct a signed_integer from another that has a larger range
		/// @tparam I The type of the input parameter to be converted.
		/// @param other The input value to be constructed from
		template<std::size_t I>
		requires( I > Bits ) //
		DAW_ATTRIB_INLINE explicit constexpr signed_integer(
		  signed_integer<I> other )
		  : m_private{ as<value_type>( other.value( ) ) } {
#if DAW_DEFAULT_SIGNED_CHECKING == 0
			if( not std::in_range<value_type>( other.value( ) ) ) {
				on_signed_integer_overflow( );
			}
#endif
		}

		// @brief Construct a signed_integer from another signed_integer of smaller
		// range.  No checks are needed
		template<std::size_t I>
		requires( I / 8 <= sizeof( value_type ) ) //
		DAW_ATTRIB_INLINE constexpr signed_integer(
		  signed_integer<I> other ) noexcept
		  : m_private{ as<value_type>( other.value( ) ) } {}

		/// @brief Allow conversion to an arithmetic type
		template<typename Arithmetic>
		requires( daw::is_arithmetic_v<Arithmetic> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator Arithmetic( ) const noexcept {
			return as<Arithmetic>( value( ) );
		}

		/// @brief Allow conversion to signed_integer types that are larger in range
		template<std::size_t I>
		requires( sint_impl::convertible_signed_int<
		          sint_impl::signed_integer_type_t<I>, value_type> ) //
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
			return signed_integer( as<value_type>( -daw::as_unsigned( value( ) ) ),
			                       unchecked );
		}

		// @brief Negates the current signed_integer, wrapping when overflow happens
		// @return A new signed_integer instance with the negated value of the
		// current instance.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		negate_wrapped( ) const {
			return signed_integer{ as<value_type>( -daw::as_unsigned( value( ) ) ),
			                       unchecked };
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
			return signed_integer( as<value_type>( ~value( ) ) );
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

		struct wrapping_result {
			value_type value;
			bool overflowed;
		};

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		add_overflowing( signed_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  sint_impl::wrapping_add( value( ), rhs.value( ), result.value );
			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		sub_overflowing( signed_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  sint_impl::wrapping_sub( value( ), rhs.value( ), result.value );
			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		mul_overflowing( signed_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  sint_impl::wrapping_mul( value( ), rhs.value( ), result.value );
			return result;
		}

		struct wrapping_div_result {
			value_type value;
			SignedIntegerErrorType error;
		};

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_div_result
		div_overflowing( signed_integer const &rhs ) const {
			wrapping_div_result result;
			result.error =
			  sint_impl::wrapping_div( value( ), rhs.value( ), result.value );

			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_div_result
		rem_overflowing( signed_integer const &rhs ) const {
			wrapping_div_result result;
			result.error =
			  sint_impl::wrapping_rem( value( ), rhs.value( ), result.value );

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
			return signed_integer{
			  as<value_type>( sint_impl::as_next_wider_or_unsigned( value( ) ) +
			                  sint_impl::as_next_wider_or_unsigned( rhs.value( ) ) ),
			  unchecked };
		}

		/// @brief saturated addition of rhs and current value and return a new
		/// signed_integer.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		add_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_add( value( ), rhs.value( ) ) );
		}

		/// @brief Add rhs to this and return a ref this this.  Checked while in
		/// debug
		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
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
		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
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
			return signed_integer{
			  as<value_type>( sint_impl::as_next_wider_or_unsigned( value( ) ) -
			                  sint_impl::as_next_wider_or_unsigned( rhs.value( ) ) ),
			  unchecked };
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
		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
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
		mul_unchecked( signed_integer const &rhs ) const {
			return signed_integer{
			  as<value_type>( sint_impl::as_next_wider_or_unsigned( value( ) ) *
			                  sint_impl::as_next_wider_or_unsigned( rhs.value( ) ) ),
			  unchecked };
		}

		/// @brief Perform saturated multiplication with rhs and return a new
		/// signed_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		mul_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_mul( value( ), rhs.value( ) ) );
		}

		/**
		 * Divides the current signed_integer by another
		 * signed_integer and assigns the result to the current
		 * signed_integer instance.
		 *
		 * The division is performed using debug-checked division.
		 *
		 * @param rhs The signed_integer instance corresponding to
		 * the divisor.
		 * @return A reference to the modified signed_integer
		 * instance (result of division).
		 */
		DAW_ATTRIB_INLINE
		constexpr signed_integer &operator/=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_div( value( ), rhs.value( ) );
			return *this;
		}

		/**
		 * Divides the current signed_integer by a compatible integer
		 * and assigns the result to the current signed_integer instance.
		 *
		 * The division is performed using debug-checked division.
		 *
		 * @param rhs The signed_integer instance corresponding to
		 * the divisor.
		 * @return A reference to the modified signed_integer
		 * instance (result of division).
		 */
		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator/=( I rhs ) {
			return *this /= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_unchecked( signed_integer const &rhs ) const {
			return signed_integer{ as<value_type>( value( ) / rhs.value( ) ),
			                       unchecked };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_saturated( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::sat_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_wrapped( signed_integer const &rhs ) const {
			if( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) {
				return min( );
			}
			return signed_integer(
			  sint_impl::debug_checked_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_euclid( signed_integer const &rhs ) const {
			signed_integer q = *this / rhs;
			signed_integer const r = *this % rhs;
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					--q;
				} else {
					++q;
				}
			}
			return q;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_euclid_checked( signed_integer const &rhs ) const {
			if( rhs == signed_integer{ } ) {
				on_signed_integer_div_by_zero( );
				return *this;
			}
			bool overflow = false;
			signed_integer q;
			switch( sint_impl::wrapping_div(
			  m_private.value, rhs.m_private.value, q.m_private.value ) ) {
			case SignedIntegerErrorType::Overflow:
				overflow = true;
				break;
			case SignedIntegerErrorType::DivideByZero:
				on_signed_integer_div_by_zero( );
				return *this;
			case SignedIntegerErrorType::None:
			default:
				[[likely]] break;
			}

			signed_integer r;
			switch( sint_impl::wrapping_rem(
			  m_private.value, rhs.m_private.value, r.m_private.value ) ) {
			case SignedIntegerErrorType::Overflow:
				overflow = true;
				break;
			case SignedIntegerErrorType::DivideByZero:
				on_signed_integer_div_by_zero( );
				return *this;
			case SignedIntegerErrorType::None:
			default:
				[[likely]] break;
			}

			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					overflow |= sint_impl::wrapping_sub(
					  q.m_private.value, value_type{ 1 }, q.m_private.value );
				} else {
					overflow |= sint_impl::wrapping_add(
					  q.m_private.value, value_type{ 1 }, q.m_private.value );
				}
			}
			if( overflow ) {
				on_signed_integer_overflow( );
			}
			return q;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_euclid_saturated( signed_integer const &rhs ) const {
			signed_integer q = div_saturated( rhs );
			signed_integer const r = rem_saturated( rhs );
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					q = q.sub_saturated( signed_integer{ 1 } );
				} else {
					q = q.add_saturated( signed_integer{ 1 } );
				}
			}
			return q;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		div_euclid_wrapped( signed_integer const &rhs ) const {
			signed_integer q = div_wrapped( rhs );
			signed_integer const r = rem_wrapped( rhs );
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					q = q.sub_wrapped( signed_integer{ 1 } );
				} else {
					q = q.add_wrapped( signed_integer{ 1 } );
				}
			}
			return q;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator%=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_rem( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator%=( I rhs ) {
			return *this %= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_rem( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_unchecked( signed_integer const &rhs ) const {
			return signed_integer{ as<value_type>( value( ) % rhs.value( ) ),
			                       unchecked };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_saturated( signed_integer const &rhs ) const {
			if( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) {
				return signed_integer{ };
			}
			return signed_integer{
			  sint_impl::debug_checked_rem( value( ), rhs.value( ) ) };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_wrapped( signed_integer const &rhs ) const {
			if( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) {
				return signed_integer{ };
			}
			return signed_integer{
			  sint_impl::debug_checked_rem( value( ), rhs.value( ) ) };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_euclid( signed_integer const &rhs ) const {
			signed_integer r = *this % rhs;
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					r += rhs;
				} else {
					r -= rhs;
				}
			}
			return r;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_euclid_checked( signed_integer const &rhs ) const {
			signed_integer r = rem_checked( rhs );
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					r = r.add_checked( rhs );
				} else {
					r = r.sub_checked( rhs );
				}
			}
			return r;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_euclid_saturated( signed_integer const &rhs ) const {
			signed_integer r = rem_saturated( rhs );
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					r = r.add_saturated( rhs );
				} else {
					r = r.sub_saturated( rhs );
				}
			}
			return r;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rem_euclid_unchecked( signed_integer const &rhs ) const {
			signed_integer r = rem_unchecked( rhs );
			if( r.is_negative( ) ) {
				if( rhs.is_positive( ) ) {
					r = r.add_unchecked( rhs );
				} else {
					r = r.sub_unchecked( rhs );
				}
			}
			return r;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator<<=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_shl( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator<<=( I rhs ) {
			return *this <<= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_shl( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_unchecked( signed_integer const &rhs ) const {
			return signed_integer{ value( ) << rhs.value( ), unchecked };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_overflowing( signed_integer n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			}
			if( n == 0 ) {
				return *this;
			}
			n &= signed_integer{ daw::bit_count_v<value_type> - 1 };
			return signed_integer( value( ) << n.value( ), unchecked );
		}

		template<typename I>
		requires( daw::is_integral_v<I> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shl_overflowing( I n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			} else if( n == 0 ) {
				return *this;
			}
			n &= daw::bit_count_v<value_type> - 1;
			return signed_integer( value( ) << n, unchecked );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator>>=( signed_integer const &rhs ) {
			m_private.value = sint_impl::debug_checked_shr( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator>>=( I rhs ) {
			return *this >>= signed_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_checked( signed_integer const &rhs ) const {
			return signed_integer( sint_impl::checked_shr( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_unchecked( signed_integer const &rhs ) const {
			return signed_integer{ value( ) >> rhs.value( ), unchecked };
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_overflowing( signed_integer n ) const {
			if( n < 0 ) {
				on_signed_integer_overflow( );
				return *this;
			}
			if( n == 0 ) {
				return *this;
			}
			n &= signed_integer{ daw::bit_count_v<value_type> - 1 };
			return signed_integer( value( ) >> n.value( ) );
		}

		template<typename I>
		requires( daw::is_integral_v<I> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		shr_overflowing( I n ) const {
			if( n < I{ } ) {
				on_signed_integer_overflow( );
				return *this;
			}
			if( n == I{ } ) {
				return *this;
			}
			n &= daw::bit_count_v<value_type> - 1;
			return signed_integer( value( ) >> n );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rotate_left( std::size_t n ) const {
			return signed_integer(
			  as<value_type>( std::rotl( daw::as_unsigned( value( ) ),
			                             n % daw::bit_count_v<value_type> ) ),
			  unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		rotate_right( std::size_t n ) const {
			return signed_integer(
			  as<value_type>( std::rotr( daw::as_unsigned( value( ) ),
			                             n % daw::bit_count_v<value_type> ) ),
			  unchecked );
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator|=( signed_integer const &rhs ) noexcept {
			m_private.value |= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator|=( I rhs ) {
			m_private.value |= as<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator&=( signed_integer const &rhs ) noexcept {
			m_private.value &= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator&=( I rhs ) {
			m_private.value &= as<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr signed_integer &
		operator^=( signed_integer const &rhs ) noexcept {
			m_private.value ^= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( sint_impl::convertible_signed_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr signed_integer &operator^=( I rhs ) {
			m_private.value ^= as<value_type>( rhs );
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator bool( ) const noexcept {
			return as<bool>( value( ) );
		}

		// Logical without short circuit
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		And( signed_integer const &rhs ) const noexcept {
			return as<bool>( *this ) and as<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		Or( signed_integer const &rhs ) const noexcept {
			return as<bool>( *this ) or as<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		reverse_bits( ) const noexcept {
			return signed_integer( daw::cxmath::to_signed(
			  daw::integers::reverse_bits( daw::cxmath::to_unsigned( value( ) ) ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_leading_zeros( ) const noexcept {
			return as<std::uint32_t>(
			  std::countl_zero( daw::cxmath::to_unsigned( value( ) ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_trailing_zeros( ) const noexcept {
			return as<std::uint32_t>(
			  std::countr_zero( daw::cxmath::to_unsigned( value( ) ) ) );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Checked in debug mode
		[[nodiscard]] constexpr signed_integer pow( unsigned pow ) const {
			return sint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs * rhs;
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Overflow is never checked
		[[nodiscard]] constexpr signed_integer pow_unchecked( unsigned pow ) const {
			return sint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_unchecked( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Overflow is checked and handler called if encountered
		[[nodiscard]] constexpr signed_integer pow_checked( unsigned pow ) const {
			return sint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_checked( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Value is wrapped when overflow is encountered
		[[nodiscard]] constexpr signed_integer pow_wrapped( unsigned pow ) const {
			return sint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_wrapped( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Value is saturated if overflow is encountered
		[[nodiscard]] constexpr signed_integer pow_saturated( unsigned pow ) const {
			return sint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_saturated( rhs );
			} );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer abs( ) const {
			if( is_negative( ) ) {
				return *this * signed_integer{ -1 };
			}
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		abs_checked( ) const {
			if( is_negative( ) ) {
				return mul_checked( signed_integer{ -1 } );
			}
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		abs_wrapped( ) const {
			if( is_negative( ) ) {
				return mul_wrapped( signed_integer{ -1 } );
			}
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		abs_saturated( ) const {
			if( is_negative( ) ) {
				return mul_saturated( signed_integer{ -1 } );
			}
			return *this;
		}

		[[nodiscard]] constexpr bool is_positive( ) const {
			return m_private.value > value_type{ };
		}

		[[nodiscard]] constexpr bool is_negative( ) const {
			return m_private.value < value_type{ };
		}

		/// @brief Convert to an unsigned_integer of the same width, like a
		/// static_cast.  The two's complement bits are kept, so negative values
		/// wrap.  Requires daw/integers/daw_unsigned.h, or include
		/// daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer<Bits>
		as_unsigned( ) const noexcept {
			return unsigned_integer<Bits>( daw::as_unsigned( value( ) ),
			                               unsigned_integer<Bits>::unchecked );
		}

		/// @brief Convert to an unsigned_integer of the same width.  Negative
		/// values call the overflow handler and the two's complement bits are
		/// returned.  Requires daw/integers/daw_unsigned.h, or include
		/// daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer<Bits>
		as_exact_unsigned( ) const {
			if( DAW_UNLIKELY( is_negative( ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return as_unsigned( );
		}

		[[nodiscard]] constexpr signed_integer signum( ) const {
			return signed_integer{ as<int>( m_private.value > value_type{ } ) -
			                       as<int>( m_private.value < value_type{ } ) };
		}

		/// @brief Returns the number of ones in the two's complement
		/// representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_ones( ) const noexcept {
			return static_cast<std::uint32_t>(
			  std::popcount( daw::as_unsigned( value( ) ) ) );
		}

		/// @brief Returns the number of zeros in the two's complement
		/// representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_zeros( ) const noexcept {
			return static_cast<std::uint32_t>(
			  std::popcount( static_cast<std::make_unsigned_t<value_type>>(
			    ~daw::as_unsigned( value( ) ) ) ) );
		}

		/// @brief Returns the number of leading ones in the two's complement
		/// representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_leading_ones( ) const noexcept {
			return static_cast<std::uint32_t>(
			  std::countl_one( daw::as_unsigned( value( ) ) ) );
		}

		/// @brief Returns the number of trailing ones in the two's complement
		/// representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_trailing_ones( ) const noexcept {
			return static_cast<std::uint32_t>(
			  std::countr_one( daw::as_unsigned( value( ) ) ) );
		}

		/// @brief Reverses the byte order
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer
		swap_bytes( ) const noexcept {
			return signed_integer(
			  int_impl::swap_bytes( daw::as_unsigned( value( ) ) ), unchecked );
		}

		/// @brief Returns floor(log2(*this)).  Values that are not positive call
		/// the overflow handler and 0 is returned
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t ilog2( ) const {
			if( DAW_UNLIKELY( not is_positive( ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
				return 0;
			}
			return static_cast<std::uint32_t>(
			  std::bit_width( daw::as_unsigned( value( ) ) ) - 1 );
		}

		/// @brief Returns the absolute value as an unsigned_integer.  This cannot
		/// overflow, min( ).unsigned_abs( ) is max( ) + 1.  Requires
		/// daw/integers/daw_unsigned.h, or include daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer<Bits>
		unsigned_abs( ) const noexcept {
			using unsigned_t = std::make_unsigned_t<value_type>;
			auto const u = static_cast<unsigned_t>( value( ) );
			return unsigned_integer<Bits>(
			  static_cast<unsigned_t>( is_negative( ) ? 0U - u : u ),
			  unsigned_integer<Bits>::unchecked );
		}

		/// @brief Returns |*this - rhs| as an unsigned_integer.  This cannot
		/// overflow.  Requires daw/integers/daw_unsigned.h, or include
		/// daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer<Bits>
		abs_diff( signed_integer const &rhs ) const noexcept {
			using unsigned_t = std::make_unsigned_t<value_type>;
			auto const l = static_cast<unsigned_t>( value( ) );
			auto const r = static_cast<unsigned_t>( rhs.value( ) );
			return unsigned_integer<Bits>(
			  static_cast<unsigned_t>( value( ) < rhs.value( ) ? r - l : l - r ),
			  unsigned_integer<Bits>::unchecked );
		}

		// try_ operations return std::nullopt instead of reporting an error.  The
		// error handlers are never called

		/// @brief Converts other if it is in range
		template<typename I>
		requires daw::is_integral_v<I> //
		[[nodiscard]] static constexpr std::optional<signed_integer>
		try_from( I other ) noexcept {
			if( not daw::in_range<value_type>( other ) ) {
				return std::nullopt;
			}
			return signed_integer( other, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_add( signed_integer const &rhs ) const noexcept {
			auto const r = add_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return signed_integer( r.value, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_sub( signed_integer const &rhs ) const noexcept {
			auto const r = sub_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return signed_integer( r.value, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_mul( signed_integer const &rhs ) const noexcept {
			auto const r = mul_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return signed_integer( r.value, unchecked );
		}

		/// @brief Division, std::nullopt when rhs is 0 or for min( ) / -1
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_div( signed_integer const &rhs ) const noexcept {
			auto const r = div_overflowing( rhs );
			if( r.error != SignedIntegerErrorType::None ) {
				return std::nullopt;
			}
			return signed_integer( r.value, unchecked );
		}

		/// @brief Remainder, std::nullopt when rhs is 0 or for min( ) % -1
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_rem( signed_integer const &rhs ) const noexcept {
			auto const r = rem_overflowing( rhs );
			if( r.error != SignedIntegerErrorType::None ) {
				return std::nullopt;
			}
			return signed_integer( r.value, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_div_euclid( signed_integer const &rhs ) const noexcept {
			if( rhs.value( ) == 0 or
			    ( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) ) {
				return std::nullopt;
			}
			return div_euclid( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_rem_euclid( signed_integer const &rhs ) const noexcept {
			if( rhs.value( ) == 0 or
			    ( value( ) == min( ) and rhs.value( ) == value_type{ -1 } ) ) {
				return std::nullopt;
			}
			return rem_euclid( rhs );
		}

		/// @brief Negation, std::nullopt for min( )
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_negate( ) const noexcept {
			if( value( ) == min( ) ) {
				return std::nullopt;
			}
			return signed_integer( -value( ), unchecked );
		}

		/// @brief Absolute value, std::nullopt for min( )
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_abs( ) const noexcept {
			if( is_negative( ) ) {
				return try_negate( );
			}
			return *this;
		}

		/// @brief Shift left, std::nullopt when rhs is negative or not less than
		/// the bit width.  Bits shifted out are discarded
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_shl( signed_integer const &rhs ) const noexcept {
			if( rhs.value( ) < 0 or std::cmp_greater_equal(
			                          rhs.value( ), daw::bit_count_v<value_type> ) ) {
				return std::nullopt;
			}
			return signed_integer( value( ) << rhs.value( ), unchecked );
		}

		/// @brief Arithmetic shift right, std::nullopt when rhs is negative or
		/// not less than the bit width
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<signed_integer>
		try_shr( signed_integer const &rhs ) const noexcept {
			if( rhs.value( ) < 0 or std::cmp_greater_equal(
			                          rhs.value( ), daw::bit_count_v<value_type> ) ) {
				return std::nullopt;
			}
			return signed_integer( value( ) >> rhs.value( ), unchecked );
		}

		[[nodiscard]] constexpr std::optional<signed_integer>
		try_pow( unsigned exp ) const noexcept {
			bool overflowed = false;
			auto const result = sint_impl::pow_impl(
			  *this,
			  exp,
			  [&]( signed_integer const &lhs, signed_integer const &rhs ) {
				  auto const r = lhs.mul_overflowing( rhs );
				  overflowed |= r.overflowed;
				  return signed_integer( r.value, unchecked );
			  } );
			if( overflowed ) {
				return std::nullopt;
			}
			return result;
		}

		/// @brief floor(log2(*this)), std::nullopt when not positive
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<std::uint32_t>
		try_ilog2( ) const noexcept {
			if( not is_positive( ) ) {
				return std::nullopt;
			}
			return ilog2( );
		}

		/// @brief Convert to an unsigned_integer of the same width, std::nullopt
		/// when negative.  Requires daw/integers/daw_unsigned.h, or include
		/// daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<
		  unsigned_integer<Bits>>
		try_as_unsigned( ) const noexcept {
			if( is_negative( ) ) {
				return std::nullopt;
			}
			return as_unsigned( );
		}
	};

	template<typename I>
	requires(
	  daw::traits::is_one_of_v<daw::make_signed_t<I>, std::int8_t, std::int16_t,
	                           std::int32_t, std::int64_t> ) //
	signed_integer( I ) -> signed_integer<sizeof( I ) * 8>;

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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result += result_t( rhs );
		return result;
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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
		result -= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;

		auto result = result_t( lhs.value( ) );
		result -= result_t( rhs );
		return result;
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result *= result_t( rhs );
		return result;
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result /= result_t( rhs );
		return result;
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result %= result_t( rhs );
		return result;
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) <<= result_t( rhs );
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) >>= result_t( rhs );
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) |= result_t( rhs );
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) &= result_t( rhs );
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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

	template<std::size_t Lhs, sint_impl::SignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( signed_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = sint_impl::signed_integer_type_t<Lhs>;
		using rhs_t = Rhs;
		using result_t = sint_impl::int_result_t<lhs_t, rhs_t>;
		return result_t( lhs.m_private.value ) ^= result_t( rhs );
	}

	template<sint_impl::SignedIntegral Lhs, std::size_t Rhs>
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
		return std::cmp_equal( lhs.value( ), rhs.value( ) );
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
		return std::cmp_not_equal( lhs.value( ), rhs.value( ) );
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
		return std::cmp_less( lhs.value( ), rhs.value( ) );
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
		return std::cmp_less_equal( lhs.value( ), rhs.value( ) );
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
		return std::cmp_greater( lhs.value( ), rhs.value( ) );
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
		return std::cmp_greater_equal( lhs.value( ), rhs.value( ) );
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
} // namespace daw::integers::inline DAW_INTEGER_VER

namespace daw::integers::inline DAW_INTEGER_VER::literals {
	[[nodiscard]] DAW_CONSTEVAL signed_integer<8>
	operator""_i8( unsigned long long v ) {
		using int_t = std::int8_t;
		if( not std::in_range<int_t>( v ) ) {
			on_signed_integer_overflow( );
		}
		return signed_integer<8>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL signed_integer<16>
	operator""_i16( unsigned long long v ) {
		using int_t = std::int16_t;
		if( not std::in_range<int_t>( v ) ) {
			on_signed_integer_overflow( );
		}
		return signed_integer<16>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL signed_integer<32>
	operator""_i32( unsigned long long v ) {
		using int_t = std::int32_t;
		if( not std::in_range<int_t>( v ) ) {
			on_signed_integer_overflow( );
		}
		return signed_integer<32>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL signed_integer<64>
	operator""_i64( unsigned long long v ) {
		using int_t = std::int64_t;
		if( not std::in_range<int_t>( v ) ) {
			on_signed_integer_overflow( );
		}
		return signed_integer<64>( as<int_t>( v ) );
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::literals

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
		using type = std::int8_t;
	};

	template<>
	struct make_signed<daw::integers::i16> {
		using type = std::int16_t;
	};

	template<>
	struct make_signed<daw::integers::i32> {
		using type = std::int32_t;
	};

	template<>
	struct make_signed<daw::integers::i64> {
		using type = std::int64_t;
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
		static constexpr std::float_denorm_style has_denorm =
		  std::float_denorm_style::denorm_absent;
		static constexpr bool has_denorm_loss = false;
		static constexpr std::float_round_style round_style =
		  std::round_toward_zero;
		static constexpr bool is_iec559 = false;
		static constexpr bool is_bounded = true;
		static constexpr bool is_modulo = numeric_limits<
		  typename daw::integers::signed_integer<Bits>::value_type>::is_modulo;
		static constexpr int digits = Bits - 1;

		static constexpr int digits10 = digits * 3 / 10;
		static constexpr int max_digits10 = 0;
		static constexpr int radix = 2;
		static constexpr int min_exponent = 0;
		static constexpr int min_exponent10 = 0;
		static constexpr int max_exponent = 0;
		static constexpr int max_exponent10 = 0;

		static constexpr bool traps = true;
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
			return daw::integers::signed_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		round_error( ) noexcept {
			return daw::integers::signed_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		infinity( ) noexcept {
			return daw::integers::signed_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		quiet_NaN( ) noexcept {
			return daw::integers::signed_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		signaling_NaN( ) noexcept {
			return daw::integers::signed_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::signed_integer<Bits>
		denorm_min( ) noexcept {
			return daw::integers::signed_integer<Bits>{ };
		}
	};

	/// std::hash support.  Hashes the same as the underlying value_type
	template<std::size_t Bits>
	struct hash<daw::integers::signed_integer<Bits>> {
		[[nodiscard]] std::size_t
		operator( )( daw::integers::signed_integer<Bits> v ) const noexcept {
			return std::hash<
			  typename daw::integers::signed_integer<Bits>::value_type>{ }(
			  v.value( ) );
		}
	};
} // namespace std
