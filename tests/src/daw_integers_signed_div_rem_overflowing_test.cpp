// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <daw/integers/daw_signed.h>

#include <daw/daw_ensure.h>

using namespace daw::integers::literals;

using daw::integers::SignedIntegerErrorType;

// div_overflowing: ordinary division
static_assert( ( 7_i8 ).div_overflowing( 3_i8 ).value == ( 2_i8 ).value( ) );
static_assert( ( 7_i8 ).div_overflowing( 3_i8 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i16 ).div_overflowing( 3_i16 ).value ==
               ( 2_i16 ).value( ) );
static_assert( ( 7_i16 ).div_overflowing( 3_i16 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i32 ).div_overflowing( 3_i32 ).value ==
               ( 2_i32 ).value( ) );
static_assert( ( 7_i32 ).div_overflowing( 3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i64 ).div_overflowing( 3_i64 ).value ==
               ( 2_i64 ).value( ) );
static_assert( ( 7_i64 ).div_overflowing( 3_i64 ).error ==
               SignedIntegerErrorType::None );

static_assert( ( -7_i8 ).div_overflowing( 3_i8 ).value == ( -2_i8 ).value( ) );
static_assert( ( -7_i8 ).div_overflowing( 3_i8 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i16 ).div_overflowing( 3_i16 ).value ==
               ( -2_i16 ).value( ) );
static_assert( ( -7_i16 ).div_overflowing( 3_i16 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i32 ).div_overflowing( 3_i32 ).value ==
               ( -2_i32 ).value( ) );
static_assert( ( -7_i32 ).div_overflowing( 3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i64 ).div_overflowing( 3_i64 ).value ==
               ( -2_i64 ).value( ) );
static_assert( ( -7_i64 ).div_overflowing( 3_i64 ).error ==
               SignedIntegerErrorType::None );

static_assert( ( 7_i32 ).div_overflowing( -3_i32 ).value ==
               ( -2_i32 ).value( ) );
static_assert( ( 7_i32 ).div_overflowing( -3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i32 ).div_overflowing( -3_i32 ).value ==
               ( 2_i32 ).value( ) );
static_assert( ( -7_i32 ).div_overflowing( -3_i32 ).error ==
               SignedIntegerErrorType::None );

// div_overflowing: overflow and division by zero
static_assert( daw::i8::min( ).div_overflowing( -1_i8 ).value ==
               daw::i8::min( ).value( ) );
