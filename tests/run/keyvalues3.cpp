#include "tier0/keyvalues3.h"
#include "tier0/utlbuffer.h"
#include "tier0/utlstring.h"
#include "tier0/utlleanvector.h"

#include <stdio.h>
#include <stdlib.h>
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

template <size_t N>
static void CheckStringHash( const char (&key)[N] )
{
	const char *pointer = key;
	const int len = (int)strlen( pointer );
	const uint32 seed = STRINGTOKEN_MURMURHASH_SEED;
	CHECK( MurmurHash2( key, seed ) == MurmurHash2( pointer, len, seed ) );
	CHECK( MurmurHash2LowerCase( key, seed ) == MurmurHash2LowerCase( pointer, seed ) );
	CHECK( CUtlStringToken( key ) == CUtlStringToken( pointer ) );
	CHECK( CKV3MemberName( key ).GetHashCode() == CKV3MemberName( pointer ).GetHashCode() );
}

static void TestStringHashes( KeyValues3 *root )
{
	constexpr char padded[32] = "WeApOn0";
	static_assert( MurmurHash2( padded, 123 ) == MurmurHash2( "WeApOn0", 123 ) );
	static_assert( MurmurHash2LowerCase( padded, 123 ) == MurmurHash2LowerCase( "weapon0", 123 ) );
	constexpr CUtlStringToken token( padded );
	CHECK( token == CUtlStringToken( "weapon0" ) );
	CheckStringHash( "weapon0" );
	CheckStringHash( padded );
	CheckStringHash( "" );
	CheckStringHash( "abc\0ignored" );
	CheckStringHash( "\x80\xFF" "AbC" );

	// Cover every tail length, ignoring nonzero bytes after the terminator.
	char buffer[32];
	for ( int len = 0; len < (int)sizeof( buffer ); ++len )
	{
		memset( buffer, 'X', sizeof( buffer ) );
		for ( int i = 0; i < len; ++i )
			buffer[i] = (char)( 'A' + i % 26 );
		buffer[len] = '\0';
		CheckStringHash( buffer );
		const char *pointer = buffer;
		CHECK( CUtlStringToken( buffer ) == CUtlStringToken( pointer ) );
		CHECK( CKV3MemberName( buffer ).GetHashCode() == CKV3MemberName( pointer ).GetHashCode() );
	}

	root->SetToEmptyTable();
	root->SetMemberInt( "weapon0", 42 );
	snprintf( buffer, sizeof( buffer ), "weapon%d", 0 );
	CHECK( root->GetMemberInt( buffer, -1 ) == 42 );
	CHECK( root->GetMemberInt( padded, -1 ) == 42 );
	CHECK( root->GetMemberCount() == 1 );
}

static KeyValues3 *MakeIntArray( KeyValues3 *kv, int count )
{
	kv->SetToEmptyArray();
	for ( int i = 0; i < count; ++i )
		kv->ArrayAddElementToTail()->SetInt( i );

	return kv;
}

static bool ArrayEquals( KeyValues3 *kv, const int *values, int count )
{
	if ( kv->GetArrayElementCount() != count )
		return false;

	for ( int i = 0; i < count; ++i )
	{
		if ( kv->GetArrayElement( i )->GetInt( -1 ) != values[i] )
			return false;
	}

	return true;
}

static void TestArrayInsert( KeyValues3 *root )
{
	KeyValues3 *arr = MakeIntArray( root, 4 );

	arr->ArrayInsertElementBefore( 1 )->SetInt( 100 );
	const int middle[] = { 0, 100, 1, 2, 3 };
	CHECK( ArrayEquals( arr, middle, 5 ) );

	arr->ArrayInsertElementBefore( 0 )->SetInt( 200 );
	const int front[] = { 200, 0, 100, 1, 2, 3 };
	CHECK( ArrayEquals( arr, front, 6 ) );

	arr->ArrayInsertElementBefore( arr->GetArrayElementCount() )->SetInt( 300 );
	const int back[] = { 200, 0, 100, 1, 2, 3, 300 };
	CHECK( ArrayEquals( arr, back, 7 ) );

	arr->ArrayInsertElementAfter( 2 )->SetInt( 400 );
	const int after[] = { 200, 0, 100, 400, 1, 2, 3, 300 };
	CHECK( ArrayEquals( arr, after, 8 ) );
}

static void TestArrayRemove( KeyValues3 *root )
{
	root->SetToEmptyTable();

	KeyValues3 *arr = MakeIntArray( root->FindOrCreateMember( "one" ), 4 );
	arr->ArrayRemoveElement( 1 );
	const int one[] = { 0, 2, 3 };
	CHECK( ArrayEquals( arr, one, 3 ) );

	arr = MakeIntArray( root->FindOrCreateMember( "several" ), 6 );
	arr->ArrayRemoveElements( 1, 3 );
	const int several[] = { 0, 4, 5 };
	CHECK( ArrayEquals( arr, several, 3 ) );

	arr = MakeIntArray( root->FindOrCreateMember( "last" ), 4 );
	arr->ArrayRemoveElement( 3 );
	const int last[] = { 0, 1, 2 };
	CHECK( ArrayEquals( arr, last, 3 ) );
	arr->ArrayAddElementToTail()->SetInt( 7 );
	const int readded[] = { 0, 1, 2, 7 };
	CHECK( ArrayEquals( arr, readded, 4 ) );

	arr = MakeIntArray( root->FindOrCreateMember( "all" ), 4 );
	arr->ArrayRemoveElements( 0, 4 );
	CHECK( arr->GetArrayElementCount() == 0 );
}

static void TestFreeKV()
{
	CKV3Arena arena;
	KeyValues3 *root = arena.Root();
	root->SetToEmptyTable();

	// A removed member's KeyValues3 goes back to the arena, so the next one reuses it
	KeyValues3 *first = nullptr;
	bool bReused = true;
	for ( int i = 0; i < 10000; ++i )
	{
		KeyValues3 *kv = root->FindOrCreateMember( "x" );
		kv->SetString( "a string long enough to be allocated on the heap" );

		if ( !first )
			first = kv;
		else if ( kv != first )
			bReused = false;

		CHECK( root->RemoveMember( "x" ) );
	}
	CHECK( bReused );
	CHECK( root->GetMemberCount() == 0 );

	root->SetMemberInt( "y", 1 );
	arena.Clear();
	arena.Root()->SetToEmptyTable();
	arena.Root()->SetMemberInt( "y", 3 );
	CHECK( arena.Root()->GetMemberInt( "y" ) == 3 );
}

