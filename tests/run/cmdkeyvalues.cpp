#include "netmessages.h"
#include "networkbasetypes.pb.h"
#include "netmessages.pb.h"

#include <stdio.h>
#include <string.h>
#include <string>

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

class CTestConfig : public CBaseCmdKeyValues<CSVCMsg_GameSessionConfiguration>
{
public:
	KeyValues *Stored() const { return m_pKeyValues; }
};

class CTestMsg : public CBaseCmdKeyValues<CCLCMsg_CmdKeyValues>
{
public:
	using CBaseCmdKeyValues::CBaseCmdKeyValues;
	KeyValues *Stored() const { return m_pKeyValues; }
};

// Null for a KeyValues on the global string table
class CTestKeyValues : public KeyValues
{
public:
	static IKeyValuesSystem *System( KeyValues *pKV ) { return pKV->*( &CTestKeyValues::m_pKVSystem ); }
};

static KeyValues *MakeSample()
{
	KeyValues *pKV = new KeyValues( "sample" );
	pKV->SetString( "mode", "listenserver" );
	pKV->SetInt( "instance", 3 );
	pKV->FindKey( "sub", true )->SetString( "x", "y" );
	return pKV;
}

static bool IsSample( KeyValues *pKV )
{
	return pKV && strcmp( pKV->GetName(), "sample" ) == 0 && strcmp( pKV->GetString( "mode" ), "listenserver" ) == 0
		&& pKV->GetInt( "instance" ) == 3 && pKV->FindKey( "sub" ) && strcmp( pKV->FindKey( "sub" )->GetString( "x" ), "y" ) == 0;
}

// Constructed without KeyValues, the data field is set but empty and parses to an empty KeyValues once
static void TestDefault()
{
	CTestConfig config;
	CHECK( config.Stored() == NULL );
	CHECK( config.has_data() && config.data().empty() );
	CHECK( !config.HasKeyValues() );

	KeyValues *pKV = config.GetKeyValues();
	CHECK( pKV != NULL && config.Stored() == pKV );
	CHECK( pKV && pKV->GetFirstSubKey() == NULL );
	CHECK( pKV && CTestKeyValues::System( pKV ) != NULL && CTestKeyValues::System( pKV ) != KeyValuesSystem() );
	CHECK( !config.HasKeyValues() );
	CHECK( config.GetKeyValues() == pKV );

	pKV->SetString( "added", "later" );
	CHECK( strcmp( config.GetKeyValues()->GetString( "added" ), "later" ) == 0 );
}

// What SetKeyValues serializes parses back from the data field alone, also after a trip over the wire
static void TestRoundTrip()
{
	CTestConfig config;
	KeyValues *pKV = MakeSample();
	CHECK( CTestKeyValues::System( pKV ) == NULL );
	config.SetKeyValues( pKV );
	CHECK( config.Stored() == pKV && config.GetKeyValues() == pKV );
	CHECK( config.HasKeyValues() );

	CTestMsg msg;
	msg.set_data( config.data() );
	CHECK( msg.Stored() == NULL && msg.HasKeyValues() );
	CHECK( IsSample( msg.GetKeyValues() ) );

	CTestMsg wire;
	CHECK( wire.ParseFromString( CTestMsg( MakeSample() ).SerializeAsString() ) );
	CHECK( IsSample( wire.GetKeyValues() ) );
}

static void TestReplace()
{
	CTestConfig config;
	config.SetKeyValues( MakeSample() );
	std::string sample = config.data();

	config.SetKeyValues( new KeyValues( "other", "only", "key" ) );
	CHECK( config.data() != sample );
	CHECK( strcmp( config.GetKeyValues()->GetString( "only" ), "key" ) == 0 );

	config.SetKeyValues( NULL );
	CHECK( config.Stored() == NULL );
	CHECK( config.has_data() && config.data().empty() && !config.HasKeyValues() );
	CHECK( config.GetKeyValues() != NULL && config.GetKeyValues()->GetFirstSubKey() == NULL );
}

// A copy owns its own KeyValues, and one not parsed yet stays unparsed
static void TestCopy()
{
	CTestConfig *pOriginal = new CTestConfig;
	pOriginal->SetKeyValues( MakeSample() );
	pOriginal->set_hostname( "host" );

	CTestConfig copy( *pOriginal );
	CHECK( copy.Stored() != NULL && copy.Stored() != pOriginal->Stored() );
	CHECK( copy.data() == pOriginal->data() );
	CHECK( copy.hostname() == "host" );
	delete pOriginal;
	CHECK( IsSample( copy.GetKeyValues() ) );

	CTestMsg unparsed;
	unparsed.set_data( copy.data() );
	CTestMsg copy2( unparsed );
	CHECK( copy2.Stored() == NULL && unparsed.Stored() == NULL );
	CHECK( IsSample( copy2.GetKeyValues() ) );
}

// Assignment copies the KeyValues and nothing else, and a source not parsed yet empties the data
static void TestAssign()
{
	CTestConfig source;
	source.SetKeyValues( MakeSample() );
	source.set_hostname( "source" );

	CTestConfig target;
	target.set_hostname( "target" );
	target = source;
	CHECK( target.Stored() != NULL && target.Stored() != source.Stored() );
	CHECK( IsSample( target.Stored() ) );
	CHECK( target.data() == source.data() );
	CHECK( target.hostname() == "target" );

	const CTestConfig &self = target;
	target = self;
	CHECK( IsSample( target.Stored() ) );

	CTestConfig unparsed;
	unparsed.set_data( source.data() );
	target = unparsed;
	CHECK( target.Stored() == NULL );
	CHECK( target.has_data() && !target.HasKeyValues() );

	const CTestConfig &unparsedSelf = unparsed;
	unparsed = unparsedSelf;
	CHECK( !unparsed.HasKeyValues() );
}

// Data that isn't binary KeyValues, as a client can send, still gives a KeyValues
static void TestBadData()
{
	CTestMsg garbage;
	garbage.set_data( std::string( "\x07garbage\xff\x01\x00\x02", 12 ) );
	CHECK( garbage.HasKeyValues() );
	CHECK( garbage.GetKeyValues() != NULL );

	CTestMsg truncated;
	truncated.set_data( CTestMsg( MakeSample() ).data().substr( 0, 10 ) );
	CHECK( truncated.GetKeyValues() != NULL );

	CTestMsg absent;
	absent.clear_data();
	CHECK( !absent.has_data() && !absent.HasKeyValues() );
	CHECK( absent.GetKeyValues() != NULL && absent.GetKeyValues()->GetFirstSubKey() == NULL );
}

int main()
{
	TestDefault();
	TestRoundTrip();
	TestReplace();
	TestCopy();
	TestAssign();
	TestBadData();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