static_assert( daw::i8::min( ).div_overflowing( -1_i8 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i16::min( ).div_overflowing( -1_i16 ).value ==
               daw::i16::min( ).value( ) );
static_assert( daw::i16::min( ).div_overflowing( -1_i16 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i32::min( ).div_overflowing( -1_i32 ).value ==
               daw::i32::min( ).value( ) );
static_assert( daw::i32::min( ).div_overflowing( -1_i32 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i64::min( ).div_overflowing( -1_i64 ).value ==
               daw::i64::min( ).value( ) );
static_assert( daw::i64::min( ).div_overflowing( -1_i64 ).error ==
               SignedIntegerErrorType::Overflow );

static_assert( ( 7_i8 ).div_overflowing( 0_i8 ).value == ( 7_i8 ).value( ) );
static_assert( ( 7_i8 ).div_overflowing( 0_i8 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i16 ).div_overflowing( 0_i16 ).value ==
               ( 7_i16 ).value( ) );
static_assert( ( 7_i16 ).div_overflowing( 0_i16 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i32 ).div_overflowing( 0_i32 ).value ==
               ( 7_i32 ).value( ) );
static_assert( ( 7_i32 ).div_overflowing( 0_i32 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i64 ).div_overflowing( 0_i64 ).value ==
               ( 7_i64 ).value( ) );
static_assert( ( 7_i64 ).div_overflowing( 0_i64 ).error ==
               SignedIntegerErrorType::DivideByZero );

// rem_overflowing: ordinary remainder
static_assert( ( 7_i8 ).rem_overflowing( 3_i8 ).value == ( 1_i8 ).value( ) );
static_assert( ( 7_i8 ).rem_overflowing( 3_i8 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i16 ).rem_overflowing( 3_i16 ).value ==
               ( 1_i16 ).value( ) );
static_assert( ( 7_i16 ).rem_overflowing( 3_i16 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i32 ).rem_overflowing( 3_i32 ).value ==
               ( 1_i32 ).value( ) );
static_assert( ( 7_i32 ).rem_overflowing( 3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( 7_i64 ).rem_overflowing( 3_i64 ).value ==
               ( 1_i64 ).value( ) );
static_assert( ( 7_i64 ).rem_overflowing( 3_i64 ).error ==
               SignedIntegerErrorType::None );

static_assert( ( -7_i8 ).rem_overflowing( 3_i8 ).value == ( -1_i8 ).value( ) );
static_assert( ( -7_i8 ).rem_overflowing( 3_i8 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i16 ).rem_overflowing( 3_i16 ).value ==
               ( -1_i16 ).value( ) );
static_assert( ( -7_i16 ).rem_overflowing( 3_i16 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i32 ).rem_overflowing( 3_i32 ).value ==
               ( -1_i32 ).value( ) );
static_assert( ( -7_i32 ).rem_overflowing( 3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i64 ).rem_overflowing( 3_i64 ).value ==
               ( -1_i64 ).value( ) );
static_assert( ( -7_i64 ).rem_overflowing( 3_i64 ).error ==
               SignedIntegerErrorType::None );

static_assert( ( 7_i32 ).rem_overflowing( -3_i32 ).value ==
               ( 1_i32 ).value( ) );
static_assert( ( 7_i32 ).rem_overflowing( -3_i32 ).error ==
               SignedIntegerErrorType::None );
static_assert( ( -7_i32 ).rem_overflowing( -3_i32 ).value ==
               ( -1_i32 ).value( ) );
static_assert( ( -7_i32 ).rem_overflowing( -3_i32 ).error ==
               SignedIntegerErrorType::None );

// rem_overflowing: overflow and division by zero
static_assert( daw::i8::min( ).rem_overflowing( -1_i8 ).value == 0 );
static_assert( daw::i8::min( ).rem_overflowing( -1_i8 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i16::min( ).rem_overflowing( -1_i16 ).value == 0 );
static_assert( daw::i16::min( ).rem_overflowing( -1_i16 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i32::min( ).rem_overflowing( -1_i32 ).value == 0 );
static_assert( daw::i32::min( ).rem_overflowing( -1_i32 ).error ==
               SignedIntegerErrorType::Overflow );
static_assert( daw::i64::min( ).rem_overflowing( -1_i64 ).value == 0 );
static_assert( daw::i64::min( ).rem_overflowing( -1_i64 ).error ==
               SignedIntegerErrorType::Overflow );

static_assert( ( 7_i8 ).rem_overflowing( 0_i8 ).value == ( 7_i8 ).value( ) );
static_assert( ( 7_i8 ).rem_overflowing( 0_i8 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i16 ).rem_overflowing( 0_i16 ).value ==
               ( 7_i16 ).value( ) );
static_assert( ( 7_i16 ).rem_overflowing( 0_i16 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i32 ).rem_overflowing( 0_i32 ).value ==
               ( 7_i32 ).value( ) );
static_assert( ( 7_i32 ).rem_overflowing( 0_i32 ).error ==
               SignedIntegerErrorType::DivideByZero );
static_assert( ( 7_i64 ).rem_overflowing( 0_i64 ).value ==
               ( 7_i64 ).value( ) );
static_assert( ( 7_i64 ).rem_overflowing( 0_i64 ).error ==
               SignedIntegerErrorType::DivideByZero );

int main( ) {
	auto handler_calls = 0;
	auto handler = [&]( daw::integers::SignedIntegerErrorType ) {
		++handler_calls;
	};
	daw::integers::register_signed_overflow_handler( handler );
	daw::integers::register_signed_div_by_zero_handler( handler );

	(void)daw::i32::min( ).div_overflowing( -1_i32 );
	(void)daw::i32::min( ).rem_overflowing( -1_i32 );
	(void)( 7_i32 ).div_overflowing( 0_i32 );
	(void)( 7_i32 ).rem_overflowing( 0_i32 );
	daw_ensure( handler_calls == 0 );
}
