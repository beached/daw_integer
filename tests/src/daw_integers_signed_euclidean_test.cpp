// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define DAW_DEFAULT_SIGNED_CHECKING 0

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

// div_euclid
static_assert( ( 7_i8 ).div_euclid( 3_i8 ) == 2_i8 );
static_assert( ( 7_i16 ).div_euclid( 3_i16 ) == 2_i16 );
static_assert( ( 7_i32 ).div_euclid( 3_i32 ) == 2_i32 );
static_assert( ( 7_i64 ).div_euclid( 3_i64 ) == 2_i64 );

static_assert( ( 7_i8 ).div_euclid( ( -3_i8 ) ) == ( -2_i8 ) );
static_assert( ( 7_i16 ).div_euclid( ( -3_i16 ) ) == ( -2_i16 ) );
static_assert( ( 7_i32 ).div_euclid( ( -3_i32 ) ) == ( -2_i32 ) );
static_assert( ( 7_i64 ).div_euclid( ( -3_i64 ) ) == ( -2_i64 ) );

static_assert( ( -7_i8 ).div_euclid( 3_i8 ) == ( -3_i8 ) );
static_assert( ( -7_i16 ).div_euclid( 3_i16 ) == ( -3_i16 ) );
static_assert( ( -7_i32 ).div_euclid( 3_i32 ) == ( -3_i32 ) );
static_assert( ( -7_i64 ).div_euclid( 3_i64 ) == ( -3_i64 ) );

static_assert( ( -7_i8 ).div_euclid( ( -3_i8 ) ) == 3_i8 );
static_assert( ( -7_i16 ).div_euclid( ( -3_i16 ) ) == 3_i16 );
static_assert( ( -7_i32 ).div_euclid( ( -3_i32 ) ) == 3_i32 );
static_assert( ( -7_i64 ).div_euclid( ( -3_i64 ) ) == 3_i64 );

static_assert( ( 6_i8 ).div_euclid( 3_i8 ) == 2_i8 );
static_assert( ( 6_i16 ).div_euclid( 3_i16 ) == 2_i16 );
static_assert( ( 6_i32 ).div_euclid( 3_i32 ) == 2_i32 );
static_assert( ( 6_i64 ).div_euclid( 3_i64 ) == 2_i64 );

static_assert( ( -6_i8 ).div_euclid( 3_i8 ) == ( -2_i8 ) );
static_assert( ( -6_i16 ).div_euclid( 3_i16 ) == ( -2_i16 ) );
static_assert( ( -6_i32 ).div_euclid( 3_i32 ) == ( -2_i32 ) );
static_assert( ( -6_i64 ).div_euclid( 3_i64 ) == ( -2_i64 ) );

static_assert( ( -6_i8 ).div_euclid( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -6_i16 ).div_euclid( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -6_i32 ).div_euclid( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -6_i64 ).div_euclid( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 0_i8 ).div_euclid( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).div_euclid( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).div_euclid( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).div_euclid( ( -3_i64 ) ) == 0_i64 );

// div_euclid_checked
static_assert( ( 7_i8 ).div_euclid_checked( 3_i8 ) == 2_i8 );
static_assert( ( 7_i16 ).div_euclid_checked( 3_i16 ) == 2_i16 );
static_assert( ( 7_i32 ).div_euclid_checked( 3_i32 ) == 2_i32 );
static_assert( ( 7_i64 ).div_euclid_checked( 3_i64 ) == 2_i64 );

static_assert( ( 7_i8 ).div_euclid_checked( ( -3_i8 ) ) == ( -2_i8 ) );
static_assert( ( 7_i16 ).div_euclid_checked( ( -3_i16 ) ) == ( -2_i16 ) );
static_assert( ( 7_i32 ).div_euclid_checked( ( -3_i32 ) ) == ( -2_i32 ) );
static_assert( ( 7_i64 ).div_euclid_checked( ( -3_i64 ) ) == ( -2_i64 ) );

static_assert( ( -7_i8 ).div_euclid_checked( 3_i8 ) == ( -3_i8 ) );
static_assert( ( -7_i16 ).div_euclid_checked( 3_i16 ) == ( -3_i16 ) );
static_assert( ( -7_i32 ).div_euclid_checked( 3_i32 ) == ( -3_i32 ) );
static_assert( ( -7_i64 ).div_euclid_checked( 3_i64 ) == ( -3_i64 ) );

