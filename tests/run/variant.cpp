#include "entity2/variant.h"

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

static bool StringIs( const CVariant &variant, const char *pszExpected )
{
	CBufferStringN<64> buf;
	return variant.AssignTo( buf ) && strcmp( buf.Get(), pszExpected ) == 0;
}

// Each constructor stores its type, and the matching conversion reads the value back
static void TestTypes()
{
	CVariant empty;
	CHECK( empty.IsNull() && empty.GetType() == FIELD_VOID );

	CHECK( CVariant( (uint8)200 ).GetType() == FIELD_UINT8 && (uint8)CVariant( (uint8)200 ) == 200 );
	CHECK( CVariant( (int16)-5 ).GetType() == FIELD_INT16 && (int16)CVariant( (int16)-5 ) == -5 );
	CHECK( CVariant( (int32)-7 ).GetType() == FIELD_INT32 && (int32)CVariant( (int32)-7 ) == -7 );
	CHECK( CVariant( (uint32)7 ).GetType() == FIELD_UINT32 );
	CHECK( CVariant( (int64)-9 ).GetType() == FIELD_INT64 && (int64)CVariant( (int64)-9 ) == -9 );
	CHECK( CVariant( (uint64)9 ).GetType() == FIELD_UINT64 );
	CHECK( CVariant( 1.25f ).GetType() == FIELD_FLOAT32 && (float32)CVariant( 1.25f ) == 1.25f );
	CHECK( CVariant( 2.5 ).GetType() == FIELD_FLOAT64 && (float64)CVariant( 2.5 ) == 2.5 );
	CHECK( CVariant( 'x' ).GetType() == FIELD_CHARACTER && (char)CVariant( 'x' ) == 'x' );
	CHECK( CVariant( true ).GetType() == FIELD_BOOLEAN && (bool)CVariant( true ) );
	CHECK( CVariant( "text" ).GetType() == FIELD_CSTRING && strcmp( (const char *)CVariant( "text" ), "text" ) == 0 );
	CHECK( CVariant( Color( 1, 2, 3, 4 ) ).GetType() == FIELD_COLOR32 );

	Vector vec( 1, 2, 3 );
	CVariant vector( vec );
	CHECK( vector.GetType() == FIELD_VECTOR && (const Vector &)vector == vec );

	CVariant assigned;
	assigned = (int32)5;
	CHECK( assigned.GetType() == FIELD_INT32 && (int32)assigned == 5 );
	assigned = 0.5f;
	CHECK( assigned.GetType() == FIELD_FLOAT32 && (float32)assigned == 0.5f );
}

// Numbers convert to int, float and bool as the game does: 64-bit and unsigned values clamp to the int range
static void TestNumberConversions()
{
	int nValue = 0;
	CHECK( CVariant( (int64)5000000000LL ).AssignTo( &nValue ) && nValue == 0x7FFFFFFF );
	CHECK( CVariant( (int64)-5000000000LL ).AssignTo( &nValue ) && nValue == (int)0x80000000 );
	CHECK( CVariant( (uint64)5000000000ULL ).AssignTo( &nValue ) && nValue == 0x7FFFFFFF );
	CHECK( CVariant( (uint32)0x90000000u ).AssignTo( &nValue ) && nValue == 0x7FFFFFFF );
	CHECK( CVariant( (uint8)250 ).AssignTo( &nValue ) && nValue == 250 );
	CHECK( CVariant( -3.75f ).AssignTo( &nValue ) && nValue == -3 );
	CHECK( !CVariant().AssignTo( &nValue ) && nValue == 0 );
	nValue = 77;
	CHECK( !CVariant( Color( 1, 2, 3, 4 ) ).AssignTo( &nValue ) && nValue == 77 );

	float flValue = 0.0f;
	CHECK( CVariant( (int32)-4 ).AssignTo( &flValue ) && flValue == -4.0f );
	CHECK( CVariant( (uint32)0x80000000u ).AssignTo( &flValue ) && flValue == 2147483648.0f );
	CHECK( CVariant( 0.25 ).AssignTo( &flValue ) && flValue == 0.25f );
	CHECK( CVariant( true ).AssignTo( &flValue ) && flValue == 1.0f );

	bool bValue = false;
	CHECK( CVariant( (uint8)3 ).AssignTo( &bValue ) && bValue );
	CHECK( CVariant( (uint64)0 ).AssignTo( &bValue ) && !bValue );
	CHECK( CVariant( 0.5f ).AssignTo( &bValue ) && bValue );

	uint64 nWide = 1;
	CHECK( CVariant( (int32)-1 ).AssignTo( &nWide ) && nWide == 0xFFFFFFFFFFFFFFFFULL );
	CHECK( CVariant( (uint8)9 ).AssignTo( &nWide ) && nWide == 9 );
	CHECK( !CVariant().AssignTo( &nWide ) && nWide == 0 );
	CHECK( CVariant( 'a' ).Get<uint64>() == 'a' );
	CHECK( CVariant( (int64)-2 ).Get<int>() == -2 && CVariant( 3.0f ).Get<float>() == 3.0f );
}

