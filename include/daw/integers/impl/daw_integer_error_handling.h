// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include <daw/daw_arith_traits.h>
#include <daw/daw_attributes.h>
#include <daw/daw_check_exceptions.h>
#include <daw/daw_cpp_feature_check.h>

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

/// Error handling shared by signed_integer and unsigned_integer
namespace daw::integers {
	enum class IntegerErrorType { Overflow, DivideByZero, None };
	using integer_error_handler_t = void ( * )( void *, IntegerErrorType );

	namespace int_impl {
		inline auto &get_integer_overflow_handler( ) {
			static DAW_CONSTINIT struct handler_t {
				integer_error_handler_t cb = nullptr;
				void *data = nullptr;
			} handler{ };
			return handler;
		}

		inline auto &get_integer_div_by_zero_handler( ) {
			static DAW_CONSTINIT struct handler_t {
				integer_error_handler_t cb = nullptr;
				void *data = nullptr;
			} handler{ };
			return handler;
		}
	} // namespace int_impl

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	DAW_ATTRIB_NOINLINE inline void
	register_integer_overflow_handler( integer_error_handler_t handler = nullptr,
	                                   void *data = nullptr ) noexcept {
		int_impl::get_integer_overflow_handler( ).cb = handler;
		int_impl::get_integer_overflow_handler( ).data = data;
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	template<typename Func>
	requires( std::is_class_v<Func> and
	          std::is_invocable_v<Func, IntegerErrorType> )
	DAW_ATTRIB_NOINLINE inline void
	register_integer_overflow_handler( Func &handler ) noexcept {
		if constexpr( std::is_const_v<Func> ) {
			register_integer_overflow_handler(
			  +[]( void *vhnd, IntegerErrorType error_type ) {
				  (void)( *static_cast<Func const *>( vhnd ) )( error_type );
			  },
			  const_cast<void *>(
			    static_cast<void const *>( std::addressof( handler ) ) ) );
		} else {
			register_integer_overflow_handler(
			  +[]( void *vhnd, IntegerErrorType error_type ) {
				  (void)( *static_cast<Func *>( vhnd ) )( error_type );
			  },
			  static_cast<void *>( std::addressof( handler ) ) );
		}
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	DAW_ATTRIB_NOINLINE inline void register_integer_div_by_zero_handler(
	  integer_error_handler_t handler = nullptr,
	  void *data = nullptr ) noexcept {
		int_impl::get_integer_div_by_zero_handler( ).cb = handler;
		int_impl::get_integer_div_by_zero_handler( ).data = data;
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	template<typename Func>
	requires( std::is_class_v<Func> and
	          std::is_invocable_v<Func, IntegerErrorType> )
	DAW_ATTRIB_NOINLINE inline void
	register_integer_div_by_zero_handler( Func &handler ) noexcept {
		if constexpr( std::is_const_v<Func> ) {
			register_integer_div_by_zero_handler(
			  +[]( void *vhnd, IntegerErrorType error_type ) {
				  (void)( *static_cast<Func const *>( vhnd ) )( error_type );
			  },
			  const_cast<void *>(
			    static_cast<void const *>( std::addressof( handler ) ) ) );
		} else {
			register_integer_div_by_zero_handler(
			  +[]( void *vhnd, IntegerErrorType error_type ) {
				  (void)( *static_cast<Func *>( vhnd ) )( error_type );
			  },
			  static_cast<void *>( std::addressof( handler ) ) );
		}
	}

	struct integer_overflow_exception : std::exception {};
	struct integer_div_by_zero_exception : std::exception {};

	DAW_ATTRIB_NOINLINE inline void on_integer_overflow( ) {
		auto handler = int_impl::get_integer_overflow_handler( );
		if( handler.cb ) {
			handler.cb( handler.data, IntegerErrorType::Overflow );
			return;
		}
		DAW_THROW_OR_TERMINATE_NA( integer_overflow_exception );
	}

	DAW_ATTRIB_NOINLINE inline void on_integer_div_by_zero( ) {
		auto handler = int_impl::get_integer_div_by_zero_handler( );
		if( handler.cb ) {
			handler.cb( handler.data, IntegerErrorType::DivideByZero );
			return;
		}
		DAW_THROW_OR_TERMINATE_NA( integer_div_by_zero_exception );
	}
} // namespace daw::integers