static void FillTable( KeyValues3 *kv )
{
	kv->SetToEmptyTable();
	kv->SetMemberBool( "bool", true );
	kv->SetMemberInt( "int", -12345 );
	kv->SetMemberUInt64( "uint64", 0x123456789ABCDEF0ull );
	kv->SetMemberDouble( "double", 0.25 );
	kv->SetMemberString( "string", "hello world" );
	kv->SetMemberVector( "vector", Vector( 1.0f, 2.0f, 3.0f ) );

	KeyValues3 *nested = kv->FindOrCreateMember( "nested" );
	nested->SetToEmptyTable();
	nested->SetMemberString( "name", "inner" );

	KeyValues3 *strings = kv->FindOrCreateMember( "strings" );
	strings->SetToEmptyArray();
	strings->ArrayAddElementToTail()->SetString( "a" );
	strings->ArrayAddElementToTail()->SetString( "bb" );
	strings->ArrayAddElementToTail()->SetString( "ccc" );
}

static void CheckTable( KeyValues3 *kv )
{
	CHECK( kv->GetType() == KV3_TYPE_TABLE );
	CHECK( kv->GetMemberCount() == 8 );
	CHECK( kv->GetMemberBool( "bool" ) == true );
	CHECK( kv->GetMemberInt( "int" ) == -12345 );
	CHECK( kv->GetMemberUInt64( "uint64" ) == 0x123456789ABCDEF0ull );
	CHECK( kv->GetMemberDouble( "double" ) == 0.25 );
	CHECK( !strcmp( kv->GetMemberString( "string" ), "hello world" ) );
	CHECK( kv->GetMemberVector( "vector" ) == Vector( 1.0f, 2.0f, 3.0f ) );
	CHECK( kv->FindMember( "missing" ) == nullptr );

	KeyValues3 *nested = kv->FindMember( "nested" );
	CHECK( nested && nested->GetType() == KV3_TYPE_TABLE );
	CHECK( nested && !strcmp( nested->GetMemberString( "name" ), "inner" ) );

	KeyValues3 *strings = kv->FindMember( "strings" );
	CHECK( strings && strings->GetType() == KV3_TYPE_ARRAY );
	CHECK( strings && strings->GetArrayElementCount() == 3 );
	if ( strings && strings->GetArrayElementCount() == 3 )
	{
		CHECK( !strcmp( strings->GetArrayElement( 0 )->GetString(), "a" ) );
		CHECK( !strcmp( strings->GetArrayElement( 1 )->GetString(), "bb" ) );
		CHECK( !strcmp( strings->GetArrayElement( 2 )->GetString(), "ccc" ) );
	}
}

static void TestRemoveMembers( KeyValues3 *root )
{
	root->SetToEmptyTable();

	// Spans several arena clusters
	char name[32];
	for ( int i = 0; i < 1000; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		root->SetMemberString( CKV3MemberName( (const char *)name ), "value" );
	}
	CHECK( root->GetMemberCount() == 1000 );

	for ( int i = 0; i < 1000; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		CHECK( root->RemoveMember( CKV3MemberName( (const char *)name ) ) );
	}
	CHECK( root->GetMemberCount() == 0 );

	// Removed members free their nested tables, arrays and strings
	for ( int i = 0; i < 100; ++i )
	{
		KeyValues3 *nested = root->FindOrCreateMember( "nested" );
		nested->SetToEmptyTable();
		nested->SetMemberString( "string", "a string long enough to be allocated on the heap" );

		KeyValues3 *tables = nested->FindOrCreateMember( "tables" );
		tables->SetToEmptyArray();
		for ( int j = 0; j < 8; ++j )
		{
			KeyValues3 *table = tables->ArrayAddElementToTail();
			table->SetToEmptyTable();
			table->SetMemberInt( "index", j );
			table->SetMemberString( "string", "another string long enough to be allocated on the heap" );
		}

		tables->ArrayRemoveElements( 2, 4 );
		CHECK( tables->GetArrayElementCount() == 4 );
		CHECK( tables->GetArrayElement( 2 )->GetMemberInt( "index" ) == 6 );

		CHECK( root->RemoveMember( "nested" ) );
		CHECK( root->GetMemberCount() == 0 );
	}

	FillTable( root );
	CheckTable( root );
	root->SetToNull();
	CHECK( root->GetType() == KV3_TYPE_NULL );

	root->SetToEmptyTable();
	root->SetMemberInt( "after", 7 );
	CHECK( root->GetMemberInt( "after" ) == 7 );
}

static void TestRoundTrips( KeyValues3 *root )
{
	FillTable( root );
	CheckTable( root );

	// Copy to and from an arena and a KeyValues3 without one
	CKV3Arena other;
	*other.Root() = *root;
	CheckTable( other.Root() );

	KeyValues3 standalone;
	standalone = *root;
	CheckTable( &standalone );

	root->SetMemberToCopyOfValue( "copy", other.Root() );
	CheckTable( root->FindMember( "copy" ) );
	root->SetMemberToCopyOfValue( "standalone", &standalone );
	CheckTable( root->FindMember( "standalone" ) );
	CHECK( root->GetMemberCount() == 10 );

	// The copies own their strings
	other.Root()->SetMemberString( "string", "changed" );
	standalone.SetMemberString( "string", "changed" );
	CHECK( !strcmp( root->GetMemberString( "string" ), "hello world" ) );
	CHECK( !strcmp( root->FindMember( "copy" )->GetMemberString( "string" ), "hello world" ) );
	CHECK( !strcmp( root->FindMember( "standalone" )->GetMemberString( "string" ), "hello world" ) );

	CHECK( root->RemoveMember( "copy" ) );
	CHECK( root->RemoveMember( "standalone" ) );
	CheckTable( root );

	// Through tier0's text writer and parser
	CUtlBuffer buffer( 0, 0, CUtlBuffer::TEXT_BUFFER );
	CUtlString error;
	CHECK( SaveKV3( g_KV3Encoding_Text, g_KV3Format_Generic, root, &error, &buffer ) );

	CKV3Arena loaded;
	CHECK( LoadKV3( loaded.Root(), &error, &buffer, g_KV3Format_Generic, "test" ) );
	CheckTable( loaded.Root() );

	buffer.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );
	KeyValues3 loadedStandalone;
	CHECK( LoadKV3( &loadedStandalone, &error, &buffer, g_KV3Format_Generic, "test" ) );
	CheckTable( &loadedStandalone );
}