static_assert( ( -7_i8 ).div_euclid_checked( ( -3_i8 ) ) == 3_i8 );
static_assert( ( -7_i16 ).div_euclid_checked( ( -3_i16 ) ) == 3_i16 );
static_assert( ( -7_i32 ).div_euclid_checked( ( -3_i32 ) ) == 3_i32 );
static_assert( ( -7_i64 ).div_euclid_checked( ( -3_i64 ) ) == 3_i64 );

static_assert( ( 6_i8 ).div_euclid_checked( 3_i8 ) == 2_i8 );
static_assert( ( 6_i16 ).div_euclid_checked( 3_i16 ) == 2_i16 );
static_assert( ( 6_i32 ).div_euclid_checked( 3_i32 ) == 2_i32 );
static_assert( ( 6_i64 ).div_euclid_checked( 3_i64 ) == 2_i64 );

static_assert( ( -6_i8 ).div_euclid_checked( 3_i8 ) == ( -2_i8 ) );
static_assert( ( -6_i16 ).div_euclid_checked( 3_i16 ) == ( -2_i16 ) );
static_assert( ( -6_i32 ).div_euclid_checked( 3_i32 ) == ( -2_i32 ) );
static_assert( ( -6_i64 ).div_euclid_checked( 3_i64 ) == ( -2_i64 ) );

static_assert( ( -6_i8 ).div_euclid_checked( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -6_i16 ).div_euclid_checked( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -6_i32 ).div_euclid_checked( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -6_i64 ).div_euclid_checked( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 0_i8 ).div_euclid_checked( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).div_euclid_checked( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).div_euclid_checked( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).div_euclid_checked( ( -3_i64 ) ) == 0_i64 );

// div_euclid_saturated
static_assert( ( 7_i8 ).div_euclid_saturated( 3_i8 ) == 2_i8 );
static_assert( ( 7_i16 ).div_euclid_saturated( 3_i16 ) == 2_i16 );
static_assert( ( 7_i32 ).div_euclid_saturated( 3_i32 ) == 2_i32 );
static_assert( ( 7_i64 ).div_euclid_saturated( 3_i64 ) == 2_i64 );

static_assert( ( 7_i8 ).div_euclid_saturated( ( -3_i8 ) ) == ( -2_i8 ) );
static_assert( ( 7_i16 ).div_euclid_saturated( ( -3_i16 ) ) == ( -2_i16 ) );
static_assert( ( 7_i32 ).div_euclid_saturated( ( -3_i32 ) ) == ( -2_i32 ) );
static_assert( ( 7_i64 ).div_euclid_saturated( ( -3_i64 ) ) == ( -2_i64 ) );

static_assert( ( -7_i8 ).div_euclid_saturated( 3_i8 ) == ( -3_i8 ) );
static_assert( ( -7_i16 ).div_euclid_saturated( 3_i16 ) == ( -3_i16 ) );
static_assert( ( -7_i32 ).div_euclid_saturated( 3_i32 ) == ( -3_i32 ) );
static_assert( ( -7_i64 ).div_euclid_saturated( 3_i64 ) == ( -3_i64 ) );

static_assert( ( -7_i8 ).div_euclid_saturated( ( -3_i8 ) ) == 3_i8 );
static_assert( ( -7_i16 ).div_euclid_saturated( ( -3_i16 ) ) == 3_i16 );
static_assert( ( -7_i32 ).div_euclid_saturated( ( -3_i32 ) ) == 3_i32 );
static_assert( ( -7_i64 ).div_euclid_saturated( ( -3_i64 ) ) == 3_i64 );

static_assert( ( 6_i8 ).div_euclid_saturated( 3_i8 ) == 2_i8 );
static_assert( ( 6_i16 ).div_euclid_saturated( 3_i16 ) == 2_i16 );
static_assert( ( 6_i32 ).div_euclid_saturated( 3_i32 ) == 2_i32 );
static_assert( ( 6_i64 ).div_euclid_saturated( 3_i64 ) == 2_i64 );

static_assert( ( -6_i8 ).div_euclid_saturated( 3_i8 ) == ( -2_i8 ) );
static_assert( ( -6_i16 ).div_euclid_saturated( 3_i16 ) == ( -2_i16 ) );
static_assert( ( -6_i32 ).div_euclid_saturated( 3_i32 ) == ( -2_i32 ) );
static_assert( ( -6_i64 ).div_euclid_saturated( 3_i64 ) == ( -2_i64 ) );

