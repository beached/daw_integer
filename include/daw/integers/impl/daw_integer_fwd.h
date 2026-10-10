// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#pragma once

#include "daw/integers/impl/version.h"

#include <cstddef>

namespace daw::integers::inline DAW_INTEGER_VER::sint_impl {
	template<std::size_t /*Bits*/>
	struct signed_integer;
} // namespace daw::integers::inline DAW_INTEGER_VER::sint_impl

namespace daw::integers::inline DAW_INTEGER_VER::uint_impl {
	template<std::size_t /*Bits*/>
	struct unsigned_integer;
} // namespace daw::integers::inline DAW_INTEGER_VER::uint_impl
