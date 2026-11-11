#include "tier0/convar.h"
#include "tier0/icvar.h"
#include "interfaces/interfaces.h"
#include "tier0/logging.h"

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

ICvar *g_pCVar = nullptr;

// Counts the logged messages that contain a text
class CLogCounter : public ILoggingListener
{
public:
	CLogCounter( const char *pszText ) : m_pszText( pszText ), m_nCount( 0 ) { LoggingSystem_RegisterLoggingListener( this ); }
	~CLogCounter() { LoggingSystem_UnregisterLoggingListener( this ); }

	void Log( const LoggingContext_t *pContext, const tchar *pMessage ) override
	{
		if ( strstr( pMessage, m_pszText ) )
			++m_nCount;
	}

	const char *m_pszText;
	int m_nCount;
};

static int s_nStringChanges = 0;
static void OnStringChanged( CConVar<CUtlString> *pCvar, CSplitScreenSlot nSlot, const CUtlString *pNewValue, const CUtlString *pOldValue )
{
	++s_nStringChanges;
}

static int s_nFilterSlot = -2;
static bool s_bAllowChange = true;
static bool FilterInt( CConVar<int32> *pCvar, CSplitScreenSlot nSlot, const int32 *pNewValue, const int32 *pOldValue )
{
	s_nFilterSlot = nSlot.Get();
	return s_bAllowChange;
}

static CConVar<float32> test_float( "test_float", FCVAR_NONE, "", 1.5f );
static CConVar<float64> test_double( "test_double", FCVAR_NONE, "", 0.1 );
static CConVar<Vector> test_vector( "test_vector", FCVAR_NONE, "", Vector( 1.0f, 2.5f, -3.0f ) );
static CConVar<int32> test_int( "test_int", FCVAR_NONE, "", 5 );
static CConVar<int32> test_clamped( "test_clamped", FCVAR_NONE, "", 5, true, 0, true, 10 );
static CConVar<int32> test_filtered( "test_filtered", FCVAR_NONE, "", 1, false, 0, false, 0, nullptr, FilterInt );
static CConVar<CUtlString> test_string( "test_string", FCVAR_NONE, "", CUtlString( "Hello" ), OnStringChanged );

static int s_nContextCalls = 0, s_nNoContextCalls = 0, s_nVoidCalls = 0;
static void CommandWithContext( const CCommandContext &context, const CCommand &command )
{
	s_nContextCalls += context.GetPlayerSlot().Get() == 3 && command.ArgC() == 2;
}

static void CommandNoContext( const CCommand &command )
{
	s_nNoContextCalls += command.ArgC() == 2;
}

static void CommandVoid()
{
	++s_nVoidCalls;
}

static ConCommand test_cmd_context( "test_cmd_context", CommandWithContext, "" );
static ConCommand test_cmd_nocontext( "test_cmd_nocontext", CommandNoContext, "" );
static ConCommand test_cmd_void( "test_cmd_void", CommandVoid, "" );

static bool ValueStringIs( const ConVarRefAbstract &cvar, const char *pszExpected )
{
	CBufferString buf;
	cvar.GetValueAsString( buf );
	return !strcmp( buf.Get(), pszExpected );
}

// Floats print in their shortest form, as the game prints them
static void TestValueToString()
{
	CHECK( ValueStringIs( test_float, "1.5" ) );
	CHECK( ValueStringIs( test_double, "0.1" ) );
	CHECK( ValueStringIs( test_vector, "1 2.5 -3" ) );

	CBufferString buf;
	test_float.GetDefaultAsString( buf );
	CHECK( !strcmp( buf.Get(), "1.5" ) );
}

// Leading whitespace is skipped, trailing whitespace or junk fails the parse and keeps the value
static void TestSetString()
{
	CHECK( test_int.SetString( " 7" ) && test_int.Get() == 7 );
	CHECK( !test_int.SetString( "8 " ) && test_int.Get() == 7 );
	CHECK( !test_int.SetString( "9x" ) && test_int.Get() == 7 );
	CHECK( test_vector.SetString( "\t4 5 6" ) && test_vector.Get() == Vector( 4.0f, 5.0f, 6.0f ) );

	test_int.Revert();
	test_vector.Revert();
	CHECK( test_int.Get() == 5 && test_int.IsSetToDefault() );
	CHECK( ValueStringIs( test_vector, "1 2.5 -3" ) );
}

static void TestClamp()
{
	test_clamped.Set( 20 );
	CHECK( test_clamped.Get() == 10 );
	CHECK( test_clamped.SetString( "-4" ) && test_clamped.Get() == 0 );
	test_clamped.Revert();
	CHECK( test_clamped.Get() == 5 );
}

// Strings compare without case: a change of case alone is stored but isn't a change
static void TestStringEquality()
{
	s_nStringChanges = 0;
	test_string.Set( CUtlString( "HELLO" ) );
	CHECK( s_nStringChanges == 0 && !strcmp( test_string.Get().Get(), "HELLO" ) && test_string.IsSetToDefault() );

	test_string.Set( CUtlString( "World" ) );
	CHECK( s_nStringChanges == 1 && !strcmp( test_string.Get().Get(), "World" ) );
	test_string.Revert();
	CHECK( s_nStringChanges == 2 && test_string.IsSetToDefault() );
}