static void CheckBinaryTypes( KeyValues3 *kv, KV3TypeEx_t stringType )
{
	CBufferStringN<128> buff;

	KeyValues3 *u32 = kv->FindMember( "uint32" );
	CHECK( u32 && u32->GetTypeEx() == KV3_TYPEEX_ARRAY_UINT32 );
	CHECK( u32 && u32->GetArrayElementCount() == 3 );
	CHECK( u32 && !strcmp( u32->ToString( buff ), "10 20 4000000000" ) );

	KeyValues3 *u64 = kv->FindMember( "uint64" );
	CHECK( u64 && u64->GetTypeEx() == KV3_TYPEEX_ARRAY_UINT64 );
	CHECK( u64 && u64->GetArrayElementCount() == 2 );
	CHECK( u64 && !strcmp( u64->ToString( buff ), "1 9223372036854775808" ) );

	KeyValues3 *color = kv->FindMember( "color" );
	CHECK( color && color->GetTypeEx() == KV3_TYPEEX_ARRAY_UINT32 );
	CHECK( kv->GetMemberColor( "color" ) == Color( 1, 2, 3, 4 ) );

	KeyValues3 *str = kv->FindMember( "string" );
	CHECK( str && str->GetTypeEx() == stringType );
	CHECK( !strcmp( kv->GetMemberString( "string" ), "a string long enough to be allocated on the heap" ) );
	CHECK( !strcmp( kv->GetMemberString( "short" ), "abc" ) );
}

static void TestBinaryTypes()
{
	// Load flags: 1 keeps object references as strings, 2 stores binary strings as arena symbols
	const uint32 nLoadStringsAsSymbols = ( 1 << 1 );

	CKV3Arena arena;
	KeyValues3 *root = arena.Root();
	root->SetToEmptyTable();

	KeyValues3 *u32 = root->FindOrCreateMember( "uint32" );
	u32->SetToEmptyArray();
	u32->ArrayAddElementToTail()->SetUInt( 10 );
	u32->ArrayAddElementToTail()->SetUInt( 20 );
	u32->ArrayAddElementToTail()->SetUInt( 4000000000u );

	KeyValues3 *u64 = root->FindOrCreateMember( "uint64" );
	u64->SetToEmptyArray();
	u64->ArrayAddElementToTail()->SetUInt64( 1 );
	u64->ArrayAddElementToTail()->SetUInt64( 0x8000000000000000ull );

	KeyValues3 *color = root->FindOrCreateMember( "color" );
	color->SetToEmptyArray();
	for ( int i = 1; i <= 4; ++i )
		color->ArrayAddElementToTail()->SetUInt( i );

	root->SetMemberString( "string", "a string long enough to be allocated on the heap" );
	root->SetMemberString( "short", "abc" );

	// tier0's binary loader stores small arrays of unsigned integers as typed arrays
	CUtlBuffer buffer;
	CUtlString error;
	CHECK( SaveKV3( g_KV3Encoding_Binary, g_KV3Format_Generic, root, &error, &buffer ) );

	CKV3Arena loaded;
	CHECK( LoadKV3( loaded.Root(), &error, &buffer, g_KV3Format_Generic, "test" ) );
	CheckBinaryTypes( loaded.Root(), KV3_TYPEEX_STRING_EXTERN );

	buffer.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );
	CKV3Arena symbols;
	CHECK( LoadKV3( symbols.Root(), &error, &buffer, g_KV3Format_Generic, "test", nLoadStringsAsSymbols ) );
	CheckBinaryTypes( symbols.Root(), KV3_TYPEEX_STRING_SYMBOL );
	CHECK( symbols.Root()->FindMember( "short" )->GetTypeEx() == KV3_TYPEEX_STRING_SYMBOL );

	// Copies keep the typed arrays and own their strings
	CKV3Arena other;
	*other.Root() = *symbols.Root();
	CheckBinaryTypes( other.Root(), KV3_TYPEEX_STRING );

	KeyValues3 standalone;
	standalone = *symbols.Root();
	CheckBinaryTypes( &standalone, KV3_TYPEEX_STRING );

	// Setting a loaded value frees its typed array
	symbols.Root()->FindMember( "uint32" )->SetInt( 5 );
	symbols.Root()->FindMember( "uint64" )->SetToEmptyArray();
	symbols.Root()->SetMemberString( "string", "changed" );
	CHECK( symbols.Root()->GetMemberInt( "uint32" ) == 5 );
	CHECK( symbols.Root()->FindMember( "uint64" )->GetArrayElementCount() == 0 );
	CHECK( !strcmp( symbols.Root()->GetMemberString( "string" ), "changed" ) );
	CheckBinaryTypes( other.Root(), KV3_TYPEEX_STRING );

	// Accessing the elements turns a typed array into a regular one
	KeyValues3 *loadedU32 = loaded.Root()->FindMember( "uint32" );
	CHECK( loadedU32->GetArrayElement( 2 ) && loadedU32->GetArrayElement( 2 )->GetUInt() == 4000000000u );
	CHECK( loadedU32->GetTypeEx() == KV3_TYPEEX_ARRAY && loadedU32->GetArrayElementCount() == 3 );
	CHECK( loadedU32->GetArrayElement( 0 )->GetSubType() == KV3_SUBTYPE_UINT32 );
	CHECK( loadedU32->GetArrayElement( 0 )->GetUInt() == 10 );

	KeyValues3 *loadedU64 = loaded.Root()->FindMember( "uint64" );
	loadedU64->ArrayAddElementToTail()->SetUInt64( 7 );
	CHECK( loadedU64->GetTypeEx() == KV3_TYPEEX_ARRAY && loadedU64->GetArrayElementCount() == 3 );
	CHECK( loadedU64->GetArrayElement( 1 ) && loadedU64->GetArrayElement( 1 )->GetUInt64() == 0x8000000000000000ull );
	CHECK( loadedU64->GetArrayElement( 2 ) && loadedU64->GetArrayElement( 2 )->GetUInt64() == 7 );
}

// Binary KV3 of { a = [ -3, -2, -1, 1000, 2000, 30000 ], b = [ 10, -20, 300 ] } with int16 typed arrays,
// which tier0's binary writer only produces from int16 typed arrays
static const unsigned char g_Int16ArraysKV3[] =
{
	0x05, 0x33, 0x56, 0x4b, 0x7c, 0x16, 0x12, 0x74, 0xe9, 0x06, 0x98, 0x46, 0xaf, 0xf2, 0xe6, 0x3e,
	0xb5, 0x90, 0x37, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
	0x35, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00, 0xfd, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xe8, 0x03, 0xd0, 0x07, 0x30, 0x75, 0x0a, 0x00, 0xec, 0xff, 0x2c, 0x01, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x09, 0x19, 0x14, 0x19, 0x14, 0x00, 0xdd, 0xee, 0xff,
};

