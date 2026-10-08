// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_integer
//

#include <daw/daw_integer.h>

#include <daw/daw_ensure.h>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace {
	template<typename Integer>
	void test_hash( ) {
		using value_t = typename Integer::value_type;
		for( auto v : { value_t{ 0 }, value_t{ 1 }, value_t{ 42 },
		                Integer::max( ).value( ), Integer::min( ).value( ) } ) {
			daw_ensure( std::hash<Integer>{ }( Integer( v ) ) ==
			            std::hash<value_t>{ }( v ) );
		}
		auto s = std::unordered_set<Integer>{ Integer( 1 ), Integer( 2 ),
		                                      Integer( 1 ) };
		daw_ensure( s.size( ) == 2 );
		daw_ensure( s.contains( Integer( 2 ) ) );
		auto m = std::unordered_map<Integer, int>{ };
		m[Integer( 3 )] = 7;
		daw_ensure( m.at( Integer( 3 ) ) == 7 );
	}
} // namespace

int main( ) {
	test_hash<daw::i8>( );
	test_hash<daw::i16>( );
	test_hash<daw::i32>( );
	test_hash<daw::i64>( );
	test_hash<daw::u8>( );
	test_hash<daw::u16>( );
	test_hash<daw::u32>( );
	test_hash<daw::u64>( );
}
