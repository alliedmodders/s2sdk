#include "networksystem/networkvar.h"

#include <stddef.h>
#include <stdio.h>

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

struct Member_t
{
	int m_nCalls = 0;
	const CNetworkVarChainer *m_pLastChainer = nullptr;
	const CEntityInstance *m_pInstanceAtCall = nullptr;

	void OnChainUpdate( const CNetworkVarChainer &chainer )
	{
		++m_nCalls;
		m_pLastChainer = &chainer;
		m_pInstanceAtCall = chainer;
	}
};

// The chainer isn't the first member, so the owner and its chainer have different addresses
struct Owner_t
{
	int m_nValue = 0;
	CNetworkVarChainer m_Chain;
	Member_t m_A;
	Member_t m_B;
};

static void Register( CNetworkVarChainer &chain, Member_t &member, uint32 nOffset )
{
	CNetworkVarChainer::ChainUpdatePropagation_t target;
	target.updateDelegate = CUtlDelegate<void ( const CNetworkVarChainer & )>( &member, &Member_t::OnChainUpdate );
	target.m_pszFieldName = nullptr;
	target.m_nFieldOffset = nOffset;
	chain.RegisterChainUpdatePropagationTarget( target );
}

// Set calls every target, bound to its member of the given owner instead of the object it was registered with
static void TestSetCallsTargets()
{
	Owner_t owner;
	Member_t registered;
	Register( owner.m_Chain, registered, offsetof( Owner_t, m_A ) );
	Register( owner.m_Chain, registered, offsetof( Owner_t, m_B ) );

	owner.m_Chain.Set( nullptr, true, &owner );
	CHECK( owner.m_A.m_nCalls == 1 );
	CHECK( owner.m_B.m_nCalls == 1 );
	CHECK( owner.m_A.m_pLastChainer == &owner.m_Chain );
	CHECK( owner.m_B.m_pLastChainer == &owner.m_Chain );
	CHECK( registered.m_nCalls == 0 );
}

// A copied chainer's targets are bound to the source's members until Set rebinds them to the new owner's
static void TestSetRebindsCopy()
{
	Owner_t source;
	Register( source.m_Chain, source.m_A, offsetof( Owner_t, m_A ) );
	Register( source.m_Chain, source.m_B, offsetof( Owner_t, m_B ) );

	Owner_t copy;
	copy.m_Chain = source.m_Chain;
	copy.m_Chain.Set( nullptr, false, &copy );
	CHECK( copy.m_A.m_nCalls == 1 );
	CHECK( copy.m_B.m_nCalls == 1 );
	CHECK( source.m_A.m_nCalls == 0 );
	CHECK( source.m_B.m_nCalls == 0 );

	copy.m_Chain.m_PropagationList[0].updateDelegate( copy.m_Chain );
	CHECK( copy.m_A.m_nCalls == 2 );
	CHECK( source.m_A.m_nCalls == 0 );
}

// The instance is stored before the targets run. Without bAssignChangeAccessorPathIds it is never dereferenced
static void TestSetStoresInstance()
{
	Owner_t owner;
	Register( owner.m_Chain, owner.m_A, offsetof( Owner_t, m_A ) );

	int nNotAnEntity = 0;
	CEntityInstance *pInstance = reinterpret_cast<CEntityInstance *>( &nNotAnEntity );
	owner.m_Chain.Set( pInstance, false, &owner );
	CHECK( (CEntityInstance *)owner.m_Chain == pInstance );
	CHECK( owner.m_A.m_pInstanceAtCall == pInstance );

	owner.m_Chain.Set( nullptr, false, &owner );
	CHECK( (CEntityInstance *)owner.m_Chain == nullptr );
	CHECK( owner.m_A.m_pInstanceAtCall == nullptr );
}

int main()
{
	TestSetCallsTargets();
	TestSetStoresInstance();
	TestSetRebindsCopy();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