static void TestInt16Arrays()
{
	CUtlBuffer buffer( g_Int16ArraysKV3, sizeof( g_Int16ArraysKV3 ), CUtlBuffer::READ_ONLY );
	CUtlString error;
	CKV3Arena arena;
	CHECK( LoadKV3( arena.Root(), &error, &buffer, g_KV3Format_Generic, "test" ) );

	KeyValues3 *root = arena.Root();
	KeyValues3 *a = root->FindMember( "a" );
	KeyValues3 *b = root->FindMember( "b" );
	CHECK( a && a->GetTypeEx() == KV3_TYPEEX_ARRAY_INT16 && a->GetArrayElementCount() == 6 );
	CHECK( b && b->GetTypeEx() == KV3_TYPEEX_ARRAY_INT16 && b->GetArrayElementCount() == 3 );
	if ( !a || !b )
		return;

	CBufferStringN<128> buff;
	CHECK( !strcmp( b->ToString( buff ), "10 -20 300" ) );
	CHECK( b->GetColor() == Color( 10, -20, 300, 255 ) );

	// A copy of up to four elements stores them in the value itself
	root->SetMemberToCopyOfValue( "copy", b );
	KeyValues3 *copy = root->FindMember( "copy" );
	CHECK( copy->GetTypeEx() == KV3_TYPEEX_ARRAY_INT16_SHORT );
	CHECK( !strcmp( copy->ToString( buff ), "10 -20 300" ) );
	CHECK( copy->GetColor() == Color( 10, -20, 300, 255 ) );

	// Accessing the elements turns a typed array into a regular one
	const int values[] = { -3, -2, -1, 1000, 2000, 30000 };
	for ( int i = 0; i < 6; ++i )
		CHECK( a->GetArrayElement( i ) && a->GetArrayElement( i )->GetInt() == values[i] );
	CHECK( a->GetTypeEx() == KV3_TYPEEX_ARRAY && a->GetArrayElementCount() == 6 );
	CHECK( a->GetArrayElement( 0 )->GetSubType() == KV3_SUBTYPE_INT16 );

	copy->ArrayAddElementToTail()->SetInt( 40 );
	CHECK( copy->GetTypeEx() == KV3_TYPEEX_ARRAY && copy->GetArrayElementCount() == 4 );
	// Only typed arrays print their elements
	CHECK( copy->ToString( buff ) == nullptr && !strcmp( buff.Get(), "<array: 4 elements>" ) );

	b->SetArrayElementCount( 2 );
	CHECK( b->GetTypeEx() == KV3_TYPEEX_ARRAY && b->GetArrayElementCount() == 2 );
	CHECK( b->GetArrayElement( 1 ) && b->GetArrayElement( 1 )->GetInt() == -20 );
}

static void TestTypeChecks( KeyValues3 *root )
{
	root->SetToEmptyTable();
	CHECK( root->IsTable() && !root->IsArray() );

	KeyValues3 *arr = MakeIntArray( root->FindOrCreateMember( "array" ), 2 );
	CHECK( arr->IsArray() && !arr->IsTable() );

	KeyValues3 *vec = root->FindOrCreateMember( "vector" );
	vec->SetVector( Vector( 1.0f, 2.0f, 3.0f ) );
	CHECK( vec->GetTypeEx() == KV3_TYPEEX_ARRAY_FLOAT32 && vec->IsArray() );

	KeyValues3 *value = root->FindOrCreateMember( "int" );
	value->SetInt( 1 );
	CHECK( !value->IsArray() && !value->IsTable() );

	root->SetToNull();
	CHECK( !root->IsArray() && !root->IsTable() );
}

static void TestInvalidMemberNames( KeyValues3 *root )
{
	root->SetToNull();
	root->SetHasInvalidMemberNames( true );
	CHECK( !root->HasInvalidMemberNames() );

	root->SetToEmptyTable();
	CHECK( !root->HasInvalidMemberNames() );
	root->SetHasInvalidMemberNames( true );
	CHECK( root->HasInvalidMemberNames() );
	CHECK( root->GetTable()->HasInvalidMemberNames() );
	root->SetHasInvalidMemberNames( false );
	CHECK( !root->HasInvalidMemberNames() );
}

static void TestRenameMember( KeyValues3 *root )
{
	FillTable( root );

	KeyValues3 *value = root->FindMember( "int" );
	CHECK( root->RenameMember( "int", "renamed" ) == value );
	CHECK( root->FindMember( "int" ) == nullptr );
	CHECK( root->FindMember( "renamed" ) == value );
	CHECK( root->GetMemberInt( "renamed" ) == -12345 );
	CHECK( root->GetMemberCount() == 8 );
	CHECK( !strcmp( root->GetMemberName( 1 ), "renamed" ) );
	CHECK( root->GetMemberHash( 1 ) == CUtlStringToken( "renamed" ) );

	CHECK( root->RenameMember( "missing", "other" ) == nullptr );
	CHECK( root->FindMember( "other" ) == nullptr );

	CHECK( root->RenameMember( "renamed", "bool" ) == nullptr );
	CHECK( root->GetMemberInt( "renamed" ) == -12345 );
	CHECK( root->GetMemberBool( "bool" ) == true );

	CHECK( root->RenameMember( "renamed", "" ) == nullptr );
	CHECK( root->FindMember( "renamed" ) == value );

	CHECK( root->RenameMember( "renamed", "int" ) == value );
	CheckTable( root );

	// Large tables also find members through a hash of their names
	root->SetToEmptyTable();
	char name[32];
	char newName[32];
	for ( int i = 0; i < 200; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		root->SetMemberInt( CKV3MemberName( (const char *)name ), i );
	}

	for ( int i = 0; i < 200; i += 2 )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		snprintf( newName, sizeof( newName ), "r%d", i );
		CHECK( root->RenameMember( CKV3MemberName( (const char *)name ), CKV3MemberName( (const char *)newName ) ) );
	}

	for ( int pass = 0; pass < 2; ++pass )
	{
		for ( int i = 0; i < 200; ++i )
		{
			snprintf( name, sizeof( name ), "%c%d", ( i % 2 ) ? 'm' : 'r', i );
			CHECK( root->GetMemberInt( CKV3MemberName( (const char *)name ), -1 ) == i );

			snprintf( name, sizeof( name ), "%c%d", ( i % 2 ) ? 'r' : 'm', i );
			CHECK( root->FindMember( CKV3MemberName( (const char *)name ) ) == nullptr );
		}
	}

	CHECK( root->RenameMember( "r0", "m1" ) == nullptr );
	CHECK( root->GetMemberCount() == 200 );
}

