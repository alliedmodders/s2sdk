#include "tier1/keyvalues3.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlstring.h"

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
	RunTest( TestArrayInsert );
	RunTest( TestArrayRemove );
	TestFreeKV();
	RunTest( TestRemoveMembers );
	RunTest( TestRoundTrips );

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
