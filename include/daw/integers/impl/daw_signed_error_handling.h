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
#include <daw/daw_check_exceptions.h>
#include <daw/daw_cpp_feature_check.h>

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

/// \brief DAW_DEFAULT_SIGNED_CHECKING can be defined to the following
/// 0 - Checked with wrapping defaults if not a named op(e.g. add_wrapped)(for
/// add/sub/mul) 1 - Unchecked 2 - Wrapped for add/sub/mul, unchecked for others
#if not defined( DAW_DEFAULT_SIGNED_CHECKING )
#if defined( DEBUG ) or not defined( NDEBUG )
#define DAW_DEFAULT_SIGNED_CHECKING 0
#else
#define DAW_DEFAULT_SIGNED_CHECKING 1
#endif
#endif

namespace daw::integers::inline DAW_INTEGER_VER {
	// The signed names are kept for compatibility.  Error handling is shared
	// with unsigned_integer, see daw_integer_error_handling.h
	using SignedIntegerErrorType = IntegerErrorType;
	using signed_int_error_handler_t = integer_error_handler_t;
	using signed_integer_overflow_exception = integer_overflow_exception;
	using signed_integer_div_by_zero_exception = integer_div_by_zero_exception;

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	DAW_ATTRIB_INLINE void register_signed_overflow_handler(
	  integer_error_handler_t handler = nullptr, void *data = nullptr ) noexcept {
		register_integer_overflow_handler( handler, data );
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	template<typename Func>
	requires( std::is_class_v<Func> and
	          std::is_invocable_v<Func, IntegerErrorType> )
	DAW_ATTRIB_INLINE void
	register_signed_overflow_handler( Func &handler ) noexcept {
		register_integer_overflow_handler( handler );
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	DAW_ATTRIB_INLINE void register_signed_div_by_zero_handler(
	  integer_error_handler_t handler = nullptr, void *data = nullptr ) noexcept {
		register_integer_div_by_zero_handler( handler, data );
	}

	/// Caller is responsible for ensuring that this is called in a context that
	/// protects against multiple threads accessing/writing at the same time
	template<typename Func>
	requires( std::is_class_v<Func> and
	          std::is_invocable_v<Func, IntegerErrorType> )
	DAW_ATTRIB_INLINE void
	register_signed_div_by_zero_handler( Func &handler ) noexcept {
		register_integer_div_by_zero_handler( handler );
	}

	DAW_ATTRIB_INLINE void on_signed_integer_overflow( ) {
		on_integer_overflow( );
	}

	DAW_ATTRIB_INLINE void on_signed_integer_div_by_zero( ) {
		on_integer_div_by_zero( );
	}
} // namespace daw::integers::inline DAW_INTEGER_VER