// Reading a string convar as another type parses it without logging errors
static void TestGetAsFromString()
{
	CLogCounter errors( "Error parsing" );

	test_string.Set( CUtlString( "12" ) );
	CHECK( test_string.GetInt() == 12 && test_string.GetFloat() == 12.0f );

	test_string.Set( CUtlString( "abc" ) );
	CHECK( test_string.GetInt() == 0 );
	CHECK( errors.m_nCount == 0 );

	test_string.Revert();
}

// Defaults and limits read typed and as strings; an unset limit empties the buffer
static void TestLimits()
{
	CHECK( test_clamped.GetDefault() == 5 && test_clamped.GetMin() == 0 && test_clamped.GetMax() == 10 );
	CHECK( test_clamped.HasMin() && test_clamped.HasMax() && !test_int.HasMin() && !test_int.HasMax() );

	CBufferString buf;
	test_clamped.GetMinAsString( buf );
	CHECK( !strcmp( buf.Get(), "0" ) );
	test_clamped.GetMaxAsString( buf );
	CHECK( !strcmp( buf.Get(), "10" ) );

	buf.Insert( 0, "stale" );
	test_int.GetMinAsString( buf );
	CHECK( !strcmp( buf.Get(), "" ) );
	buf.Insert( 0, "stale" );
	test_int.GetMaxAsString( buf );
	CHECK( !strcmp( buf.Get(), "" ) );
}

// SetAs and GetAs convert between primitive types, and to and from string convars
static void TestConversions()
{
	test_int.SetFloat( 7.9f );
	CHECK( test_int.Get() == 7 && test_int.GetFloat() == 7.0f && test_int.GetBool() );
	CHECK( !strcmp( test_int.GetString().Get(), "7" ) );

	test_int.SetBool( false );
	CHECK( test_int.Get() == 0 && !test_int.IsSetToDefault() && !test_int.IsAllSetToDefault() );
	test_int.Revert();
	CHECK( test_int.IsAllSetToDefault() );

	test_string.SetFloat( 2.5f );
	CHECK( !strcmp( test_string.Get().Get(), "2.5" ) && test_string.GetFloat() == 2.5f );
	test_string.SetInt( 3 );
	CHECK( !strcmp( test_string.Get().Get(), "3" ) && test_string.GetInt() == 3 );
	test_string.Revert();

	// A vector convar isn't primitive: setting it from a number does nothing
	test_vector.SetInt( 1 );
	CHECK( ValueStringIs( test_vector, "1 2.5 -3" ) );
}

// A typed reference finds a convar by name; one of the wrong type gets invalid data
static void TestReferences()
{
	CConVarRef<int32> ref( "test_int" );
	CHECK( ref.IsConVarDataAvailable() && ref.Get() == 5 );
	ref.Set( 6 );
	CHECK( test_int.Get() == 6 );
	test_int.Revert();

	const ConVarRefAbstract &intRef = test_int;
	CConVarRef<float32> wrongType( intRef );
	CHECK( !wrongType.IsConVarDataValid() );
}

// The default slot reaches the filter as slot 0, and a rejected change keeps the value
static void TestFilter()
{
	test_filtered.Set( 2 );
	CHECK( s_nFilterSlot == 0 && test_filtered.Get() == 2 );

	CLogCounter warnings( "Can't change 'test_filtered' right now." );
	s_bAllowChange = false;
	test_filtered.Set( 3 );
	CHECK( test_filtered.Get() == 2 && warnings.m_nCount == 1 );
	s_bAllowChange = true;
}

// Each kind of command callback is called with its own arguments
static void TestDispatch()
{
	CCommand command;
	command.Tokenize( "test arg" );
	CCommandContext context( CT_NO_TARGET, CPlayerSlot( 3 ) );

	test_cmd_context.Dispatch( context, command );
	test_cmd_nocontext.Dispatch( context, command );
	test_cmd_void.Dispatch( context, command );
	CHECK( s_nContextCalls == 1 && s_nNoContextCalls == 1 && s_nVoidCalls == 1 );
}

int main()
{
	g_pCVar = (ICvar *)CreateInterface( CVAR_INTERFACE_VERSION, nullptr );
	// Connecting it fills tier0's own cvar system pointer, which registration uses
	if ( !g_pCVar || !g_pCVar->Connect( CreateInterface ) || g_pCVar->Init() != INIT_OK )
	{
		printf( "no cvar system\n" );
		return 1;
	}

	ConVar_Register();

	TestValueToString();
	TestSetString();
	TestClamp();
	TestStringEquality();
	TestGetAsFromString();
	TestLimits();
	TestConversions();
	TestReferences();
	TestFilter();
	TestDispatch();

	ConVar_Unregister();
	g_pCVar->Shutdown();
	g_pCVar->Disconnect();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
