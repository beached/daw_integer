# DAW Integer - Safer C++ Integers

[![License: Boost](https://img.shields.io/badge/License-Boost%201.0-blue.svg)](https://www.boost.org/LICENSE_1_0.txt)

## Content
  * [Intro](#intro)
  * [Code Examples](#code-examples)
  * [Headers](#headers)
  * [Constructing and Converting](#constructing-and-converting)
  * [Arithmetic and Comparisons](#arithmetic-and-comparisons)
  * [Overflow Behaviour - Choosing per Operation](#overflow-behaviour---choosing-per-operation)
  * [Division and Remainder](#division-and-remainder)
  * [Signed and Unsigned Conversions](#signed-and-unsigned-conversions)
  * [Bit Operations](#bit-operations)
  * [Standard Library Integration](#standard-library-integration)
  * [Error Handling](#error-handling)
  * [Checking Modes](#checking-modes)
  * [Installing/Using](#installingusing)
  * [Performance considerations](#performance-considerations)
  * [Build configuration points](#build-configuration-points)
  * [Requirements](#requirements)
  * [Limitations](#limitations)

## Intro
###### [Top](#content)

DAW Integer is a header only C++20 library of integer types that make overflow, division by zero, and out of range conversions explicit instead of silent or undefined:
  * `daw::i8`, `daw::i16`, `daw::i32`, `daw::i64` - signed integers
  * `daw::u8`, `daw::u16`, `daw::u32`, `daw::u64` - unsigned integers
  * The same size as the builtin integer they wrap, trivially copyable, and usable in constant expressions
  * In optimized builds using the unchecked mode, the generated code is the same as for builtin integers

The default operators are checked in debug builds and unchecked in release builds, similar to Rust's integers.  Each operation is also available with an explicit overflow behaviour, so the choice can be made where it matters:
  * `_checked` - report errors through a handler
  * `_wrapped` - wrap around on overflow
  * `_saturated` - clamp to `min( )`/`max( )`
  * `_overflowing` - return the wrapped value and an overflow flag
  * `_unchecked` - no checks, the caller guarantees the result fits
  * `try_` - return a `std::optional` that is empty on error

Some other notable features are:
  * No implicit narrowing and no mixing of signed with unsigned in arithmetic, both are compile errors
  * No integer promotion, `i8 + i8` is an `i8`
  * Comparisons with any integer type are value correct, `daw::u32{ 0 } > -1` is `true`
  * Explicit signed/unsigned conversions, `as_unsigned( )`, `as_signed( )`, and checked versions
  * Bit operations such as `count_ones`, `ilog2`, `rotate_left`, `swap_bytes`, and `next_power_of_two`
  * `std::numeric_limits`, `std::hash`, `std::format`, and iostream support
  * A replaceable error handler, defaulting to throwing an exception, or `std::terminate` when exceptions are disabled

The library is using the [BSL](LICENSE) license

The following shows the basics
```c++
#include <daw/daw_integer.h>

daw::i32 add( daw::i32 a, daw::i32 b ) {
  return a + b; // overflow is reported in checked builds
}

int main( ) {
  using namespace daw::integers::literals;
  auto total = add( 40_i32, 2_i32 );
  return total.value( ) == 42 ? 0 : 1;
}
```

## Code Examples
###### [Top](#content)

* The sections below have small, working examples
* [Tests](tests/src) provide another source of working code samples.  Some places to start
  * [daw_integers_unsigned_test.cpp](tests/src/daw_integers_unsigned_test.cpp) - a tour of the interface
  * [daw_integers_try_test.cpp](tests/src/daw_integers_try_test.cpp) - the `try_` operations
  * [daw_integers_signed_checking_modes_test.cpp](tests/src/daw_integers_signed_checking_modes_test.cpp) - default operators in each checking mode

## Headers
###### [Top](#content)

| Header | Provides |
|---|---|
| `<daw/daw_integer.h>` | Both signed and unsigned types. Use this one if unsure |
| `<daw/integers/daw_signed.h>` | `daw::i8` ... `daw::i64` |
| `<daw/integers/daw_unsigned.h>` | `daw::u8` ... `daw::u64` |
| `<daw/integers/daw_integer_format.h>` | `std::formatter` specializations |
| `<daw/integers/daw_integer_iostream.h>` | `operator<<` and `operator>>` |

`std::numeric_limits` and `std::hash` come with the type headers.  Members that return the other signedness, such as `as_unsigned( )`, need both type headers, which `<daw/daw_integer.h>` provides.

## Constructing and Converting
###### [Top](#content)

Construction is explicit.  Values from builtin integers are range checked, and widening between the library types is implicit.
```c++
using namespace daw::integers::literals;

auto a = daw::i32{ 42 };
auto b = 42_i32;                                // literals _i8 ... _i64, _u8 ... _u64
auto c = daw::u8{ 200U };
daw::i64 d = a;                                 // widening is implicit
auto e = daw::i16( d );                         // narrowing is explicit
auto f = daw::u32::conversion_checked( -1 );    // out of range, reported to the handler
auto g = daw::u32::try_from( -1 );              // std::nullopt
auto h = daw::u8::conversion_unchecked( 300 );  // truncates like static_cast, 44
int i = static_cast<int>( a );                  // or a.value( )
```
Literals are checked at compile time, `300_u8` does not compile.

## Arithmetic and Comparisons
###### [Top](#content)

All the arithmetic, bitwise, shift, increment, and compound assignment operators are supported.  Mixing widths results in the wider type, and builtin integers of the same signedness can be used directly.
```c++
using namespace daw::integers::literals;

auto x = 5_i32 + 3_i32;   // i32
auto y = 5_i32 * 2_i64;   // i64, the wider of the two
auto z = 5_u32 + 1U;      // builtin of the same signedness
auto w = 1_i8 + 1_i8;     // i8, there is no promotion to int
```
Mixing signed and unsigned in arithmetic is a compile error, convert explicitly with `as_signed( )`/`as_unsigned( )`.  Unlike the builtin operators, comparisons with any integer type compare the values
```c++
static_assert( 5_u32 > -1 );   // false for builtin 5U > -1
```

## Overflow Behaviour - Choosing per Operation
###### [Top](#content)

The default operators follow the [checking mode](#checking-modes).  When the behaviour matters, choose it for the operation
```c++
auto const m = daw::i32::max( );
auto const one = daw::i32{ 1 };

m.add_checked( one );     // reports overflow, then returns the wrapped value
m.add_wrapped( one );     // daw::i32::min( )
m.add_saturated( one );   // daw::i32::max( )
m.add_unchecked( one );   // no check, the caller guarantees it fits
m.try_add( one );         // std::optional<daw::i32>, empty here

auto [value, overflowed] = m.add_overflowing( one ); // value is the wrapped result
```

| Operation | `_checked` | `_wrapped` | `_saturated` | `_unchecked` | `_overflowing` | `try_` |
|---|:-:|:-:|:-:|:-:|:-:|:-:|
| `add`, `sub`, `mul` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `div`, `rem` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `div_euclid` | ✓ | ✓ | ✓ | | | ✓ |
| `rem_euclid` | ✓ | | ✓ | ✓ | | ✓ |
| `pow( unsigned )` | ✓ | ✓ | ✓ | ✓ | | ✓ |
| `shl`, `shr` | ✓ | | | ✓ | ✓ | ✓ |
| `negate` | ✓ | ✓ | signed | signed | | ✓ |
| `abs` (signed) | ✓ | ✓ | ✓ | | | ✓ |

`shl_overflowing`/`shr_overflowing` take the shift count modulo the bit width.  `try_` operations never call the error handler.

## Division and Remainder
###### [Top](#content)

Division by zero is reported to the div by zero handler in checked mode, and `min( ) / -1` is reported as an overflow for signed types.
```c++
daw::i32{ 7 }.try_div( daw::i32{ 0 } );           // std::nullopt
daw::i32{ -7 }.div_euclid( daw::i32{ 2 } );       // -4
daw::i32{ -7 }.rem_euclid( daw::i32{ 2 } );       // 1

auto r = daw::i32{ 7 }.div_overflowing( daw::i32{ 0 } );
// r.error == daw::integers::IntegerErrorType::DivideByZero
```

## Signed and Unsigned Conversions
###### [Top](#content)

| Member | Result | When the value doesn't fit |
|---|---|---|
| `as_unsigned( )`/`as_signed( )` | same width, other signedness | keeps the bits, like `static_cast` |
| `as_exact_unsigned( )`/`as_exact_signed( )` | same width, other signedness | reports overflow, then keeps the bits |
| `try_as_unsigned( )`/`try_as_signed( )` | `std::optional` | `std::nullopt` |
| `unsigned_abs( )` | absolute value as unsigned | always fits |
| `abs_diff( rhs )` | `\|a - b\|` as unsigned | always fits |

```c++
daw::i8{ -1 }.as_unsigned( );                   // 255
daw::u8{ 255 }.as_signed( );                    // -1
daw::i8{ -1 }.try_as_unsigned( );               // std::nullopt
daw::i8::min( ).unsigned_abs( );                // 128
daw::i32{ -5 }.abs_diff( daw::i32{ 5 } );       // 10
```

## Bit Operations
###### [Top](#content)

* `count_ones( )`, `count_zeros( )`
* `count_leading_zeros( )`, `count_trailing_zeros( )`, `count_leading_ones( )`, `count_trailing_ones( )`
* `ilog2( )` and `try_ilog2( )`
* `rotate_left( n )`, `rotate_right( n )`
* `reverse_bits( )`, `swap_bytes( )`
* Unsigned only: `is_power_of_two( )`, `next_power_of_two( )`, `try_next_power_of_two( )`
* `from_bytes_le( ptr )`, `from_bytes_be( ptr )`

```c++
daw::u32{ 0b1011U }.count_ones( );               // 3
daw::u32{ 1024U }.ilog2( );                      // 10
daw::u32{ 0x1234'5678U }.swap_bytes( );          // 0x7856'3412
daw::u32{ 65U }.next_power_of_two( );            // 128
```
Bit operations on signed types work on the two's complement representation.

## Standard Library Integration
###### [Top](#content)

```c++
#include <daw/daw_integer.h>
#include <daw/integers/daw_integer_format.h>
#include <daw/integers/daw_integer_iostream.h>

using namespace daw::integers::literals;

auto s = std::format( "{} {:#x} {}", -5_i32, 255_u32, 65_i8 );  // "-5 0xff 65"
std::cout << 200_u8 << '\n';                                     // 200
std::unordered_set<daw::i64> set{ 1_i64, 2_i64 };
static_assert( std::numeric_limits<daw::u16>::max( ) == 65535U );
```
* `i8` and `u8` format and stream as numbers, not characters
* All of the format specs of the underlying type are supported
* `operator>>` behaves like the builtin extractors, values out of range set `failbit` and store the closest limit.  Unsigned extraction rejects a leading `-` instead of wrapping

## Error Handling
###### [Top](#content)

### Exceptions
Errors default to throwing `daw::integers::integer_overflow_exception` or `daw::integers::integer_div_by_zero_exception`.

### -fno-exceptions
If exceptions are disabled the library will call `std::terminate` on errors by default.

### Custom Error Handling
A handler can be registered for overflow and for division by zero.  It can be a function pointer and a user data pointer, or a reference to a callable that outlives its registration.
```c++
auto overflows = 0;
auto handler = [&]( daw::integers::IntegerErrorType ) {
  ++overflows;
};
daw::integers::register_integer_overflow_handler( handler );
daw::integers::register_integer_div_by_zero_handler( handler );
```
Unlike exceptions, a handler may return.  The operation then continues with a defined result, usually the wrapped value.  Passing `nullptr` restores the default.

The handlers are global and shared by the signed and unsigned types.  Registering is not thread safe, so register at startup before other threads use the types.  The `register_signed_*` names and `Signed*` types from earlier versions are still available as aliases.

## Checking Modes
###### [Top](#content)

The behaviour of the default operators(`+`, `-`, `*`, `/`, `%`, `<<`, `>>`, `++`, `--`, unary `-`, and narrowing construction) is set by `DAW_DEFAULT_SIGNED_CHECKING` and `DAW_DEFAULT_UNSIGNED_CHECKING`
* `0` - Checked.  Errors call the handler, overflowing results wrap.  The default when `DEBUG` is defined or `NDEBUG` is not
* `1` - Unchecked.  The default otherwise.  Add, subtract, and multiply wrap
* `2` - Wrapped for add, subtract, and multiply, unchecked for the rest

The `_checked`, `_overflowing`, and `try_` operations, as well as add, subtract, and multiply with `_wrapped`/`_saturated`/`_unchecked`, behave the same in every mode.  The `_wrapped` and `_saturated` division and remainder operations use the default checking for division by zero.  Every translation unit in a program should use the same mode.

## Installing/Using
###### [Top](#content)

### Using in cmake
```cmake
find_package( daw-integer )
#...
target_link_libraries( MyTarget daw::daw-integer )
```

### As header only
The library is header only and can be cloned, along with its dependency [header_libraries](https://github.com/beached/header_libraries), followed by adding the `include/` subfolders of each to the compiler's include path

### Including in cmake project via FetchContent
To use daw_integer in your cmake projects, adding the following should allow it to pull it in along with the dependencies:
```cmake
include( FetchContent )
FetchContent_Declare(
  daw_integer
  GIT_REPOSITORY https://github.com/beached/daw_integer
  GIT_TAG release
)
FetchContent_MakeAvailable( daw_integer )
#...
target_link_libraries( MyTarget daw::daw-integer )
```
When the dependencies are already installed, setting `DAW_USE_PACKAGE_MANAGEMENT` to `ON` uses `find_package` for them instead of FetchContent.

### Installing
On a system with bash, it is similar on other systems too, the following can install for the system
```bash
git clone https://github.com/beached/daw_integer
cd daw_integer
mkdir build
cd build
cmake ..
cmake --install .
```

### Testing
The following will build and run the tests.
```bash
git clone https://github.com/beached/daw_integer
cd daw_integer
mkdir build
cd build
cmake -DDAW_ENABLE_TESTING=On ..
cmake --build .
ctest .
```

## Performance considerations
###### [Top](#content)

With optimizations on and the unchecked mode, the operators generate the same code as builtin integers.  The checked mode adds a branch per operation that can overflow.  Some things to be aware of
* Signed overflow is defined to wrap in the unchecked mode.  The compiler cannot assume it does not happen, so a few loop optimizations available to builtin `int` are not done
* Without optimizations, each operation is a few function calls deep.  Debug builds will be slower than with builtin integers

## Build configuration points
###### [Top](#content)

There are a few defines that affect how DAW Integer operates
* `DAW_DEFAULT_SIGNED_CHECKING` - Checking mode of the signed default operators, see [Checking Modes](#checking-modes)
* `DAW_DEFAULT_UNSIGNED_CHECKING` - Checking mode of the unsigned default operators
* Exceptions are not used when they are disabled(e.g. `-fno-exceptions`), errors without a handler call `std::terminate`

## Requirements
###### [Top](#content)

* C++20 compiler
* [header_libraries](https://github.com/beached/header_libraries), fetched automatically by cmake
* GCC 16 has been tested

### For building tests
  * git
  * cmake 3.20 or later
  * C++20 compiler

## Limitations
###### [Top](#content)

* In the unchecked mode, division by zero, `min( ) / -1` and shifts by a negative or too large count are undefined behaviour, the same as for builtin integers.  This includes `div_wrapped`, `div_saturated`, `rem_wrapped` and `rem_saturated` with a zero divisor.  Use the `_checked` or `try_` operations when they are possible
* The types are not `std::integral`, so library functions constrained on it, such as `std::to_chars`, `std::cmp_less`, and `std::views::iota`, need `.value( )`
* There are no implicit conversions to builtin integers.  Indexing and `switch` need `.value( )`
* Compound assignment and arithmetic with `int` literals does not compile for `i8` and `i16`, as `int` is wider.  Use library values, e.g. `x += daw::i8{ 1 }`
* The minimum value cannot be written as a negated literal, `-128_i8` does not compile.  Use `daw::i8::min( )`
* Each translation unit chooses its checking mode, mixing modes is an ODR violation
