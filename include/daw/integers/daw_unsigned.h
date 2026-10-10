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
#include "daw/integers/impl/daw_integer_error_handling.h"
#include "daw/integers/impl/daw_integer_fwd.h"
#include "daw/integers/impl/daw_unsigned_impl.h"

#include <daw/daw_arith_traits.h>
#include <daw/daw_as.h>
#include <daw/daw_attributes.h>
#include <daw/daw_bit_count.h>
#include <daw/daw_consteval.h>
#include <daw/daw_cpp_feature_check.h>
#include <daw/daw_cxmath.h>
#include <daw/daw_int_cmp.h>
#include <daw/daw_integer_reverse.h>
#include <daw/daw_likely.h>
#include <daw/traits/daw_traits_is_one_of.h>

#include <algorithm>
#include <bit>
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>

namespace daw::integers::inline DAW_INTEGER_VER::uint_impl {
	template<std::size_t /*Bits*/>
	struct unsigned_integer_type;

	template<>
	struct unsigned_integer_type<8> {
		using type = std::uint8_t;
	};

	template<>
	struct unsigned_integer_type<16> {
		using type = std::uint16_t;
	};

	template<>
	struct unsigned_integer_type<32> {
		using type = std::uint32_t;
	};

	template<>
	struct unsigned_integer_type<64> {
		using type = std::uint64_t;
	};

	template<std::size_t Bits>
	using unsigned_integer_type_t = typename unsigned_integer_type<Bits>::type;

	template<typename T>
	inline constexpr bool is_unsigned_integral_v =
	  daw::is_integral_v<T> and daw::is_unsigned_v<T> and
	  not std::is_same_v<T, bool>;

	template<typename T>
	concept UnsignedIntegral = is_unsigned_integral_v<T>;

	template<UnsignedIntegral Lhs, UnsignedIntegral Rhs>
	using uint_result_t =
	  typename std::conditional<( sizeof( Lhs ) >= sizeof( Rhs ) ),
	                            unsigned_integer<sizeof( Lhs ) * 8>,
	                            unsigned_integer<sizeof( Rhs ) * 8>>::type;