static void TestOverlayKeysFrom( KeyValues3 *root )
{
	CKV3Arena arena;
	KeyValues3 *src = arena.Root();
	src->SetToEmptyTable();
	src->SetMemberInt( "int", 5 );
	src->SetMemberString( "added", "a string long enough to be allocated on the heap" );
	KeyValues3 *srcNested = src->FindOrCreateMember( "nested" );
	srcNested->SetToEmptyTable();
	srcNested->SetMemberString( "name", "overlaid" );
	srcNested->SetMemberInt( "extra", 9 );

	// Tables on both sides are merged
	FillTable( root );
	root->OverlayKeysFrom( src, true );
	CHECK( root->GetMemberCount() == 9 );
	CHECK( root->GetMemberInt( "int" ) == 5 );
	CHECK( !strcmp( root->GetMemberString( "added" ), "a string long enough to be allocated on the heap" ) );
	CHECK( !strcmp( root->GetMemberString( "string" ), "hello world" ) );
	KeyValues3 *nested = root->FindMember( "nested" );
	CHECK( nested && nested->GetMemberCount() == 2 );
	CHECK( nested && !strcmp( nested->GetMemberString( "name" ), "overlaid" ) );
	CHECK( nested && nested->GetMemberInt( "extra" ) == 9 );

	// Without recursion they are replaced
	FillTable( root );
	root->FindMember( "nested" )->SetMemberInt( "kept", 1 );
	root->OverlayKeysFrom( src, false );
	CHECK( root->GetMemberCount() == 9 );
	CHECK( root->GetMemberInt( "int" ) == 5 );
	nested = root->FindMember( "nested" );
	CHECK( nested && nested != srcNested && nested->GetMemberCount() == 2 );
	CHECK( nested && nested->FindMember( "kept" ) == nullptr );
	CHECK( nested && nested->GetMemberInt( "extra" ) == 9 );

	// The copies own their strings
	src->SetMemberString( "added", "changed" );
	srcNested->SetMemberString( "name", "changed" );
	CHECK( !strcmp( root->GetMemberString( "added" ), "a string long enough to be allocated on the heap" ) );
	CHECK( nested && !strcmp( nested->GetMemberString( "name" ), "overlaid" ) );

	// A value that isn't a table becomes one
	root->SetInt( 1 );
	root->OverlayKeysFrom( src, true );
	CHECK( root->IsTable() && root->GetMemberCount() == 3 );
	CHECK( root->GetMemberInt( "int" ) == 5 );
	CHECK( !strcmp( root->GetMemberString( "added" ), "changed" ) );
}

static void TestFindMemberHint( KeyValues3 *root )
{
	FillTable( root );

	KV3MemberId_t hint = KV3_INVALID_MEMBER;
	CHECK( root->FindMember( "double", hint ) == root->FindMember( "double" ) );
	CHECK( hint == 4 );

	CHECK( root->FindMember( "string", hint ) == root->FindMember( "string" ) );
	CHECK( hint == 5 );

	CHECK( root->FindMember( "int", hint ) == root->FindMember( "int" ) );
	CHECK( hint == 2 );

	KeyValues3 def;
	CHECK( root->FindMember( "missing", hint, &def ) == &def );
	CHECK( hint == 2 );

	hint = 1000;
	const KeyValues3 *constRoot = root;
	CHECK( constRoot->FindMember( "strings", hint ) == root->FindMember( "strings" ) );
	CHECK( hint == 8 );

	CHECK( constRoot->FindMember( "bool", hint ) == root->FindMember( "bool" ) );
	CHECK( hint == 1 );

	// Members found through the hash of their names leave the hint alone
	root->SetToEmptyTable();
	char name[32];
	for ( int i = 0; i < 200; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		root->SetMemberInt( CKV3MemberName( (const char *)name ), i );
	}

	hint = 0;
	CHECK( root->FindMember( "m150", hint ) && root->FindMember( "m150", hint )->GetInt() == 150 );
	CHECK( hint == 0 );
}

static bool HasComment( KV3MetaData_t *meta, int key, const char *comment )
{
	int index = meta->m_Comments.Find( key );
	return meta->m_Comments.IsValidIndex( index ) && !strcmp( meta->m_Comments[index].Get(), comment );
}

static void TestMetaData()
{
	CKV3Arena arena;
	arena.EnableMetaData( true );

	KeyValues3 *root = arena.Root();
	root->SetToEmptyTable();

	// Spans several arena clusters
	const int count = 600;
	char name[32];
	for ( int i = 0; i < count; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		KeyValues3 *member = root->FindOrCreateMember( CKV3MemberName( (const char *)name ) );
		member->SetInt( i );

		KV3MetaData_t *meta = member->GetMetaData();
		CHECK( meta && meta->m_Comments.Count() == 0 );
		if ( !meta )
			continue;

		meta->m_nLine = i;
		meta->m_Comments.Insert( 1, CBufferString( "a comment long enough to be allocated on the heap" ) );
		meta->m_Comments.Insert( 2, CBufferString( name ) );
	}

	for ( int i = 0; i < count; ++i )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		KV3MetaData_t *meta = root->FindMember( CKV3MemberName( (const char *)name ) )->GetMetaData();
		CHECK( meta && meta->m_nLine == i && meta->m_Comments.Count() == 2 );
		CHECK( meta && HasComment( meta, 1, "a comment long enough to be allocated on the heap" ) );
		CHECK( meta && HasComment( meta, 2, name ) );
	}

	// Copies take the metadata along
	KeyValues3 *copy = root->FindOrCreateMember( "copy" );
	*copy = *root->FindMember( "m150" );
	KV3MetaData_t *copyMeta = copy->GetMetaData();
	CHECK( copyMeta && copyMeta->m_nLine == 150 && HasComment( copyMeta, 2, "m150" ) );

	if ( copyMeta )
	{
		copyMeta->Clear();
		CHECK( copyMeta->m_nLine == 0 && copyMeta->m_Comments.Count() == 0 );
		copyMeta->m_Comments.Insert( 3, CBufferString( "after clear" ) );
		CHECK( HasComment( copyMeta, 3, "after clear" ) );
	}

	// A freed member's metadata is cleared for the next one
	for ( int i = 0; i < count; i += 2 )
	{
		snprintf( name, sizeof( name ), "m%d", i );
		CHECK( root->RemoveMember( CKV3MemberName( (const char *)name ) ) );
	}

	for ( int i = 0; i < count; i += 2 )
	{
		snprintf( name, sizeof( name ), "n%d", i );
		KV3MetaData_t *meta = root->FindOrCreateMember( CKV3MemberName( (const char *)name ) )->GetMetaData();
		CHECK( meta && meta->m_nLine == 0 && meta->m_Comments.Count() == 0 );
	}

	arena.EnableMetaData( false );
	CHECK( root->GetMetaData() == nullptr );
	CHECK( root->FindMember( "m151" )->GetMetaData() == nullptr );

	arena.EnableMetaData( true );
	KV3MetaData_t *meta = root->FindMember( "m151" )->GetMetaData();
	CHECK( meta && meta->m_nLine == 0 && meta->m_Comments.Count() == 0 );
	meta = root->FindMember( "m551" )->GetMetaData();
	CHECK( meta && meta->m_nLine == 0 && meta->m_Comments.Count() == 0 );

	arena.Purge();
	CHECK( arena.IsMetaDataEnabled() );
	meta = arena.Root()->GetMetaData();
	CHECK( meta && meta->m_Comments.Count() == 0 );
	if ( meta )
		meta->m_Comments.Insert( 1, CBufferString( "a comment long enough to be allocated on the heap" ) );
}

