// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

// The stream operators must only depend on the stream-like interface and not
// on <istream>/<ostream>.  Do not include any stream headers here, the
// operators are instantiated at the end of the TU and a later include of a
// stream header would hide a dependency on them
#include <daw/integers/daw_integer_iostream.h>

#include <daw/daw_ensure.h>

namespace {
	struct traits_like {
		using int_type = int;

		static constexpr int_type to_int_type( char c ) {
			return static_cast<int_type>( c );
		}

		static constexpr bool eq_int_type( int_type l, int_type r ) {
			return l == r;
		}
	};

	struct ios_like {
		using char_type = char;
		using traits_type = traits_like;
		using iostate = unsigned;
		static constexpr iostate failbit = 4U;
		static constexpr int adjustfield = 0;

		iostate state = 0U;

		[[maybe_unused]] char fill( ) const {
			return ' ';
		}

		[[maybe_unused]] bool good( ) const {
			return state == 0U;
		}

		[[maybe_unused]] int width( ) const {
			return 0;
		}

		[[maybe_unused]] int flags( ) const {
			return 0;
		}

		[[maybe_unused]] void setstate( iostate s ) {
			state |= s;
		}
	};

	struct ostream_like : ios_like {
		// The value written, after the operators promote it
		long long last = 0;

		[[maybe_unused]] ostream_like &write( char const *, int ) {
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( int v ) {
			last = v;
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( long v ) {
			last = v;
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( long long v ) {
			last = v;
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( unsigned v ) {
			last = static_cast<long long>( v );
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( unsigned long v ) {
			last = static_cast<long long>( v );
			return *this;
		}

		[[maybe_unused]] ostream_like &operator<<( unsigned long long v ) {
			last = static_cast<long long>( v );
			return *this;
		}
	};

	struct istream_like : ios_like {
		char next = ' ';
		long long value = 0;

		struct sentry {
			bool ok;

			[[maybe_unused]] explicit sentry( istream_like &is )
			  : ok( is.good( ) ) {}

			[[maybe_unused]] explicit operator bool( ) const {
				return ok;
			}
		};

		[[maybe_unused]] istream_like &read( char *, int ) {
			return *this;
		}

		[[maybe_unused]] int peek( ) const {
			return traits_type::to_int_type( next );
		}

		[[maybe_unused]] char widen( char c ) const {
			return c;
		}

		[[maybe_unused]] istream_like &operator>>( long long &v ) {
			v = value;
			return *this;
		}

		[[maybe_unused]] istream_like &operator>>( unsigned long long &v ) {
			v = static_cast<unsigned long long>( value );
			return *this;
		}
	};

	void test_write( ) {
		auto os = ostream_like{ };
		os << daw::i8{ -5 };
		daw_ensure( os.last == -5 );
		os << daw::u8{ 65 };
		daw_ensure( os.last == 65 );
		os << daw::u32{ 7U };
		daw_ensure( os.last == 7 );
		os << daw::i64::min( );
		daw_ensure( os.last == daw::i64::min( ).value( ) );
	}

	void test_read( ) {
		auto is = istream_like{ };
		is.value = 42;
		auto i = daw::i32{ };
		is >> i;
		daw_ensure( i == 42 and is.good( ) );

		is.value = 300;
		auto i8 = daw::i8{ };
		is >> i8;
		daw_ensure( i8 == daw::i8::max( ) and not is.good( ) );

		auto neg = istream_like{ };
		neg.next = '-';
		auto u = daw::u32{ 7U };
		neg >> u;
		daw_ensure( u == 0U and not neg.good( ) );
	}
} // namespace

int main( ) {
	test_write( );
	test_read( );
}
