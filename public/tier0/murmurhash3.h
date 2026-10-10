//======= Copyright © Valve Corporation, All rights reserved. =================
//
// Public domain MurmurHash3 by Austin Appleby is a very solid general-purpose
// hash with a 32-bit output. References:
// http://code.google.com/p/smhasher/ (home of MurmurHash3)
// https://sites.google.com/site/murmurhash/avalanche
// http://www.strchr.com/hash_functions
//
//=============================================================================

#ifndef MURMURHASH3_H
#define MURMURHASH3_H

#if defined(_WIN32)
#pragma once
#endif

#include <string.h>
#include "tier0/platform.h"

inline uint32 MurmurHash3_32( const void *key, size_t len, uint32 seed, bool bCaselessStringVariant = false )
{
	const uint8 *data = (const uint8 *)key;
	const size_t nblocks = len / 4;
	uint32 uSourceBitwiseAndMask = 0xDFDFDFDF | ((uint32)bCaselessStringVariant - 1);

	uint32 h1 = seed;

	//----------
	// body

	for ( size_t i = 0; i < nblocks; i++ )
	{
		uint32 k1;
		memcpy( &k1, data + i * 4, sizeof( k1 ) );
		k1 = LittleDWord( k1 );
		k1 &= uSourceBitwiseAndMask;

		k1 *= 0xcc9e2d51;
		k1 = (k1 << 15) | (k1 >> 17);
		k1 *= 0x1b873593;

		h1 ^= k1;
		h1 = (h1 << 13) | (h1 >> 19);
		h1 = h1*5+0xe6546b64;
	}

	//----------
	// tail

	const uint8 *tail = data + nblocks*4;

	uint32 k1 = 0;

	switch ( len & 3 )
	{
	case 3: k1 ^= tail[2] << 16; [[fallthrough]];
	case 2: k1 ^= tail[1] << 8; [[fallthrough]];
	case 1: k1 ^= tail[0];
		k1 &= uSourceBitwiseAndMask;
		k1 *= 0xcc9e2d51;
		k1 = (k1 << 15) | (k1 >> 17);
		k1 *= 0x1b873593;
		h1 ^= k1;
	}

	//----------
	// finalization

	h1 ^= (uint32)len;

	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6b;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;

	return h1;
}

inline uint32 MurmurHash3String( const char *pszKey, size_t len )
{
	return MurmurHash3_32( pszKey, len, 1047 /*anything will do for a seed*/, false );
}

inline uint32 MurmurHash3StringCaseless( const char *pszKey, size_t len )
{
	return MurmurHash3_32( pszKey, len, 1047 /*anything will do for a seed*/, true );
}

inline uint32 MurmurHash3String( const char *pszKey )
{
	return MurmurHash3String( pszKey, strlen( pszKey ) );
}

inline uint32 MurmurHash3StringCaseless( const char *pszKey )
{
	return MurmurHash3StringCaseless( pszKey, strlen( pszKey ) );
}

template <typename T>
inline uint32 MurmurHash3Item( const T &item )
{
	return MurmurHash3_32( &item, sizeof(item), 1047 );
}

inline uint32 MurmurHash3Int( uint32 h )
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

inline uint32 MurmurHash3Int64( uint64 h )
{
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdull;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53ull;
	h ^= h >> 33;
	return (uint32)h;
}

template <>
inline uint32 MurmurHash3Item( const uint32 &item )
{
	return MurmurHash3Int( item );
}

template <>
inline uint32 MurmurHash3Item( const int32 &item )
{
	return MurmurHash3Int( item );
}


template<typename T>
struct MurmurHash3Functor
{
	typedef uint32 TargetType;
	TargetType operator()(const T &key) const
	{
		return MurmurHash3Item( key );
	}
};

template<>
struct MurmurHash3Functor<char *>
{
	typedef uint32 TargetType;
	TargetType operator()(const char *key) const
	{
		return MurmurHash3String( key );
	}
};

template<>
struct MurmurHash3Functor<const char *>
{
	typedef uint32 TargetType;
	TargetType operator()(const char *key) const
	{
		return MurmurHash3String( key );
	}
};

#endif	// MURMURHASH3_H