// Tables and arrays allocated smaller than their type, freed from an arena's raw entries and from the heap
static void TestUndersizedNodes()
{
	CKV3Arena src;
	src.Root()->SetToEmptyTable();
	src.Root()->SetMemberInt( "a", 1 );

	CUtlBuffer buf;
	CUtlString error;
	CHECK( SaveKV3( g_KV3Encoding_Binary, g_KV3Format_Generic, src.Root(), &error, &buf ) );

	for ( int i = 0; i < 8; ++i )
	{
		CUtlBuffer in( buf.Base(), buf.TellPut(), CUtlBuffer::READ_ONLY );
		CKV3Arena arena;
		CHECK( LoadKV3( arena.Root(), &error, &in, g_KV3Format_Generic, "test" ) );
		CHECK( arena.Root()->GetMemberInt( "a" ) == 1 );
	}

	for ( int i = 0; i < 8; ++i )
	{
		KeyValues3 kv;
		kv.SetToEmptyTable();
		kv.SetMemberVector( "vector", Vector( 1.0f, 2.0f, 3.0f ) );

		CUtlBuffer text( 0, 0, CUtlBuffer::TEXT_BUFFER );
		CHECK( SaveKV3( g_KV3Encoding_Text, g_KV3Format_Generic, &kv, &error, &text ) );
	}

	// A damaged heap shows up in later allocations
	for ( int i = 0; i < 1000; ++i )
		free( malloc( 16 + i * 8 ) );
}

// The raw entry lists are protected; only the arena's loaders fill them
class CRawListArena : public CKV3Arena
{
public:
	int ArrayFreeBytes() const { return m_RawArrayEntries.FreeBytes(); }
	int ArrayAllocatedBytes() const { return m_RawArrayEntries.AllocatedBytes(); }
};

// A raw entry list is never grown while it holds nodes: a node that only fits without its entry's link goes to a
// cluster. A binary load leaves the allocator's rounding free in the list, so look for a load that leaves exactly the
// node size of a clustered array (2 to 6 elements) free
static void TestRawListFit()
{
	bool bTested = false;
	for ( int nArrays = 1; nArrays <= 12 && !bTested; ++nArrays )
	{
		CKV3Arena src;
		src.Root()->SetToEmptyTable();
		for ( int i = 0; i < nArrays; ++i )
		{
			char name[ 16 ];
			V_snprintf( name, sizeof( name ), "a%d", i );
			KeyValues3 *pArray = src.Root()->FindOrCreateMember( name );
			pArray->SetArrayElementCount( 3 + i );
			for ( int j = 0; j < 3 + i; ++j )
				pArray->GetArrayElement( j )->SetToEmptyTable();
		}

		CUtlBuffer buf;
		CUtlString error;
		CHECK( SaveKV3( g_KV3Encoding_Binary, g_KV3Format_Generic, src.Root(), &error, &buf ) );

		CUtlBuffer in( buf.Base(), buf.TellPut(), CUtlBuffer::READ_ONLY );
		CRawListArena arena;
		CHECK( LoadKV3( arena.Root(), &error, &in, g_KV3Format_Generic, "test" ) );

		int nCount = -1;
		for ( int n = 2; n <= KV3_ARRAY_MAX_FIXED_MEMBERS; ++n )
		{
			if ( (int)CKeyValues3Array::TotalSizeOf( n ) == arena.ArrayFreeBytes() )
				nCount = n;
		}
		if ( nCount < 0 )
			continue;

		bTested = true;
		int nAllocated = arena.ArrayAllocatedBytes();
		arena.Root()->FindOrCreateMember( "new" )->SetArrayElementCount( nCount );
		CHECK( arena.ArrayAllocatedBytes() == nAllocated );
		CHECK( arena.Root()->FindMember( "new" )->GetArrayElementCount() == nCount );

		for ( int i = 0; i < nArrays; ++i )
		{
			char name[ 16 ];
			V_snprintf( name, sizeof( name ), "a%d", i );
			KeyValues3 *pArray = arena.Root()->FindMember( name );
			CHECK( pArray && pArray->GetArrayElementCount() == 3 + i );
		}
	}
	CHECK( bTested );
}

// A raw-list array that grew onto the heap is released once: removing it leaves its node purged, so clearing the
// arena doesn't free its elements again
static void TestRawArrayReleasedOnce()
{
	CKV3Arena src;
	src.Root()->SetToEmptyTable();
	for ( int i = 0; i < 4; ++i )
	{
		char name[ 16 ];
		V_snprintf( name, sizeof( name ), "a%d", i );
		src.Root()->FindOrCreateMember( name )->SetArrayElementCount( 3 );
	}

	CUtlBuffer buf;
	CUtlString error;
	CHECK( SaveKV3( g_KV3Encoding_Binary, g_KV3Format_Generic, src.Root(), &error, &buf ) );

	for ( int round = 0; round < 2; ++round )
	{
		CUtlBuffer in( buf.Base(), buf.TellPut(), CUtlBuffer::READ_ONLY );
		CRawListArena arena;
		CHECK( LoadKV3( arena.Root(), &error, &in, g_KV3Format_Generic, "test" ) );

		// The array's node is the value's data
		KeyValues3 *grown = arena.Root()->FindMember( "a1" );
		CKeyValues3Array *node = grown ? *(CKeyValues3Array **)( (uint8 *)grown + 8 ) : nullptr;
		CHECK( node && arena.IsArrayRawAllocated( node ) );
		if ( !node )
			continue;
		grown->SetArrayElementCount( 40 );
		CHECK( ( (const uint8 *)node )[13] != 0 );
		CHECK( arena.Root()->RemoveMember( CKV3MemberName( "a1" ) ) );

		// The node stays in the raw list, purged back to its inline elements (no longer dynamic, no elements)
		CHECK( ( (const uint8 *)node )[13] == 0 && node->Count() == 0 );
		if ( round == 0 )
			arena.Clear();
		else
			arena.Purge();
	}
}