// Strings parse into numbers, booleans and vectors; an empty string converts to the zero value
static void TestStringConversions()
{
	int nValue = 0;
	CHECK( CVariant( "42" ).AssignTo( &nValue ) && nValue == 42 );
	CHECK( CVariant( "" ).AssignTo( &nValue ) && nValue == 0 );

	string_t name = MAKE_STRING( "17" );
	CHECK( CVariant( name ).AssignTo( &nValue ) && nValue == 17 );
	CHECK( !CVariant( string_t() ).AssignTo( &nValue ) && nValue == 0 );

	float flValue = 0.0f;
	CHECK( CVariant( "1.5" ).AssignTo( &flValue ) && flValue == 1.5f );

	bool bValue = true;
	CHECK( CVariant( "true" ).AssignTo( &bValue ) && bValue );
	CHECK( !CVariant( "" ).AssignTo( &bValue ) && !bValue );

	Vector vec;
	CHECK( CVariant( "1 2 3" ).AssignTo( &vec ) && vec == Vector( 1, 2, 3 ) );
	vec.Init( 5, 5, 5 );
	CHECK( !CVariant( "" ).AssignTo( &vec ) && vec == vec3_origin );

	QAngle ang( 10, 20, 30 );
	CVariant angle( ang );
	CHECK( angle.AssignTo( &vec ) && vec == Vector( 10, 20, 30 ) );

	Vector4D vec4( 1, 1, 1, 1 );
	CHECK( !CVariant( "" ).AssignTo( &vec4 ) && vec4 == vec4_origin );
}

// The text forms of every printable type
static void TestToString()
{
	CHECK( StringIs( CVariant( (int32)-12 ), "-12" ) );
	CHECK( StringIs( CVariant( (uint32)4000000000u ), "4000000000" ) );
	CHECK( StringIs( CVariant( (int64)-5000000000LL ), "-5000000000" ) );
	CHECK( StringIs( CVariant( 0.5f ), "0.5" ) );
	CHECK( StringIs( CVariant( true ), "true" ) );
	CHECK( StringIs( CVariant( 'q' ), "q" ) );
	CHECK( StringIs( CVariant( Color( 1, 2, 3, 4 ) ), "1 2 3 4" ) );
	CHECK( StringIs( CVariant( "abc" ), "abc" ) );
	CHECK( StringIs( CVariant( string_t() ), "(null)" ) );

	Vector vec( 1, 2.5f, -3 );
	CHECK( StringIs( CVariant( vec ), "1 2.5 -3" ) );

	CBufferStringN<64> buf;
	buf.Insert( 0, "stale" );
	CHECK( !CVariant().AssignTo( buf ) && buf.Length() == 0 );

	CHECK( strcmp( CVariant( (int32)8 ).ToString(), "8" ) == 0 );
	CHECK( strcmp( CVariant( string_t() ).ToString(), "" ) == 0 );
	CHECK( strcmp( VariantFieldTypeName( FIELD_INT32 ), "integer" ) == 0 && strcmp( VariantFieldTypeName( FIELD_FLOAT32 ), "float" ) == 0 );
}