	template<std::size_t Bits>
	DAW_ATTRIB_FLATINLINE constexpr unsigned_integer<Bits>
	pow_impl( unsigned_integer<Bits> base, unsigned exp, auto &&multiplier ) {
		auto result = unsigned_integer<Bits>{ 1U };

		while( exp != 0 ) {
			if( ( exp & 1 ) == 1 ) {
				result = multiplier( result, base );
			}
			exp /= 2U;
			if( exp != 0 ) {
				base = multiplier( base, base );
			}
		}
		return result;
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::uint_impl

namespace daw::integers::inline DAW_INTEGER_VER {
	using u8 = unsigned_integer<8>;
	using u16 = unsigned_integer<16>;
	using u32 = unsigned_integer<32>;
	using u64 = unsigned_integer<64>;

	/// @brief Unsigned Integer type with overflow checked/wrapping/saturated
	/// operations
	template<std::size_t Bits>
	struct [[DAW_PREF_NAME( u8 ), DAW_PREF_NAME( u16 ), DAW_PREF_NAME( u32 ),
	         DAW_PREF_NAME( u64 )]] unsigned_integer {
		using UnsignedInteger =
		  typename uint_impl::unsigned_integer_type<Bits>::type;
		static_assert( daw::is_integral_v<UnsignedInteger> and
		                 daw::is_unsigned_v<UnsignedInteger>,
		               "Only unsigned integer types are supported" );
		using value_type = UnsignedInteger;
		using reference = value_type &;
		using const_reference = value_type const &;

		/// @brief Returns the maximum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL unsigned_integer max( ) noexcept {
			return unsigned_integer( daw::numeric_limits<value_type>::max( ) );
		}

		/// @brief Returns the minimum value of the underlying integer type
		[[nodiscard]] static DAW_CONSTEVAL unsigned_integer min( ) noexcept {
			return unsigned_integer( daw::numeric_limits<value_type>::min( ) );
		}

		struct private_t {
			value_type value{ };
		} m_private{ };

		explicit unsigned_integer( ) = default;

		// Construct from an integer type and ensure value is in range.  Negative
		// values are out of range
		template<typename I>
		requires daw::is_integral_v<I> //
		DAW_ATTRIB_INLINE constexpr explicit unsigned_integer( I v )
		  : m_private{ as<value_type>( v ) } {
			if constexpr( not uint_impl::convertible_unsigned_int<value_type, I> ) {
				if( DAW_UNLIKELY( not daw::in_range<value_type>( v ) ) ) {
					DAW_UNLIKELY_BRANCH
					on_integer_overflow( );
				}
			}
		}

		static constexpr struct unchecked_t {
			explicit unchecked_t( ) = default;
		} unchecked{ };

		template<typename I>
		requires daw::is_integral_v<I> //
		DAW_ATTRIB_INLINE constexpr explicit unsigned_integer( I v, unchecked_t )
		  : m_private{ as<value_type>( v ) } {}

		/// @brief Creates an integer from a bytes object using little-endian byte
		/// order.
		/// @param ptr A byte array representing the integer in little-endian byte
		/// order
		/// @return The unsigned_integer represented by the bytes in little-endian
		/// order.
		[[nodiscard]] static constexpr unsigned_integer
		from_bytes_le( unsigned char const *ptr ) noexcept {
			return unsigned_integer{ uint_impl::from_bytes_le<value_type>(
			  ptr, std::make_index_sequence<sizeof( value_type )>{ } ) };
		}

		/// @brief Creates an integer from a bytes object using big-endian byte
		/// order.
		/// @param ptr A byte array representing the integer in big-endian byte
		/// order
		/// @return The unsigned_integer represented by the bytes in big-endian
		/// order.
		[[nodiscard]] static constexpr unsigned_integer
		from_bytes_be( unsigned char const *ptr ) noexcept {
			return unsigned_integer{ uint_impl::from_bytes_be<value_type>(
			  ptr, std::make_index_sequence<sizeof( value_type )>{ } ) };
		}

		/// @brief `conversion_checked` provides safe type conversion operations
		/// with overflow and underflow checks.
		/// @param other Integer to convert to unsigned_integer
		/// @returns A unsigned_integer with value of other
		template<typename I>
		requires( daw::is_integral_v<I> ) //
		[[nodiscard]] static constexpr unsigned_integer
		conversion_checked( I other ) {
			if( DAW_UNLIKELY( not daw::in_range<value_type>( other ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return unsigned_integer( as<value_type>( other ) );
		}

		/// @brief `conversion_checked` provides safe type conversion operations
		/// with overflow checks.
		/// @param other unsigned_integer of different size to convert
		/// @returns A unsigned_integer with value of other
		template<std::size_t I>
		[[nodiscard]] static constexpr unsigned_integer
		conversion_checked( unsigned_integer<I> other ) {
			return unsigned_integer( conversion_checked( other.value( ) ) );
		}

		/// @brief Performs an unchecked conversion between unsigned integer types.
		/// @tparam I The bit width of the unsigned integer to be converted.
		/// @param other The unsigned integer value to convert.
		/// @return The converted unsigned integer value.
		template<std::size_t I>
		[[nodiscard]] static constexpr unsigned_integer
		conversion_unchecked( unsigned_integer<I> other ) {
			return unsigned_integer( as<value_type>( other.value( ) ) );
		}

		/// @brief Converts another numeric type to an unsigned integer without
		/// bounds checking.
		/// @tparam I The type of the input parameter to be converted.
		/// @param other The input value to be converted to an unsigned integer.
		/// @return A `unsigned_integer` representing the converted value.
		template<typename I>
		requires daw::is_integral_v<I> //
		static constexpr unsigned_integer conversion_unchecked( I other ) {
			return unsigned_integer( as<value_type>( other ), unchecked );
		}

		/// @brief Construct an unsigned_integer from another that has a larger
		/// range
		/// @tparam I The bit width of the input parameter to be converted.
		/// @param other The input value to be constructed from
		template<std::size_t I>
		requires( I > Bits ) //
		DAW_ATTRIB_INLINE explicit constexpr unsigned_integer(
		  unsigned_integer<I> other )
		  : m_private{ as<value_type>( other.value( ) ) } {
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
			if( not daw::in_range<value_type>( other.value( ) ) ) {
				on_integer_overflow( );
			}
#endif
		}

		// @brief Construct an unsigned_integer from another unsigned_integer of
		// smaller range.  No checks are needed
		template<std::size_t I>
		requires( I / 8 <= sizeof( value_type ) ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer(
		  unsigned_integer<I> other ) noexcept
		  : m_private{ as<value_type>( other.value( ) ) } {}

		/// @brief Allow conversion to an arithmetic type
		template<typename Arithmetic>
		requires( daw::is_arithmetic_v<Arithmetic> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator Arithmetic( ) const noexcept {
			return as<Arithmetic>( value( ) );
		}

		/// @brief Allow conversion to unsigned_integer types that are larger in
		/// range
		template<std::size_t I>
		requires( uint_impl::convertible_unsigned_int<
		          uint_impl::unsigned_integer_type_t<I>, value_type> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator unsigned_integer<I>( ) const noexcept {
			return unsigned_integer<I>( value( ) );
		}

		/// @brief Access to underlying value
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr value_type
		value( ) const noexcept {
			return m_private.value;
		}

		/// @brief Negate the value modulo 2^Bits, the same as the builtin unary
		/// minus on unsigned types.  Never checked
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		operator-( ) const noexcept {
			return negate_unchecked( );
		}

		// @brief Negates the current unsigned_integer modulo 2^Bits, the same as
		// the builtin unary minus on unsigned types.
		// @return A new unsigned_integer instance with the negated value of the
		// current instance.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		negate_unchecked( ) const noexcept {
			return unsigned_integer( as<value_type>( value_type{ } - value( ) ),
			                         unchecked );
		}

		/// @brief Computes the bitwise not and returns as an unsigned integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		operator~( ) const {
			return unsigned_integer( as<value_type>( ~value( ) ), unchecked );
		}

		/// @brief Add unsigned_integer rhs to self.  Checked in debug mode
		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator+=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_add( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief increment current value.  Checked in debug mode
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator++( ) {
			m_private.value =
			  uint_impl::debug_checked_add( value( ), value_type{ 1 } );
			return *this;
		}

		/// @brief increment current value and return previous value.  Checked in
		/// debug mode
		DAW_ATTRIB_INLINE constexpr unsigned_integer operator++( int ) {
			auto result = *this;
			operator++( );
			return result;
		}

		struct wrapping_result {
			value_type value;
			bool overflowed;
		};

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		add_overflowing( unsigned_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  uint_impl::wrapping_add( value( ), rhs.value( ), result.value );
			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		sub_overflowing( unsigned_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  uint_impl::wrapping_sub( value( ), rhs.value( ), result.value );
			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_result
		mul_overflowing( unsigned_integer const &rhs ) const {
			wrapping_result result;
			result.overflowed =
			  uint_impl::wrapping_mul( value( ), rhs.value( ), result.value );
			return result;
		}

		struct wrapping_div_result {
			value_type value;
			IntegerErrorType error;
		};

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_div_result
		div_overflowing( unsigned_integer const &rhs ) const {
			wrapping_div_result result;
			result.error =
			  uint_impl::wrapping_div( value( ), rhs.value( ), result.value );
			return result;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr wrapping_div_result
		rem_overflowing( unsigned_integer const &rhs ) const {
			wrapping_div_result result;
			result.error =
			  uint_impl::wrapping_rem( value( ), rhs.value( ), result.value );
			return result;
		}

		/// @brief add rhs to current value and return a new unsigned_integer.
		/// Addition is checked and calls error handler on overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		add_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_add( value( ), rhs.value( ) ) );
		}

		/// @brief add rhs to current value and return a new unsigned_integer.
		/// Addition is wrapped on overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		add_wrapped( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::wrapped_add( value( ), rhs.value( ) ) );
		}

		/// @brief add rhs to current value and return a new unsigned_integer. No
		/// overflow checking is performed
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		add_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ uint_impl::wrapped_add( value( ), rhs.value( ) ),
			                         unchecked };
		}

		/// @brief saturated addition of rhs and current value and return a new
		/// unsigned_integer.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		add_saturated( unsigned_integer const &rhs ) const {
			return unsigned_integer( uint_impl::sat_add( value( ), rhs.value( ) ) );
		}

		/// @brief Add rhs to this and return a ref this this.  Checked while in
		/// debug
		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator+=( I rhs ) {
			return *this += unsigned_integer( rhs );
		}

		/// @brief Subtract rhs to this and return a ref this this.  Checked while
		/// in debug
		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator-=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_sub( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief Subtract rhs to this and return a ref this this.  Checked while
		/// in debug
		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator-=( I rhs ) {
			return *this -= unsigned_integer( rhs );
		}

		/// @brief Subtract rhs from this and return a new unsigned_integer.
		/// Checked for overflow.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		sub_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_sub( value( ), rhs.value( ) ) );
		}

		/// @brief Subtract rhs from this and return a new unsigned_integer.  On
		/// overflow value is wrapped
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		sub_wrapped( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::wrapped_sub( value( ), rhs.value( ) ) );
		}

		/// @brief Subtract rhs from this and return a new unsigned_integer.  No
		/// overflow checking.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		sub_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ uint_impl::wrapped_sub( value( ), rhs.value( ) ),
			                         unchecked };
		}

		/// @brief Subtract rhs from this and return a new unsigned_integer.  On
		/// overflow value is saturated.
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		sub_saturated( unsigned_integer const &rhs ) const {
			return unsigned_integer( uint_impl::sat_sub( value( ), rhs.value( ) ) );
		}

		/// @brief prefix decrement return a ref to self. Checked in debug
		/// modes.
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator--( ) {
			m_private.value =
			  uint_impl::debug_checked_sub( value( ), value_type{ 1 } );
			return *this;
		}

		/// @brief postfix decrement return the previous value. Checked in debug
		/// modes.
		DAW_ATTRIB_INLINE constexpr unsigned_integer operator--( int ) {
			auto result = *this;
			operator--( );
			return result;
		}

		/// @brief Multiple self with rhs and store the product in this.  Checked in
		/// debug modes
		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator*=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_mul( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief Multiple self with rhs and store the product in this.  Checked in
		/// debug modes
		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator*=( I rhs ) {
			return *this *= unsigned_integer( rhs );
		}

		/// @brief Perform checked multiplication with rhs and return a new
		/// unsigned_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		mul_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_mul( value( ), rhs.value( ) ) );
		}

		/// @brief Perform wrapped multiplication with rhs and return a new
		/// unsigned_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		mul_wrapped( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::wrapped_mul( value( ), rhs.value( ) ) );
		}

		/// @brief Perform unchecked multiplication with rhs and return a new
		/// unsigned_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		mul_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ uint_impl::wrapped_mul( value( ), rhs.value( ) ),
			                         unchecked };
		}

		/// @brief Perform saturated multiplication with rhs and return a new
		/// unsigned_integer
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		mul_saturated( unsigned_integer const &rhs ) const {
			return unsigned_integer( uint_impl::sat_mul( value( ), rhs.value( ) ) );
		}

		/// @brief Divide self by rhs and store the quotient in this.  Division by
		/// zero is checked in debug modes
		DAW_ATTRIB_INLINE
		constexpr unsigned_integer &operator/=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_div( value( ), rhs.value( ) );
			return *this;
		}

		/// @brief Divide self by rhs and store the quotient in this.  Division by
		/// zero is checked in debug modes
		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator/=( I rhs ) {
			return *this /= unsigned_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_div( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ as<value_type>( value( ) / rhs.value( ) ),
			                         unchecked };
		}

		/// @brief Unsigned division cannot overflow, this is the same as operator/
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_saturated( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::debug_checked_div( value( ), rhs.value( ) ) );
		}

		/// @brief Unsigned division cannot overflow, this is the same as operator/
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_wrapped( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::debug_checked_div( value( ), rhs.value( ) ) );
		}

		/// @brief Euclidean division.  For unsigned values this is the same as
		/// operator/
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_euclid( unsigned_integer const &rhs ) const {
			return *this / rhs;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_euclid_checked( unsigned_integer const &rhs ) const {
			return div_checked( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_euclid_saturated( unsigned_integer const &rhs ) const {
			return div_saturated( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		div_euclid_wrapped( unsigned_integer const &rhs ) const {
			return div_wrapped( rhs );
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator%=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_rem( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator%=( I rhs ) {
			return *this %= unsigned_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_rem( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ as<value_type>( value( ) % rhs.value( ) ),
			                         unchecked };
		}

		/// @brief Unsigned remainder cannot overflow, this is the same as operator%
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_saturated( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::debug_checked_rem( value( ), rhs.value( ) ) );
		}

		/// @brief Unsigned remainder cannot overflow, this is the same as operator%
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_wrapped( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::debug_checked_rem( value( ), rhs.value( ) ) );
		}

		/// @brief Euclidean remainder.  For unsigned values this is the same as
		/// operator%
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_euclid( unsigned_integer const &rhs ) const {
			return *this % rhs;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_euclid_checked( unsigned_integer const &rhs ) const {
			return rem_checked( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_euclid_saturated( unsigned_integer const &rhs ) const {
			return rem_saturated( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rem_euclid_unchecked( unsigned_integer const &rhs ) const {
			return rem_unchecked( rhs );
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator<<=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_shl( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator<<=( I rhs ) {
			return *this <<= unsigned_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shl_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_shl( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shl_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ uint_impl::promote( value( ) ) << rhs.value( ),
			                         unchecked };
		}

		/// @brief Shift left by n modulo the bit width
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shl_overflowing( unsigned_integer n ) const {
			return shl_overflowing( n.value( ) );
		}

		/// @brief Shift left by n modulo the bit width.  A negative n calls the
		/// overflow handler and returns the current value
		template<typename I>
		requires( daw::is_integral_v<I> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shl_overflowing( I n ) const {
			if( n < I{ } ) {
				on_integer_overflow( );
				return *this;
			}
			auto const count =
			  static_cast<std::size_t>( n ) % daw::bit_count_v<value_type>;
			return unsigned_integer( uint_impl::promote( value( ) ) << count,
			                         unchecked );
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator>>=( unsigned_integer const &rhs ) {
			m_private.value = uint_impl::debug_checked_shr( value( ), rhs.value( ) );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator>>=( I rhs ) {
			return *this >>= unsigned_integer( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shr_checked( unsigned_integer const &rhs ) const {
			return unsigned_integer(
			  uint_impl::checked_shr( value( ), rhs.value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shr_unchecked( unsigned_integer const &rhs ) const {
			return unsigned_integer{ uint_impl::promote( value( ) ) >> rhs.value( ),
			                         unchecked };
		}

		/// @brief Shift right by n modulo the bit width
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shr_overflowing( unsigned_integer n ) const {
			return shr_overflowing( n.value( ) );
		}

		/// @brief Shift right by n modulo the bit width.  A negative n calls the
		/// overflow handler and returns the current value
		template<typename I>
		requires( daw::is_integral_v<I> ) //
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		shr_overflowing( I n ) const {
			if( n < I{ } ) {
				on_integer_overflow( );
				return *this;
			}
			auto const count =
			  static_cast<std::size_t>( n ) % daw::bit_count_v<value_type>;
			return unsigned_integer( value( ) >> count, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rotate_left( std::size_t n ) const {
			return unsigned_integer(
			  std::rotl( value( ), as<int>( n % daw::bit_count_v<value_type> ) ),
			  unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		rotate_right( std::size_t n ) const {
			return unsigned_integer(
			  std::rotr( value( ), as<int>( n % daw::bit_count_v<value_type> ) ),
			  unchecked );
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator|=( unsigned_integer const &rhs ) noexcept {
			m_private.value |= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator|=( I rhs ) {
			m_private.value |= as<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator&=( unsigned_integer const &rhs ) noexcept {
			m_private.value &= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator&=( I rhs ) {
			m_private.value &= as<value_type>( rhs );
			return *this;
		}

		DAW_ATTRIB_INLINE constexpr unsigned_integer &
		operator^=( unsigned_integer const &rhs ) noexcept {
			m_private.value ^= rhs.value( );
			return *this;
		}

		template<typename I>
		requires( uint_impl::convertible_unsigned_int<value_type, I> ) //
		DAW_ATTRIB_INLINE constexpr unsigned_integer &operator^=( I rhs ) {
			m_private.value ^= as<value_type>( rhs );
			return *this;
		}

		[[nodiscard]] DAW_ATTRIB_INLINE explicit constexpr
		operator bool( ) const noexcept {
			return as<bool>( value( ) );
		}

		// Logical without short circuit
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		And( unsigned_integer const &rhs ) const noexcept {
			return as<bool>( *this ) and as<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		Or( unsigned_integer const &rhs ) const noexcept {
			return as<bool>( *this ) or as<bool>( rhs );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		reverse_bits( ) const noexcept {
			return unsigned_integer( daw::integers::reverse_bits( value( ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_leading_zeros( ) const noexcept {
			if constexpr( Bits == 8 ) {
				return daw::cxmath::count_leading_zeroes(
				         as<std::uint32_t>( value( ) ) ) -
				       24;
			} else if constexpr( Bits == 16 ) {
				return daw::cxmath::count_leading_zeroes(
				         as<std::uint32_t>( value( ) ) ) -
				       16;
			} else {
				return daw::cxmath::count_leading_zeroes( value( ) );
			}
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		count_trailing_zeros( ) const noexcept {
			if constexpr( Bits == 8 or Bits == 16 ) {
				return std::min( { as<std::uint32_t>( bit_count_v<value_type> ),
				                   daw::cxmath::count_trailing_zeros(
				                     as<std::uint32_t>( value( ) ) ) } );
			} else {
				return daw::cxmath::count_trailing_zeros( value( ) );
			}
		}

		/// @brief Convert to a signed_integer of the same width, like a
		/// static_cast.  The two's complement bits are kept, so values larger than
		/// the signed maximum wrap.  Requires daw/integers/daw_signed.h, or
		/// include daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer<Bits>
		as_signed( ) const noexcept {
			using signed_t = std::make_signed_t<value_type>;
			return signed_integer<Bits>( static_cast<signed_t>( value( ) ),
			                             signed_integer<Bits>::unchecked );
		}

		/// @brief Convert to a signed_integer of the same width.  Values larger
		/// than the signed maximum call the overflow handler and the two's
		/// complement bits are returned.  Requires daw/integers/daw_signed.h, or
		/// include daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr signed_integer<Bits>
		as_exact_signed( ) const {
			using signed_t = std::make_signed_t<value_type>;
			if( DAW_UNLIKELY( value( ) > daw::numeric_limits<signed_t>::max( ) ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
			}
			return as_signed( );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Checked in debug mode
		[[nodiscard]] constexpr unsigned_integer pow( unsigned pow ) const {
			return uint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs * rhs;
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Overflow is never checked
		[[nodiscard]] constexpr unsigned_integer
		pow_unchecked( unsigned pow ) const {
			return uint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_unchecked( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Overflow is checked and handler called if encountered
		[[nodiscard]] constexpr unsigned_integer pow_checked( unsigned pow ) const {
			return uint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_checked( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Value is wrapped when overflow is encountered
		[[nodiscard]] constexpr unsigned_integer pow_wrapped( unsigned pow ) const {
			return uint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_wrapped( rhs );
			} );
		}

		/// @brief compute pow using current value as base and pow as exponent.
		/// Value is saturated if overflow is encountered
		[[nodiscard]] constexpr unsigned_integer
		pow_saturated( unsigned pow ) const {
			return uint_impl::pow_impl( *this, pow, []( auto &&lhs, auto &&rhs ) {
				return lhs.mul_saturated( rhs );
			} );
		}

		/// @brief Returns the number of ones in the binary representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_ones( ) const noexcept {
			return static_cast<std::uint32_t>( std::popcount( value( ) ) );
		}

		/// @brief Returns the number of zeros in the binary representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_zeros( ) const noexcept {
			return static_cast<std::uint32_t>(
			  std::popcount( static_cast<value_type>( ~value( ) ) ) );
		}

		/// @brief Returns the number of leading ones in the binary representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_leading_ones( ) const noexcept {
			return static_cast<std::uint32_t>( std::countl_one( value( ) ) );
		}

		/// @brief Returns the number of trailing ones in the binary
		/// representation
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t
		count_trailing_ones( ) const noexcept {
			return static_cast<std::uint32_t>( std::countr_one( value( ) ) );
		}

		/// @brief Reverses the byte order
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		swap_bytes( ) const noexcept {
			return unsigned_integer( int_impl::swap_bytes( value( ) ), unchecked );
		}

		/// @brief Returns true when exactly one bit is set
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
		is_power_of_two( ) const noexcept {
			return std::has_single_bit( value( ) );
		}

		/// @brief Returns the smallest power of two that is not less than this.
		/// 0 returns 1.  When the result does not fit, it is 0 and overflow is
		/// reported in debug modes
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		next_power_of_two( ) const {
			if( DAW_UNLIKELY( value( ) > ( max( ).value( ) / 2U + 1U ) ) ) {
				DAW_UNLIKELY_BRANCH
#if DAW_DEFAULT_UNSIGNED_CHECKING == 0
				on_integer_overflow( );
#endif
				return unsigned_integer( );
			}
			return unsigned_integer( std::bit_ceil( value( ) ), unchecked );
		}

		/// @brief Returns the smallest power of two that is not less than this,
		/// std::nullopt when it does not fit
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_next_power_of_two( ) const noexcept {
			if( value( ) > ( max( ).value( ) / 2U + 1U ) ) {
				return std::nullopt;
			}
			return unsigned_integer( std::bit_ceil( value( ) ), unchecked );
		}

		/// @brief Returns floor(log2(*this)).  Zero calls the overflow handler and
		/// 0 is returned
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::uint32_t ilog2( ) const {
			if( DAW_UNLIKELY( value( ) == 0 ) ) {
				DAW_UNLIKELY_BRANCH
				on_integer_overflow( );
				return 0;
			}
			return static_cast<std::uint32_t>( std::bit_width( value( ) ) - 1 );
		}

		/// @brief Returns |*this - rhs|.  This cannot overflow
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr unsigned_integer
		abs_diff( unsigned_integer const &rhs ) const noexcept {
			return unsigned_integer( value( ) < rhs.value( )
			                           ? rhs.value( ) - value( )
			                           : value( ) - rhs.value( ),
			                         unchecked );
		}

		// try_ operations return std::nullopt instead of reporting an error.  The
		// error handlers are never called

		/// @brief Converts other if it is in range
		template<typename I>
		requires daw::is_integral_v<I> //
		[[nodiscard]] static constexpr std::optional<unsigned_integer>
		try_from( I other ) noexcept {
			if( not daw::in_range<value_type>( other ) ) {
				return std::nullopt;
			}
			return unsigned_integer( other, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_add( unsigned_integer const &rhs ) const noexcept {
			auto const r = add_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return unsigned_integer( r.value, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_sub( unsigned_integer const &rhs ) const noexcept {
			auto const r = sub_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return unsigned_integer( r.value, unchecked );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_mul( unsigned_integer const &rhs ) const noexcept {
			auto const r = mul_overflowing( rhs );
			if( r.overflowed ) {
				return std::nullopt;
			}
			return unsigned_integer( r.value, unchecked );
		}

		/// @brief Division, std::nullopt when rhs is 0
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_div( unsigned_integer const &rhs ) const noexcept {
			if( rhs.value( ) == 0 ) {
				return std::nullopt;
			}
			return unsigned_integer( value( ) / rhs.value( ), unchecked );
		}

		/// @brief Remainder, std::nullopt when rhs is 0
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_rem( unsigned_integer const &rhs ) const noexcept {
			if( rhs.value( ) == 0 ) {
				return std::nullopt;
			}
			return unsigned_integer( value( ) % rhs.value( ), unchecked );
		}

		/// @brief For unsigned values this is the same as try_div
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_div_euclid( unsigned_integer const &rhs ) const noexcept {
			return try_div( rhs );
		}

		/// @brief For unsigned values this is the same as try_rem
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_rem_euclid( unsigned_integer const &rhs ) const noexcept {
			return try_rem( rhs );
		}

		/// @brief Shift left, std::nullopt when rhs is not less than the bit
		/// width.  Bits shifted out are discarded
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_shl( unsigned_integer const &rhs ) const noexcept {
			if( rhs.value( ) >= daw::bit_count_v<value_type> ) {
				return std::nullopt;
			}
			return unsigned_integer( uint_impl::promote( value( ) ) << rhs.value( ),
			                         unchecked );
		}

		/// @brief Shift right, std::nullopt when rhs is not less than the bit
		/// width
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<unsigned_integer>
		try_shr( unsigned_integer const &rhs ) const noexcept {
			if( rhs.value( ) >= daw::bit_count_v<value_type> ) {
				return std::nullopt;
			}
			return unsigned_integer( value( ) >> rhs.value( ), unchecked );
		}

		[[nodiscard]] constexpr std::optional<unsigned_integer>
		try_pow( unsigned exp ) const noexcept {
			bool overflowed = false;
			auto const result = uint_impl::pow_impl(
			  *this,
			  exp,
			  [&]( unsigned_integer const &lhs, unsigned_integer const &rhs ) {
				  auto const r = lhs.mul_overflowing( rhs );
				  overflowed |= r.overflowed;
				  return unsigned_integer( r.value, unchecked );
			  } );
			if( overflowed ) {
				return std::nullopt;
			}
			return result;
		}

		/// @brief floor(log2(*this)), std::nullopt when 0
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<std::uint32_t>
		try_ilog2( ) const noexcept {
			if( value( ) == 0 ) {
				return std::nullopt;
			}
			return ilog2( );
		}

		/// @brief Convert to a signed_integer of the same width, std::nullopt when
		/// larger than the signed maximum.  Requires daw/integers/daw_signed.h,
		/// or include daw/daw_integer.h
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr std::optional<
		  signed_integer<Bits>>
		try_as_signed( ) const noexcept {
			using signed_t = std::make_signed_t<value_type>;
			if( value( ) > daw::numeric_limits<signed_t>::max( ) ) {
				return std::nullopt;
			}
			return as_signed( );
		}
	};

	template<typename I>
	requires(
	  daw::traits::is_one_of_v<daw::make_unsigned_t<I>, std::uint8_t,
	                           std::uint16_t, std::uint32_t, std::uint64_t> ) //
	unsigned_integer( I ) -> unsigned_integer<sizeof( I ) * 8>;

	// Addition
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result += result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		auto result = result_t( lhs.value( ) );
		result += result_t( rhs );
		return result;
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator+( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		auto result = result_t( lhs );
		result += result_t( rhs.value( ) );
		return result;
	}

	// Subtraction
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result -= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		auto result = result_t( lhs.value( ) );
		result -= result_t( rhs );
		return result;
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator-( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		auto result = result_t( lhs );
		result -= result_t( rhs.value( ) );
		return result;
	}

	// Multiplication
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result *= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		auto result = result_t( lhs.value( ) );
		result *= result_t( rhs );
		return result;
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator*( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		auto result = result_t( lhs );
		result *= result_t( rhs.value( ) );
		return result;
	}

	// Division
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result /= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		auto result = result_t( lhs.value( ) );
		result /= result_t( rhs );
		return result;
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator/( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		auto result = result_t( lhs );
		result /= result_t( rhs.value( ) );
		return result;
	}

	// Remainder
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		auto result = result_t( lhs.value( ) );
		result %= result_t( rhs.value( ) );
		return result;
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		auto result = result_t( lhs.value( ) );
		result %= result_t( rhs );
		return result;
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator%( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		auto result = result_t( lhs );
		result %= result_t( rhs.value( ) );
		return result;
	}

	// Shift Left
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		return result_t( lhs.value( ) ) <<= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		return result_t( lhs.value( ) ) <<= result_t( rhs );
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<<( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		return result_t( lhs ) <<= result_t( rhs.value( ) );
	}

	// Shift Right
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		return result_t( lhs.value( ) ) >>= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		return result_t( lhs.value( ) ) >>= result_t( rhs );
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>>( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		return result_t( lhs ) >>= result_t( rhs.value( ) );
	}

	// Bitwise Or
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		return result_t( lhs.value( ) ) |= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		return result_t( lhs.value( ) ) |= result_t( rhs );
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator|( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		return result_t( lhs ) |= result_t( rhs.value( ) );
	}

	// Bitwise And
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		return result_t( lhs.value( ) ) &= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		return result_t( lhs.value( ) ) &= result_t( rhs );
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator&( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		return result_t( lhs ) &= result_t( rhs.value( ) );
	}

	// Bitwise Xor
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, rhs_t>;
		return result_t( lhs.value( ) ) ^= result_t( rhs.value( ) );
	}

	template<std::size_t Lhs, uint_impl::UnsignedIntegral Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( unsigned_integer<Lhs> lhs, Rhs rhs ) {
		using lhs_t = uint_impl::unsigned_integer_type_t<Lhs>;
		using result_t = uint_impl::uint_result_t<lhs_t, Rhs>;
		return result_t( lhs.value( ) ) ^= result_t( rhs );
	}

	template<uint_impl::UnsignedIntegral Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator^( Lhs lhs, unsigned_integer<Rhs> rhs ) {
		using rhs_t = uint_impl::unsigned_integer_type_t<Rhs>;
		using result_t = uint_impl::uint_result_t<Lhs, rhs_t>;
		return result_t( lhs ) ^= result_t( rhs.value( ) );
	}

	// Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator==( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator==( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator==( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_equal( lhs, rhs.value( ) );
	}

	// Not Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator!=( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_not_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator!=( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_not_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_not_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator!=( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_not_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_not_equal( lhs, rhs.value( ) );
	}

	// Less Than
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator<( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_less( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_less( lhs.value( ), rhs ) ) {
		return daw::cmp_less( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_less( lhs, rhs.value( ) ) ) {
		return daw::cmp_less( lhs, rhs.value( ) );
	}

	// Less Than or Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator<=( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_less_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<=( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_less_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_less_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator<=( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_less_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_less_equal( lhs, rhs.value( ) );
	}

	// Greater Than
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator>( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_greater( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_greater( lhs.value( ), rhs ) ) {
		return daw::cmp_greater( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_greater( lhs, rhs.value( ) ) ) {
		return daw::cmp_greater( lhs, rhs.value( ) );
	}

	// Greater Than or Equal To
	template<std::size_t Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr bool
	operator>=( unsigned_integer<Lhs> lhs, unsigned_integer<Rhs> rhs ) {
		return daw::cmp_greater_equal( lhs.value( ), rhs.value( ) );
	}

	template<std::size_t Lhs, typename Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>=( unsigned_integer<Lhs> lhs, Rhs &&rhs )
	  -> decltype( daw::cmp_greater_equal( lhs.value( ), rhs ) ) {
		return daw::cmp_greater_equal( lhs.value( ), rhs );
	}

	template<typename Lhs, std::size_t Rhs>
	[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
	operator>=( Lhs &&lhs, unsigned_integer<Rhs> rhs )
	  -> decltype( daw::cmp_greater_equal( lhs, rhs.value( ) ) ) {
		return daw::cmp_greater_equal( lhs, rhs.value( ) );
	}
} // namespace daw::integers::inline DAW_INTEGER_VER

namespace daw::integers::inline DAW_INTEGER_VER::literals {
	[[nodiscard]] DAW_CONSTEVAL unsigned_integer<8>
	operator""_u8( unsigned long long v ) {
		using int_t = std::uint8_t;
		if( not daw::in_range<int_t>( v ) ) {
			on_integer_overflow( );
		}
		return unsigned_integer<8>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL unsigned_integer<16>
	operator""_u16( unsigned long long v ) {
		using int_t = std::uint16_t;
		if( not daw::in_range<int_t>( v ) ) {
			on_integer_overflow( );
		}
		return unsigned_integer<16>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL unsigned_integer<32>
	operator""_u32( unsigned long long v ) {
		using int_t = std::uint32_t;
		if( not daw::in_range<int_t>( v ) ) {
			on_integer_overflow( );
		}
		return unsigned_integer<32>( as<int_t>( v ) );
	}

	[[nodiscard]] DAW_CONSTEVAL unsigned_integer<64>
	operator""_u64( unsigned long long v ) {
		using int_t = std::uint64_t;
		if( not daw::in_range<int_t>( v ) ) {
			on_integer_overflow( );
		}
		return unsigned_integer<64>( as<int_t>( v ) );
	}
} // namespace daw::integers::inline DAW_INTEGER_VER::literals

namespace daw {
	using daw::integers::u16;
	using daw::integers::u32;
	using daw::integers::u64;
	using daw::integers::u8;

	template<>
	struct make_unsigned<daw::integers::u8> {
		using type = std::uint8_t;
	};

	template<>
	struct make_unsigned<daw::integers::u16> {
		using type = std::uint16_t;
	};

	template<>
	struct make_unsigned<daw::integers::u32> {
		using type = std::uint32_t;
	};

	template<>
	struct make_unsigned<daw::integers::u64> {
		using type = std::uint64_t;
	};

	template<>
	struct make_signed<daw::integers::u8> {
		using type = std::int8_t;
	};

	template<>
	struct make_signed<daw::integers::u16> {
		using type = std::int16_t;
	};

	template<>
	struct make_signed<daw::integers::u32> {
		using type = std::int32_t;
	};

	template<>
	struct make_signed<daw::integers::u64> {
		using type = std::int64_t;
	};
} // namespace daw

namespace std {
	template<std::size_t Bits>
	struct numeric_limits<daw::integers::unsigned_integer<Bits>> {
		static constexpr bool is_specialized = true;
		static constexpr bool is_signed = false;
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
		  typename daw::integers::unsigned_integer<Bits>::value_type>::is_modulo;
		static constexpr int digits = Bits;

		static constexpr int digits10 = digits * 3 / 10;
		static constexpr int max_digits10 = 0;
		static constexpr int radix = 2;
		static constexpr int min_exponent = 0;
		static constexpr int min_exponent10 = 0;
		static constexpr int max_exponent = 0;
		static constexpr int max_exponent10 = 0;

		static constexpr bool traps = true;
		static constexpr bool tinyness_before = false;

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		min( ) noexcept {
			return daw::integers::unsigned_integer<Bits>::min( );
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		max( ) noexcept {
			return daw::integers::unsigned_integer<Bits>::max( );
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		lowest( ) noexcept {
			return daw::integers::unsigned_integer<Bits>::min( );
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		epsilon( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		round_error( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		infinity( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		quiet_NaN( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		signaling_NaN( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}

		[[nodiscard]] static constexpr daw::integers::unsigned_integer<Bits>
		denorm_min( ) noexcept {
			return daw::integers::unsigned_integer<Bits>{ };
		}
	};

	/// std::hash support.  Hashes the same as the underlying value_type
	template<std::size_t Bits>
	struct hash<daw::integers::unsigned_integer<Bits>> {
		[[nodiscard]] std::size_t
		operator( )( daw::integers::unsigned_integer<Bits> v ) const noexcept {
			return std::hash<
			  typename daw::integers::unsigned_integer<Bits>::value_type>{ }(
			  v.value( ) );
		}
	};
} // namespace std