// A purged arena has no raw entry lists left: arrays allocated in it afterwards go to clusters again
static void TestPurgeRawLists()
{
	CKV3Arena src;
	src.Root()->SetToEmptyTable();
	for ( int i = 0; i < 4; ++i )
	{
		char name[ 16 ];
		V_snprintf( name, sizeof( name ), "a%d", i );
		src.Root()->FindOrCreateMember( name )->SetArrayElementCount( 3 );
	}

	CUtlBuffer buf;
	CUtlString error;
	CHECK( SaveKV3( g_KV3Encoding_Binary, g_KV3Format_Generic, src.Root(), &error, &buf ) );

	CUtlBuffer in( buf.Base(), buf.TellPut(), CUtlBuffer::READ_ONLY );
	CRawListArena arena;
	CHECK( LoadKV3( arena.Root(), &error, &in, g_KV3Format_Generic, "test" ) );
	CHECK( arena.ArrayAllocatedBytes() > 0 );

	arena.Purge();
	CHECK( arena.ArrayAllocatedBytes() == 0 );
	if ( arena.ArrayAllocatedBytes() != 0 )
		return;

	arena.Root()->SetToEmptyTable();
	arena.Root()->FindOrCreateMember( "new" )->SetArrayElementCount( 2 );
	CHECK( arena.Root()->FindMember( "new" )->GetArrayElementCount() == 2 );
}

static void TestNumbersFromPartialStrings( KeyValues3 *root )
{
	// Signed integers take the number before the first invalid character, as tier0's getters do
	root->SetString( "12abc" );
	CHECK( root->GetInt( -1 ) == 12 );
	CHECK( root->GetInt64( -1 ) == 12 );
	CHECK( root->GetShort( -1 ) == 12 );

	// Unsigned and floating point parsing return the default
	CHECK( root->GetUInt( 7 ) == 7 );
	CHECK( root->GetFloat( 0.5f ) == 0.5f );

	// No number at all
	root->SetString( "abc" );
	CHECK( root->GetInt( -1 ) == -1 );
}

static void TestStringTypes( KeyValues3 *root )
{
	// A value constructed as a string holds an empty short string
	KeyValues3 str( KV3_TYPEEX_STRING );
	CHECK( str.GetTypeEx() == KV3_TYPEEX_STRING_SHORT );
	CHECK( str.GetString() && !strcmp( str.GetString(), "" ) );

	// External strings are referenced whatever their length
	static const char szShort[] = "abc";
	root->SetStringExternal( szShort );
	CHECK( root->GetTypeEx() == KV3_TYPEEX_STRING_EXTERN );
	CHECK( root->GetString() == szShort );
}

static void TestEmptyMemberName( KeyValues3 *root )
{
	root->SetToEmptyTable();
	root->SetMemberInt( "", 5 );
	CHECK( root->GetMemberInt( "" ) == 5 );

	const char *name = root->GetMemberName( 0 );
	CHECK( name && !strcmp( name, "" ) );
	if ( !name )
		return;

	KeyValues3 copy;
	copy = *root;
	CHECK( copy.GetMemberName( 0 ) && !strcmp( copy.GetMemberName( 0 ), "" ) );
	CHECK( copy.GetMemberInt( "" ) == 5 );
}

static void TestBoolFromDouble( KeyValues3 *root )
{
	// Converted through int64, so fractions below one are false
	root->SetDouble( 0.5 );
	CHECK( !root->GetBool() );
	root->SetDouble( -2.0 );
	CHECK( root->GetBool() );
}

static void TestArrayInsertNonArray( KeyValues3 *root )
{
	root->SetInt( 5 );
	CHECK( root->ArrayInsertElementBefore( 0 ) == nullptr );
	CHECK( root->GetTypeEx() == KV3_TYPEEX_INT && root->GetInt() == 5 );
}

static void TestReadTypedArrays( KeyValues3 *root )
{
	root->SetVector( Vector( 1.0f, 2.0f, 3.0f ) );
	CHECK( root->GetTypeEx() == KV3_TYPEEX_ARRAY_FLOAT32 );
	CHECK( root->GetColor() == Color( 1, 2, 3, 255 ) );

	CUtlBuffer buffer( g_Int16ArraysKV3, sizeof( g_Int16ArraysKV3 ), CUtlBuffer::READ_ONLY );
	CUtlString error;
	CKV3Arena arena;
	CHECK( LoadKV3( arena.Root(), &error, &buffer, g_KV3Format_Generic, "test" ) );

	KeyValues3 *b = arena.Root()->FindMember( "b" );
	CHECK( b && b->GetTypeEx() == KV3_TYPEEX_ARRAY_INT16 );
	CHECK( b && b->GetVector() == Vector( 10.0f, -20.0f, 300.0f ) );
}

static bool IsInStorage( const CKV3MemberNameWithStorage &name )
{
	const char *pObject = (const char *)&name;
	return name.GetString() >= pObject && name.GetString() < pObject + sizeof( name );
}

class CMemberNameWithSymId : public CKV3MemberNameWithStorage
{
public:
	CMemberNameWithSymId( const char *pszString, UtlSymLargeId_t symid ) : CKV3MemberNameWithStorage( pszString ) { m_SymId = symid; }
};