// Copies own their vector data, Convert changes the stored type, and ConvertToCopiedData detaches from the source
static void TestCopiesAndConvert()
{
	Vector vec( 1, 2, 3 );
	CVariant source( vec );
	CHECK( ( source.GetFlags() & SV_FREE ) == 0 );

	CVariant copy( source );
	CHECK( ( copy.GetFlags() & SV_FREE ) != 0 && (const Vector &)copy == vec );
	vec.Init( 9, 9, 9 );
	CHECK( (const Vector &)copy == Vector( 1, 2, 3 ) && (const Vector &)source == vec );

	source.ConvertToCopiedData();
	CHECK( ( source.GetFlags() & SV_FREE ) != 0 );
	vec.Init( 0, 0, 0 );
	CHECK( (const Vector &)source == Vector( 9, 9, 9 ) );

	CVariant number( 2.75f );
	CHECK( number.Convert( FIELD_INT32 ) && number.GetType() == FIELD_INT32 && (int32)number == 2 );
	CHECK( number.Convert( FIELD_FLOAT64 ) && (float64)number == 2.0 );
	CHECK( number.Convert( FIELD_BOOLEAN ) && (bool)number );
	CHECK( number.Convert( FIELD_CSTRING ) && strcmp( (const char *)number, "true" ) == 0 );

	CVariant text( "4 5 6" );
	CHECK( text.Convert( FIELD_VECTOR ) && (const Vector &)text == Vector( 4, 5, 6 ) );

	CVariant unsignedValue( (int64)70000 );
	CHECK( unsignedValue.Convert( FIELD_UINT32 ) && unsignedValue.GetType() == FIELD_UINT32 && (uint32)unsignedValue == 70000 );

	CVariant colorText( "10 20 30 40" );
	CHECK( colorText.Convert( FIELD_COLOR32 ) && (Color)colorText == Color( 10, 20, 30, 40 ) );

	CVariant freed( text );
	freed.Free();
	CHECK( freed.IsNull() && freed.GetFlags() == 0 );

	// Assigning over a variant that owns data frees it first, and assigning a variant to itself keeps it
	Vector owned( 7, 8, 9 );
	CVariant target( owned, true );
	CHECK( ( target.GetFlags() & SV_FREE ) != 0 );
	target = CVariant( (int32)5 );
	CHECK( target.GetType() == FIELD_INT32 && (int32)target == 5 && target.GetFlags() == 0 );

	CVariant self( owned, true );
	const CVariant &selfRef = self;
	self = selfRef;
	CHECK( (const Vector &)self == Vector( 7, 8, 9 ) && ( self.GetFlags() & SV_FREE ) != 0 );

	// A string the variant owns stays valid as a string_t
	CVariant owner( "abc", true );
	CHECK( owner.Convert( FIELD_STRING ) && owner.GetType() == FIELD_STRING && strcmp( owner.ToString(), "abc" ) == 0 );

	// One it doesn't own becomes a string_t to the same text
	static const char s_szBorrowed[] = "def";
	CVariant borrower( s_szBorrowed );
	CHECK( borrower.Convert( FIELD_STRING ) && borrower.GetType() == FIELD_STRING && borrower.ToString() == s_szBorrowed && borrower.GetFlags() == 0 );
}

// Floats convert to integers by truncation, also to unsigned 64-bit values past INT64_MAX
static void TestFloatLimits()
{
	CHECK( CVariant( 2147483520.0f ).Get<int>() == 2147483520 && CVariant( -2.9 ).Get<int>() == -2 );
	CHECK( CVariant( 1.5e19 ).Get<uint64>() == 15000000000000000000ULL );
	CHECK( CVariant( 2.9f ).Get<uint8>() == 2 && CVariant( -7.5 ).Get<int16>() == -7 );
}

// Equal types and values compare equal
static void TestCompare()
{
	CHECK( CVariant( (int32)3 ) == CVariant( (int32)3 ) );
	CHECK( CVariant( (int32)3 ) != CVariant( (int32)4 ) );
	CHECK( CVariant( (int32)3 ) != CVariant( (uint32)3 ) );
	CHECK( CVariant( Color( 1, 2, 3, 4 ) ) == CVariant( Color( 1, 2, 3, 4 ) ) );
	CHECK( CVariant( 1.5f ) == CVariant( 1.5f ) );
}

int main()
{
	TestTypes();
	TestNumberConversions();
	TestStringConversions();
	TestToString();
	TestCopiesAndConvert();
	TestFloatLimits();
	TestCompare();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
