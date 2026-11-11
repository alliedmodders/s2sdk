#include "tier0/bitbuf.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_nFailures = 0;

#define CHECK( expr ) \
	do \
	{ \
		if ( !( expr ) ) \
		{ \
			printf( "%s:%d: CHECK( %s ) failed\n", __FILE__, __LINE__, #expr ); \
			++g_nFailures; \
		} \
	} while ( 0 )

static const uint32 s_VarInts32[] = { 0, 1, 127, 128, 16383, 16384, 0x7FFFFFFF, 0xFFFFFFFF };
static const uint64 s_VarInts64[] = { 0, 1, 127, 128, 0xFFFFFFFFull, 1ull << 35, 0x7FFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull };
static const int64 s_SignedVarInts[] = { 0, -1, 1, -64, 64, -0x80000000LL, 0x7FFFFFFFLL, -0x7FFFFFFFFFFFFFFFLL - 1, 0x7FFFFFFFFFFFFFFFLL };
static const uint32 s_UBitVars[] = { 0, 15, 16, 255, 256, 4095, 4096, 0xFFFFFFFF };

// Every value kind reads back as written, from every starting bit, with byte counts that do and don't allow
// whole dword reads
static void TestRoundTrip()
{
	for ( int nStart = 0; nStart < 40; nStart++ )
	{
		for ( int nPad = 0; nPad < 4; nPad++ )
		{
			uint32 data[64] = {};
			bf_write write( data, sizeof( data ) );
			write.SeekToBit( nStart );
			write.WriteOneBit( 1 );
			write.WriteUBitLong( 5, 3 );
			write.WriteUBitLong( 0xFFFFFFFF, 32 );
			write.WriteSBitLong( -1234, 13 );
			write.WriteBitLong( 77, 7, false );
			write.WriteBitLong( (unsigned int)-3, 5, true );
			for ( uint32 n : s_UBitVars )
				write.WriteUBitVar( n );
			for ( uint32 n : s_VarInts32 )
				write.WriteVarInt32( n );
			for ( uint64 n : s_VarInts64 )
				write.WriteVarInt64( n );
			for ( int64 n : s_SignedVarInts )
			{
				write.WriteSignedVarInt32( (int32)n );
				write.WriteSignedVarInt64( n );
			}
			write.WriteChar( -7 );
			write.WriteByte( 200 );
			write.WriteShort( -30000 );
			write.WriteWord( 60000 );
			write.WriteLong( -2000000000 );
			write.WriteLongLong( -1234567890123456789LL );
			write.WriteFloat( -1.5f );
			write.WriteBitFloat( 3.25f );
			write.WriteString( "hello" );
			write.WriteBytes( "raw", 3 );
			CHECK( !write.IsOverflowed() );

			bf_read read( data, write.GetNumBytesWritten() + nPad );
			read.Seek( nStart );
			CHECK( read.ReadOneBit() == 1 );
			CHECK( read.ReadUBitLong( 3 ) == 5 );
			CHECK( read.ReadUBitLong( 32 ) == 0xFFFFFFFF );
			CHECK( read.ReadSBitLong( 13 ) == -1234 );
			CHECK( read.ReadBitLong( 7, false ) == 77 );
			CHECK( (int)read.ReadBitLong( 5, true ) == -3 );
			for ( uint32 n : s_UBitVars )
				CHECK( read.ReadUBitVar() == n );
			for ( uint32 n : s_VarInts32 )
				CHECK( read.ReadVarInt32() == n );
			for ( uint64 n : s_VarInts64 )
				CHECK( read.ReadVarInt64() == n );
			for ( int64 n : s_SignedVarInts )
			{
				CHECK( read.ReadSignedVarInt32() == (int32)n );
				CHECK( read.ReadSignedVarInt64() == n );
			}
			CHECK( read.ReadChar() == -7 );
			CHECK( read.ReadByte() == 200 );
			CHECK( read.ReadShort() == -30000 );
			CHECK( read.ReadWord() == 60000 );
			CHECK( read.ReadLong() == -2000000000 );
			CHECK( read.ReadLongLong() == -1234567890123456789LL );
			CHECK( read.ReadFloat() == -1.5f );
			CHECK( read.ReadBitFloat() == 3.25f );
			char sz[8];
			CHECK( read.ReadString( sz, sizeof( sz ) ) && strcmp( sz, "hello" ) == 0 );
			char raw[3];
			CHECK( read.ReadBytes( raw, 3 ) && memcmp( raw, "raw", 3 ) == 0 );
			CHECK( !read.IsOverflowed() );
			CHECK( read.GetNumBitsRead() == write.GetNumBitsWritten() );
		}
	}
}

static bool GetBit( const void *p, int nBit )
{
	return ( ( (const uint8 *)p )[nBit >> 3] >> ( nBit & 7 ) ) & 1;
}

// Bit blocks copy at any alignment of the bits and of the memory on either side, and leave the bits around them alone
static void TestBitBlocks()
{
	uint8 src[72];
	for ( int i = 0; i < (int)sizeof( src ); i++ )
		src[i] = (uint8)( i * 37 + 11 );

	static const int s_Lengths[] = { 0, 1, 7, 8, 9, 31, 32, 33, 64, 100, 257, 512 };
	for ( int nDstBit = 0; nDstBit < 34; nDstBit++ )
	{
		for ( int nOffset = 0; nOffset < 4; nOffset++ )
		{
			for ( int nBits : s_Lengths )
			{
				uint32 data[32];
				memset( data, 0xFF, sizeof( data ) );
				bf_write write( data, sizeof( data ) );
				write.SeekToBit( nDstBit );
				CHECK( write.WriteBits( src + nOffset, nBits ) );
				CHECK( write.GetNumBitsWritten() == nDstBit + nBits );

				bool bSame = true;
				for ( int i = 0; i < nDstBit; i++ )
					bSame = bSame && GetBit( data, i );
				for ( int i = 0; i < nBits; i++ )
					bSame = bSame && GetBit( data, nDstBit + i ) == GetBit( src + nOffset, i );
				for ( int i = nDstBit + nBits; i < (int)sizeof( data ) * 8; i++ )
					bSame = bSame && GetBit( data, i );
				CHECK( bSame );

				uint8 out[72];
				memset( out, 0, sizeof( out ) );
				bf_read read( data, sizeof( data ) );
				read.Seek( nDstBit );
				read.ReadBits( out + nOffset, nBits );
				bSame = true;
				for ( int i = 0; i < nBits; i++ )
					bSame = bSame && GetBit( out + nOffset, i ) == GetBit( src + nOffset, i );
				CHECK( bSame && read.GetNumBitsRead() == nDstBit + nBits );

				uint32 copy[32] = {};
				bf_write dst( copy, sizeof( copy ) );
				dst.SeekToBit( nOffset * 3 );
				read.Seek( nDstBit );
				if ( nBits )
					CHECK( dst.WriteBitsFromBuffer( &read, nBits ) );
				bSame = true;
				for ( int i = 0; i < nBits; i++ )
					bSame = bSame && GetBit( copy, nOffset * 3 + i ) == GetBit( src + nOffset, i );
				CHECK( bSame );
			}
		}
	}
}

// Coords, normals and angles come back within their encodings' resolution
static void TestEncodedFloats()
{
	static const float s_Coords[] = { 0.0f, 0.03125f, -0.03125f, 0.5f, 1.5f, -2.75f, 100.25f, -2047.9f, 2048.0f, 15000.5f, -16000.25f };
	for ( float f : s_Coords )
	{
		uint32 data[16] = {};
		bf_write write( data, sizeof( data ) );
		write.WriteBitCoord( f );
		write.WriteBitCoordMP( f, false, false );
		write.WriteBitCoordMP( f, false, true );
		write.WriteBitCoordMP( f, true, false );
		CHECK( !write.IsOverflowed() );

		bf_read read( data, sizeof( data ) );
		CHECK( fabsf( read.ReadBitCoord() - f ) < 1.0f / 32 );
		CHECK( fabsf( read.ReadBitCoordMP( false, false ) - f ) < 1.0f / 32 );
		CHECK( fabsf( read.ReadBitCoordMP( false, true ) - f ) < 1.0f / 8 );
		CHECK( read.ReadBitCoordMP( true, false ) == (float)(int)f );
		CHECK( read.GetNumBitsRead() == write.GetNumBitsWritten() );

		// The bit forms read the same number of bits
		bf_read bits( data, sizeof( data ) );
		bits.ReadBitCoordBits();
		bits.ReadBitCoordMPBits( false, false );
		bits.ReadBitCoordMPBits( false, true );
		bits.ReadBitCoordMPBits( true, false );
		CHECK( bits.GetNumBitsRead() == write.GetNumBitsWritten() );
	}

	for ( float f = -1.0f; f <= 1.0f; f += 0.0625f )
	{
		uint32 data[4] = {};
		bf_write write( data, sizeof( data ) );
		write.WriteBitNormal( f );
		bf_read read( data, sizeof( data ) );
		CHECK( fabsf( read.ReadBitNormal() - f ) < 1.0f / 2047 );
	}

	{
		uint32 data[16] = {};
		bf_write write( data, sizeof( data ) );
		Vector coord( 1.5f, 0.0f, -300.25f );
		Vector normal( 0.6f, -0.48f, -0.64f );
		QAngle angles( 10.5f, -90.0f, 179.75f );
		write.WriteBitVec3Coord( coord );
		write.WriteBitVec3Normal( normal );
		write.WriteBitAngles( angles );

		bf_read read( data, sizeof( data ) );
		Vector coordRead, normalRead;
		QAngle anglesRead;
		read.ReadBitVec3Coord( coordRead );
		read.ReadBitVec3Normal( normalRead );
		read.ReadBitAngles( anglesRead );
		for ( int i = 0; i < 3; i++ )
		{
			CHECK( fabsf( coordRead[i] - coord[i] ) < 1.0f / 32 );
			CHECK( fabsf( normalRead[i] - normal[i] ) < 0.002f );
			CHECK( fabsf( anglesRead[i] - angles[i] ) < 1.0f / 32 );
		}
		CHECK( read.GetNumBitsRead() == write.GetNumBitsWritten() );
	}

	for ( int nBits = 8; nBits <= 30; nBits++ )
	{
		for ( float f = 0.0f; f < 360.0f; f += 17.3f )
		{
			uint32 data[4] = {};
			bf_write write( data, sizeof( data ) );
			write.WriteBitAngle( f, nBits );
			bf_read read( data, sizeof( data ) );
			float fRead = read.ReadBitAngle( nBits );
			CHECK( fRead <= f + 1e-4f && f - fRead < 360.0f / ( 1 << nBits ) + 1e-4f );
		}
	}
}

// Reads past the end return 0, set the overflow flag and stop at the end, except for peeks and ReadNBits, which keep
// the position; writes past the end write nothing
static void TestOverflow()
{
	uint32 data[2] = { 0xFFFFFFFF, 0xFFFFFFFF };

	bf_read read( data, sizeof( data ), 10 );
	read.SetAssertOnOverflow( false );
	CHECK( read.ReadUBitLong( 8 ) == 0xFF );
	CHECK( read.PeekUBitLong( 2 ) == 3 && read.GetNumBitsRead() == 8 );
	CHECK( read.ReadNBits<3>() == 0 && read.IsOverflowed() && read.GetNumBitsRead() == 8 );
	CHECK( read.ReadNBits<2>() == 3 && read.GetNumBitsRead() == 10 );
	CHECK( read.ReadOneBit() == 0 && read.GetNumBitsLeft() == 0 );

	read.Reset();
	CHECK( !read.IsOverflowed() && read.GetNumBitsRead() == 0 );
	CHECK( read.Seek( 8 ) && read.ReadUBitLong( 3 ) == 0 && read.IsOverflowed() && read.GetNumBitsRead() == 10 );
	read.Reset();
	CHECK( !read.Seek( 11 ) && read.IsOverflowed() && read.GetNumBitsRead() == 10 );
	read.Reset();
	CHECK( !read.SeekRelative( -1 ) && read.IsOverflowed() );
	read.Reset();
	read.Seek( 9 );
	CHECK( read.ReadBitCoordBits() == 0 && read.IsOverflowed() && read.GetNumBitsRead() == 9 );

	char sz[8];
	bf_read unterminated( data, sizeof( data ) );
	unterminated.SetAssertOnOverflow( false );
	CHECK( !unterminated.ReadString( sz, sizeof( sz ) ) && unterminated.IsOverflowed() );

	uint32 out[2] = {};
	bf_write write( out, sizeof( out ), 40 );
	write.SetAssertOnOverflow( false );
	write.WriteUBitLong( 0x12345678, 32 );
	write.WriteUBitLong( 0xFF, 9 );
	CHECK( write.IsOverflowed() && write.GetNumBitsWritten() == 40 && out[1] == 0 );
	write.Reset();
	CHECK( !write.IsOverflowed() && write.GetNumBitsWritten() == 0 );
	CHECK( !write.WriteBits( data, 41 ) && write.IsOverflowed() && write.GetNumBitsWritten() == 0 );
	write.Reset();
	write.SeekToBit( 39 );
	write.WriteOneBit( 1 );
	write.WriteOneBit( 1 );
	CHECK( write.IsOverflowed() && write.GetNumBitsWritten() == 40 && out[1] == 0x80 );
}

// Strings read whole or by line, into a char buffer or a CBufferString; what doesn't fit is cut but still read past
static void TestStrings()
{
	static const char szLong[] = "a string longer than the buffers below";

	uint32 data[32] = {};
	bf_write write( data, sizeof( data ) );
	write.WriteString( "line one\nline two" );
	write.WriteString( "" );
	write.WriteString( szLong );
	write.WriteByte( 42 );
	write.WriteString( "alloc" );
	CHECK( !write.IsOverflowed() );

	bf_read chars( data, write.GetNumBytesWritten() );
	char sz[64], small[8];
	int nChars = -1;
	CHECK( chars.ReadString( sz, sizeof( sz ), true, &nChars ) && strcmp( sz, "line one" ) == 0 && nChars == 8 );
	CHECK( chars.ReadString( sz, sizeof( sz ), false, &nChars ) && strcmp( sz, "line two" ) == 0 && nChars == 8 );
	CHECK( chars.ReadString( sz, sizeof( sz ), false, &nChars ) && sz[0] == 0 && nChars == 0 );
	CHECK( !chars.ReadString( small, sizeof( small ), false, &nChars ) && nChars == 7 && memcmp( small, szLong, 7 ) == 0 && small[7] == 0 );
	CHECK( chars.ReadByte() == 42 );
	bool bOverflow = true;
	char *pAlloc = chars.ReadAndAllocateString( &bOverflow );
	CHECK( !bOverflow && strcmp( pAlloc, "alloc" ) == 0 );
	delete[] pAlloc;
	CHECK( !chars.IsOverflowed() );

	bf_read strings( data, write.GetNumBytesWritten() );
	CBufferString str;
	CHECK( strings.ReadString( &str, true, &nChars ) && strcmp( str.Get(), "line one" ) == 0 && nChars == 8 );
	CHECK( strings.ReadString( &str, false, &nChars ) && strcmp( str.Get(), "line two" ) == 0 );
	CHECK( strings.ReadString( &str ) && str.Length() == 0 );
	CBufferStringN<16> fixed( false );
	CHECK( !strings.ReadString( &fixed, false, &nChars ) && nChars == fixed.AllocatedNum() - 1 );
	CHECK( strncmp( fixed.Get(), szLong, nChars ) == 0 && fixed.IsStackAllocated() );
	CHECK( strings.ReadByte() == 42 );
	CHECK( strings.ReadString( &str ) && strcmp( str.Get(), "alloc" ) == 0 );
}

// Bit range comparisons and removal
static void TestBitRanges()
{
	uint32 a[8] = {}, b[8] = {};
	bf_write writeA( a, sizeof( a ) ), writeB( b, sizeof( b ) );
	writeA.SeekToBit( 5 );
	writeB.SeekToBit( 19 );
	for ( int i = 0; i < 100; i++ )
	{
		writeA.WriteOneBit( ( i * 7 ) % 3 == 0 );
		writeB.WriteOneBit( ( i * 7 ) % 3 == 0 );
	}

	bf_read readA( a, sizeof( a ) ), readB( b, sizeof( b ) );
	CHECK( readA.CompareBitsAt( 5, &readB, 19, 100 ) == 0 );
	CHECK( readA.CompareBitsAt( 5, &readB, 18, 100 ) != 0 );
	readA.Seek( 5 );
	readB.Seek( 19 );
	CHECK( readA.CompareBits( &readB, 20 ) == 0 );

	uint32 bits[4] = {};
	bf_write write( bits, sizeof( bits ) );
	write.WriteUBitLong( 0x3F, 6 );
	write.WriteUBitLong( 0, 10 );
	write.WriteUBitLong( 0x2A, 6 );
	bf_read excise( bits, sizeof( bits ), 22 );
	excise.ExciseBits( 6, 10 );
	CHECK( excise.GetNumBitsRead() == 6 && excise.GetNumBitsLeft() == 6 );
	excise.Seek( 0 );
	CHECK( excise.ReadUBitLong( 6 ) == 0x3F && excise.ReadUBitLong( 6 ) == 0x2A );
}

int main()
{
	TestRoundTrip();
	TestBitBlocks();
	TestEncodedFloats();
	TestOverflow();
	TestStrings();
	TestBitRanges();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