static_assert( ( -6_i8 ).div_euclid_saturated( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -6_i16 ).div_euclid_saturated( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -6_i32 ).div_euclid_saturated( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -6_i64 ).div_euclid_saturated( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 0_i8 ).div_euclid_saturated( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).div_euclid_saturated( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).div_euclid_saturated( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).div_euclid_saturated( ( -3_i64 ) ) == 0_i64 );

// div_euclid_wrapped
static_assert( ( 7_i8 ).div_euclid_wrapped( 3_i8 ) == 2_i8 );
static_assert( ( 7_i16 ).div_euclid_wrapped( 3_i16 ) == 2_i16 );
static_assert( ( 7_i32 ).div_euclid_wrapped( 3_i32 ) == 2_i32 );
static_assert( ( 7_i64 ).div_euclid_wrapped( 3_i64 ) == 2_i64 );

static_assert( ( 7_i8 ).div_euclid_wrapped( ( -3_i8 ) ) == ( -2_i8 ) );
static_assert( ( 7_i16 ).div_euclid_wrapped( ( -3_i16 ) ) == ( -2_i16 ) );
static_assert( ( 7_i32 ).div_euclid_wrapped( ( -3_i32 ) ) == ( -2_i32 ) );
static_assert( ( 7_i64 ).div_euclid_wrapped( ( -3_i64 ) ) == ( -2_i64 ) );

static_assert( ( -7_i8 ).div_euclid_wrapped( 3_i8 ) == ( -3_i8 ) );
static_assert( ( -7_i16 ).div_euclid_wrapped( 3_i16 ) == ( -3_i16 ) );
static_assert( ( -7_i32 ).div_euclid_wrapped( 3_i32 ) == ( -3_i32 ) );
static_assert( ( -7_i64 ).div_euclid_wrapped( 3_i64 ) == ( -3_i64 ) );

static_assert( ( -7_i8 ).div_euclid_wrapped( ( -3_i8 ) ) == 3_i8 );
static_assert( ( -7_i16 ).div_euclid_wrapped( ( -3_i16 ) ) == 3_i16 );
static_assert( ( -7_i32 ).div_euclid_wrapped( ( -3_i32 ) ) == 3_i32 );
static_assert( ( -7_i64 ).div_euclid_wrapped( ( -3_i64 ) ) == 3_i64 );

static_assert( ( 6_i8 ).div_euclid_wrapped( 3_i8 ) == 2_i8 );
static_assert( ( 6_i16 ).div_euclid_wrapped( 3_i16 ) == 2_i16 );
static_assert( ( 6_i32 ).div_euclid_wrapped( 3_i32 ) == 2_i32 );
static_assert( ( 6_i64 ).div_euclid_wrapped( 3_i64 ) == 2_i64 );

static_assert( ( -6_i8 ).div_euclid_wrapped( 3_i8 ) == ( -2_i8 ) );
static_assert( ( -6_i16 ).div_euclid_wrapped( 3_i16 ) == ( -2_i16 ) );
static_assert( ( -6_i32 ).div_euclid_wrapped( 3_i32 ) == ( -2_i32 ) );
static_assert( ( -6_i64 ).div_euclid_wrapped( 3_i64 ) == ( -2_i64 ) );

static_assert( ( -6_i8 ).div_euclid_wrapped( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -6_i16 ).div_euclid_wrapped( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -6_i32 ).div_euclid_wrapped( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -6_i64 ).div_euclid_wrapped( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 0_i8 ).div_euclid_wrapped( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).div_euclid_wrapped( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).div_euclid_wrapped( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).div_euclid_wrapped( ( -3_i64 ) ) == 0_i64 );

