#include "tier0/convar.h"
#include "tier0/icvar.h"
#include "interfaces/interfaces.h"

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

static bool IsInside( const CCommand &command, const void *p )
{
	return p >= (const void *)&command && p < (const void *)( &command + 1 );
}

// A copy tokenizes the other's command string again, so its arguments sit in its own buffers
static void TestCopy()
{
	CCommand *pSrc = new CCommand;
	pSrc->Tokenize( "say hello world" );

	CCommand copy( *pSrc );
	CHECK( copy.ArgC() == 3 && !strcmp( copy.Arg( 1 ), "hello" ) && !strcmp( copy.ArgS(), "hello world" ) );
	CHECK( IsInside( copy, copy.Arg( 1 ) ) && !IsInside( *pSrc, copy.Arg( 1 ) ) );

	CCommand assigned;
	assigned.Tokenize( "x" );
	assigned = *pSrc;
	CHECK( assigned.ArgC() == 3 && !strcmp( assigned.Arg( 2 ), "world" ) );
	CHECK( IsInside( assigned, assigned.Arg( 2 ) ) );

	delete pSrc;
	CHECK( !strcmp( copy.Arg( 2 ), "world" ) && !strcmp( assigned.Arg( 1 ), "hello" ) );

	CCommand empty, emptyCopy( empty );
	CHECK( emptyCopy.ArgC() == 0 && !strcmp( emptyCopy.GetCommandString(), "" ) );
}

// An argument holding a break character is one argument in the source and splits in the copy
static void TestCopyRetokenizes()
{
	const char *ppArgV[] = { "echo", "a(b" };
	CCommand command( 2, ppArgV );
	CHECK( command.ArgC() == 2 );

	CCommand copy( command );
	CHECK( copy.ArgC() == 4 && !strcmp( copy.Arg( 1 ), "a" ) && !strcmp( copy.Arg( 2 ), "(" ) );

	// Only an early return keeps the source's own arguments
	CCommand &self = command;
	command = self;
	CHECK( command.ArgC() == 2 && !strcmp( command.Arg( 1 ), "a(b" ) );
}

// The buffers hold 512 characters, so a command of 510 fits and is copied whole
static void TestFullLength()
{
	char szCommand[512];
	memset( szCommand, 'a', sizeof( szCommand ) );
	memcpy( szCommand, "echo ", 5 );
	szCommand[511] = '\0';

	CCommand command;
	CHECK( !command.Tokenize( szCommand ) );

	szCommand[510] = '\0';
	CHECK( command.Tokenize( szCommand ) && command.ArgC() == 2 );

	CCommand copy( command ), assigned;
	assigned = command;
	CHECK( copy.ArgC() == 2 && strlen( copy.Arg( 1 ) ) == 505 );
	CHECK( assigned.ArgC() == 2 && strlen( assigned.Arg( 1 ) ) == 505 );
}

int main()
{
	// Tokenize's default break set is the cvar system's, which tier0 has
	g_pCVar = (ICvar *)CreateInterface( CVAR_INTERFACE_VERSION, nullptr );
	if ( !g_pCVar || g_pCVar->Init() != INIT_OK )
	{
		printf( "no cvar system\n" );
		return 1;
	}

	TestCopy();
	TestCopyRetokenizes();
	TestFullLength();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