// The name points at its own copy of the string, also after being copied or relocated
static void TestMemberNameWithStorage( KeyValues3 *root )
{
	CKV3MemberNameWithStorage empty;
	CHECK( empty.GetHashCode() == 0 && strcmp( empty.GetString(), "" ) == 0 && empty.GetSymId() == UTL_INVAL_SYMBOL_LARGE );

	char buffer[64] = "Member_Name";
	CKV3MemberNameWithStorage name( (const char *)buffer );
	strcpy( buffer, "overwritten" );
	CHECK( strcmp( name.GetString(), "Member_Name" ) == 0 && IsInStorage( name ) );
	CHECK( name.GetHashCode() == CKV3MemberName( "member_name" ).GetHashCode() );

	const char k_szLong[] = "a member name longer than the thirty two chars of inline storage";
	CKV3MemberNameWithStorage longName( k_szLong );
	CHECK( strcmp( longName.GetString(), k_szLong ) == 0 && !IsInStorage( longName ) );
	CHECK( longName.GetHashCode() == CUtlStringToken( k_szLong ).GetHashCode() );

	CKV3MemberNameWithStorage copy( name );
	CHECK( strcmp( copy.GetString(), "Member_Name" ) == 0 && IsInStorage( copy ) && copy.GetHashCode() == name.GetHashCode() );

	copy = longName;
	CHECK( strcmp( copy.GetString(), k_szLong ) == 0 && copy.GetHashCode() == longName.GetHashCode() );

	CKV3MemberNameWithStorage *pSelf = &copy;
	copy = *pSelf;
	CHECK( strcmp( copy.GetString(), k_szLong ) == 0 );

	copy.Set( nullptr );
	CHECK( copy.GetHashCode() == 0 && strcmp( copy.GetString(), "" ) == 0 );

	// A new string drops the symbol id of the old one
	CMemberNameWithSymId withSymId( "Member_Name", 7 );
	CHECK( withSymId.GetSymId() == 7 );
	withSymId.Set( "Other_Name" );
	CHECK( withSymId.GetSymId() == UTL_INVAL_SYMBOL_LARGE && strcmp( withSymId.GetString(), "Other_Name" ) == 0 );

	CMemberNameWithSymId assigned( "Member_Name", 7 );
	static_cast< CKV3MemberNameWithStorage & >( assigned ) = longName;
	CHECK( assigned.GetSymId() == UTL_INVAL_SYMBOL_LARGE && strcmp( assigned.GetString(), k_szLong ) == 0 );

	// Growing relocates the names through their copy constructor
	CUtlLeanVector< CKV3MemberNameWithStorage > names;
	for ( int i = 0; i < 20; i++ )
		names.AddToTail( name );

	bool bIntact = true;
	for ( int i = 0; i < names.Count(); i++ )
		bIntact = bIntact && IsInStorage( names[i] ) && strcmp( names[i].GetString(), "Member_Name" ) == 0;
	CHECK( bIntact );

	// Usable wherever a CKV3MemberName is
	root->SetMemberInt( name, 5 );
	KeyValues3 *member = root->FindMember( CKV3MemberNameWithStorage( "MEMBER_NAME" ) );
	CHECK( member && member->GetInt() == 5 );
}

// The value and its subtype are copied, the flags stay the destination's own
static void TestCopyFromFlags( KeyValues3 *root )
{
	KeyValues3 src;
	src.SetInt( 7 );
	src.SetFlag( KEYVALUES3_FLAG_RESOURCE_REFERENCE, true );

	root->SetFlag( KEYVALUES3_FLAG_MULTILINE_STRING, true );
	*root = src;
	CHECK( root->GetInt() == 7 && root->GetSubType() == KV3_SUBTYPE_INT32 );
	CHECK( root->GetAllFlags() == KEYVALUES3_FLAG_MULTILINE_STRING );
}

// A typed array is turned into a regular one to swap its items, as the other element accessors do
static void TestArraySwapTyped( KeyValues3 *root )
{
	root->SetVector( Vector( 1.0f, 2.0f, 3.0f ) );
	CHECK( root->GetTypeEx() == KV3_TYPEEX_ARRAY_FLOAT32 );
	root->ArraySwapItems( 0, 2 );
	CHECK( root->GetTypeEx() == KV3_TYPEEX_ARRAY );
	CHECK( root->GetVector() == Vector( 3.0f, 2.0f, 1.0f ) );
}

// Clearing an arena leaves binary data that was never allocated unallocated
static void TestArenaClearBinaryData()
{
	CKV3Arena arena;
	arena.Root()->SetInt( 1 );
	arena.Clear();
	CHECK( arena.GetBinaryData().Size() == 0 );
	CHECK( arena.Root()->IsNull() );
}

// An array that already has elements is emptied too
static void TestSetToEmptyArray( KeyValues3 *root )
{
	root->SetArrayElementCount( 3 );
	root->SetToEmptyArray();
	CHECK( root->GetTypeEx() == KV3_TYPEEX_ARRAY && root->GetSubType() == KV3_SUBTYPE_ARRAY );
	CHECK( root->GetArrayElementCount() == 0 );
}

static void TestOverlayKeysFromNonTables()
{
	CKV3Arena arena;
	arena.EnableMetaData( true );

	KeyValues3 *root = arena.Root();
	root->SetToEmptyTable();
	KeyValues3 *src = root->FindOrCreateMember( "src" );
	src->SetToEmptyTable();
	src->GetMetaData()->m_nLine = 12;

	// A value that isn't a table is cleared and only becomes one when a member is added
	KeyValues3 *dest = root->FindOrCreateMember( "dest" );
	dest->SetInt( 1 );
	dest->OverlayKeysFrom( src, true );
	CHECK( dest->IsNull() );

	// The source table's metadata comes along
	CHECK( dest->GetMetaData() && dest->GetMetaData()->m_nLine == 12 );

	// Only a table has members to overlay
	KeyValues3 notTable;
	notTable.SetInt( 5 );
	dest->SetInt( 1 );
	dest->OverlayKeysFrom( &notTable, true );
	CHECK( dest->IsNull() );

	dest->SetToEmptyTable();
	dest->SetMemberInt( "kept", 1 );
	dest->OverlayKeysFrom( &notTable, true );
	CHECK( dest->IsTable() && dest->GetMemberInt( "kept" ) == 1 );
}

// Runs a test on an arena's root and on a KeyValues3 without an arena, whose members are allocated on the heap
static void RunTest( void ( *pfnTest )( KeyValues3 * ) )
{
	{
		CKV3Arena arena;
		pfnTest( arena.Root() );
	}

	{
		KeyValues3 kv;
		pfnTest( &kv );
	}

	{
		KeyValues3 *kv = new KeyValues3;
		pfnTest( kv );
		delete kv;
	}
}

int main()
{
	RunTest( TestStringHashes );
	RunTest( TestArrayInsert );
	RunTest( TestArrayRemove );
	TestFreeKV();
	RunTest( TestRemoveMembers );
	RunTest( TestRoundTrips );
	TestBinaryTypes();
	TestInt16Arrays();
	RunTest( TestTypeChecks );
	RunTest( TestInvalidMemberNames );
	RunTest( TestRenameMember );
	RunTest( TestOverlayKeysFrom );
	RunTest( TestFindMemberHint );
	TestMetaData();
	TestUndersizedNodes();
	TestRawListFit();
	TestPurgeRawLists();
	TestRawArrayReleasedOnce();
	RunTest( TestNumbersFromPartialStrings );
	RunTest( TestStringTypes );
	RunTest( TestEmptyMemberName );
	RunTest( TestBoolFromDouble );
	RunTest( TestArrayInsertNonArray );
	RunTest( TestReadTypedArrays );
	RunTest( TestMemberNameWithStorage );
	RunTest( TestCopyFromFlags );
	TestOverlayKeysFromNonTables();
	RunTest( TestSetToEmptyArray );
	RunTest( TestArraySwapTyped );
	TestArenaClearBinaryData();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