// rem_euclid
static_assert( ( 7_i8 ).rem_euclid( 3_i8 ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid( 3_i16 ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid( 3_i32 ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid( 3_i64 ) == 1_i64 );

static_assert( ( 7_i8 ).rem_euclid( ( -3_i8 ) ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid( ( -3_i16 ) ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid( ( -3_i32 ) ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid( ( -3_i64 ) ) == 1_i64 );

static_assert( ( -7_i8 ).rem_euclid( 3_i8 ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid( 3_i16 ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid( 3_i32 ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid( 3_i64 ) == 2_i64 );

static_assert( ( -7_i8 ).rem_euclid( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 6_i8 ).rem_euclid( 3_i8 ) == 0_i8 );
static_assert( ( 6_i16 ).rem_euclid( 3_i16 ) == 0_i16 );
static_assert( ( 6_i32 ).rem_euclid( 3_i32 ) == 0_i32 );
static_assert( ( 6_i64 ).rem_euclid( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid( 3_i8 ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid( 3_i16 ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid( 3_i32 ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid( ( -3_i8 ) ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid( ( -3_i16 ) ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid( ( -3_i32 ) ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid( ( -3_i64 ) ) == 0_i64 );

static_assert( ( 0_i8 ).rem_euclid( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).rem_euclid( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).rem_euclid( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).rem_euclid( ( -3_i64 ) ) == 0_i64 );

// rem_euclid_checked
static_assert( ( 7_i8 ).rem_euclid_checked( 3_i8 ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_checked( 3_i16 ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_checked( 3_i32 ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_checked( 3_i64 ) == 1_i64 );

static_assert( ( 7_i8 ).rem_euclid_checked( ( -3_i8 ) ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_checked( ( -3_i16 ) ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_checked( ( -3_i32 ) ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_checked( ( -3_i64 ) ) == 1_i64 );

static_assert( ( -7_i8 ).rem_euclid_checked( 3_i8 ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_checked( 3_i16 ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_checked( 3_i32 ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_checked( 3_i64 ) == 2_i64 );

static_assert( ( -7_i8 ).rem_euclid_checked( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_checked( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_checked( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_checked( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 6_i8 ).rem_euclid_checked( 3_i8 ) == 0_i8 );
static_assert( ( 6_i16 ).rem_euclid_checked( 3_i16 ) == 0_i16 );
static_assert( ( 6_i32 ).rem_euclid_checked( 3_i32 ) == 0_i32 );
static_assert( ( 6_i64 ).rem_euclid_checked( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_checked( 3_i8 ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_checked( 3_i16 ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_checked( 3_i32 ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_checked( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_checked( ( -3_i8 ) ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_checked( ( -3_i16 ) ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_checked( ( -3_i32 ) ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_checked( ( -3_i64 ) ) == 0_i64 );

static_assert( ( 0_i8 ).rem_euclid_checked( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).rem_euclid_checked( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).rem_euclid_checked( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).rem_euclid_checked( ( -3_i64 ) ) == 0_i64 );

// rem_euclid_saturated
static_assert( ( 7_i8 ).rem_euclid_saturated( 3_i8 ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_saturated( 3_i16 ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_saturated( 3_i32 ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_saturated( 3_i64 ) == 1_i64 );

static_assert( ( 7_i8 ).rem_euclid_saturated( ( -3_i8 ) ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_saturated( ( -3_i16 ) ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_saturated( ( -3_i32 ) ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_saturated( ( -3_i64 ) ) == 1_i64 );

static_assert( ( -7_i8 ).rem_euclid_saturated( 3_i8 ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_saturated( 3_i16 ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_saturated( 3_i32 ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_saturated( 3_i64 ) == 2_i64 );

static_assert( ( -7_i8 ).rem_euclid_saturated( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_saturated( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_saturated( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_saturated( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 6_i8 ).rem_euclid_saturated( 3_i8 ) == 0_i8 );
static_assert( ( 6_i16 ).rem_euclid_saturated( 3_i16 ) == 0_i16 );
static_assert( ( 6_i32 ).rem_euclid_saturated( 3_i32 ) == 0_i32 );
static_assert( ( 6_i64 ).rem_euclid_saturated( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_saturated( 3_i8 ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_saturated( 3_i16 ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_saturated( 3_i32 ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_saturated( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_saturated( ( -3_i8 ) ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_saturated( ( -3_i16 ) ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_saturated( ( -3_i32 ) ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_saturated( ( -3_i64 ) ) == 0_i64 );

static_assert( ( 0_i8 ).rem_euclid_saturated( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).rem_euclid_saturated( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).rem_euclid_saturated( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).rem_euclid_saturated( ( -3_i64 ) ) == 0_i64 );

// rem_euclid_unchecked
static_assert( ( 7_i8 ).rem_euclid_unchecked( 3_i8 ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_unchecked( 3_i16 ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_unchecked( 3_i32 ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_unchecked( 3_i64 ) == 1_i64 );

static_assert( ( 7_i8 ).rem_euclid_unchecked( ( -3_i8 ) ) == 1_i8 );
static_assert( ( 7_i16 ).rem_euclid_unchecked( ( -3_i16 ) ) == 1_i16 );
static_assert( ( 7_i32 ).rem_euclid_unchecked( ( -3_i32 ) ) == 1_i32 );
static_assert( ( 7_i64 ).rem_euclid_unchecked( ( -3_i64 ) ) == 1_i64 );

static_assert( ( -7_i8 ).rem_euclid_unchecked( 3_i8 ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_unchecked( 3_i16 ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_unchecked( 3_i32 ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_unchecked( 3_i64 ) == 2_i64 );

static_assert( ( -7_i8 ).rem_euclid_unchecked( ( -3_i8 ) ) == 2_i8 );
static_assert( ( -7_i16 ).rem_euclid_unchecked( ( -3_i16 ) ) == 2_i16 );
static_assert( ( -7_i32 ).rem_euclid_unchecked( ( -3_i32 ) ) == 2_i32 );
static_assert( ( -7_i64 ).rem_euclid_unchecked( ( -3_i64 ) ) == 2_i64 );

static_assert( ( 6_i8 ).rem_euclid_unchecked( 3_i8 ) == 0_i8 );
static_assert( ( 6_i16 ).rem_euclid_unchecked( 3_i16 ) == 0_i16 );
static_assert( ( 6_i32 ).rem_euclid_unchecked( 3_i32 ) == 0_i32 );
static_assert( ( 6_i64 ).rem_euclid_unchecked( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_unchecked( 3_i8 ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_unchecked( 3_i16 ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_unchecked( 3_i32 ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_unchecked( 3_i64 ) == 0_i64 );

static_assert( ( -6_i8 ).rem_euclid_unchecked( ( -3_i8 ) ) == 0_i8 );
static_assert( ( -6_i16 ).rem_euclid_unchecked( ( -3_i16 ) ) == 0_i16 );
static_assert( ( -6_i32 ).rem_euclid_unchecked( ( -3_i32 ) ) == 0_i32 );
static_assert( ( -6_i64 ).rem_euclid_unchecked( ( -3_i64 ) ) == 0_i64 );

static_assert( ( 0_i8 ).rem_euclid_unchecked( ( -3_i8 ) ) == 0_i8 );
static_assert( ( 0_i16 ).rem_euclid_unchecked( ( -3_i16 ) ) == 0_i16 );
static_assert( ( 0_i32 ).rem_euclid_unchecked( ( -3_i32 ) ) == 0_i32 );
static_assert( ( 0_i64 ).rem_euclid_unchecked( ( -3_i64 ) ) == 0_i64 );

// Saturated boundary behavior
static_assert( daw::i8::min( ).div_euclid_saturated( -daw::i8{ 1 } ) ==
               daw::i8::max( ) );
static_assert( daw::i16::min( ).div_euclid_saturated( -daw::i16{ 1 } ) ==
               daw::i16::max( ) );
static_assert( daw::i32::min( ).div_euclid_saturated( -daw::i32{ 1 } ) ==
               daw::i32::max( ) );
static_assert( daw::i64::min( ).div_euclid_saturated( -daw::i64{ 1 } ) ==
               daw::i64::max( ) );

static_assert( ( -daw::i8{ 1 } ).rem_euclid_saturated( daw::i8::min( ) ) ==
               daw::i8::max( ) );
static_assert( ( -daw::i16{ 1 } ).rem_euclid_saturated( daw::i16::min( ) ) ==
               daw::i16::max( ) );
static_assert( ( -daw::i32{ 1 } ).rem_euclid_saturated( daw::i32::min( ) ) ==
               daw::i32::max( ) );
static_assert( ( -daw::i64{ 1 } ).rem_euclid_saturated( daw::i64::min( ) ) ==
               daw::i64::max( ) );

int main( ) {
	auto overflow_count = 0;
	auto divide_by_zero_count = 0;
	auto overflow_handler = [&](
	                          daw::integers::SignedIntegerErrorType error_type ) {
		daw_ensure( error_type == daw::integers::SignedIntegerErrorType::Overflow );
		++overflow_count;
	};
	auto divide_by_zero_handler =
	  [&]( daw::integers::SignedIntegerErrorType error_type ) {
		  daw_ensure( error_type ==
		              daw::integers::SignedIntegerErrorType::DivideByZero );
		  ++divide_by_zero_count;
	  };
	daw::integers::register_signed_overflow_handler( overflow_handler );
	daw::integers::register_signed_div_by_zero_handler( divide_by_zero_handler );

	(void)( 7_i32 ).div_euclid_checked( 0_i32 );
	daw_ensure( divide_by_zero_count == 1 );

	(void)( 7_i32 ).rem_euclid_checked( 0_i32 );
	daw_ensure( divide_by_zero_count == 2 );

	(void)daw::i32::min( ).div_euclid_checked( -1_i32 );
	daw_ensure( overflow_count == 1 );

	auto const remainder = daw::i32::min( ).rem_euclid_checked( -1_i32 );
	daw_ensure( remainder == 0_i32 );
	daw_ensure( overflow_count == 2 );
}
