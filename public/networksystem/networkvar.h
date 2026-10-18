#ifndef NETWORKVAR_H
#define NETWORKVAR_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utldelegate.h"
#include "tier0/utlvector.h"
#include "entity2/entityinstance.h"
#include "networksystem/networksystemtypes.h"

class CNetworkVarChainer : public CEntityOwnerPtr
{
public:
	CNetworkVarChainer() : m_bForwardStateChanges( false ) {}

	struct ChainUpdatePropagation_t
	{
		CUtlDelegate<void ( const CNetworkVarChainer & )> updateDelegate;
		// The networked member updateDelegate is bound to: its name, and its offset in the chainer's owner
		const char* m_pszFieldName;
		uint32 m_nFieldOffset;
	};

	void RegisterChainUpdatePropagationTarget( ChainUpdatePropagation_t& target ) { m_PropagationList.AddToTail( target ); }

	// Points each target at its member of pOwner, then calls it.
	// AMNOTE: bAssignChangeAccessorPathIds has the entity fill in m_PathIndex
	void Set( CEntityInstance* pInstance, bool bAssignChangeAccessorPathIds, void* pOwner );

	// These pass the call on to the entity, unless there is none or it is being constructed.
	// NetworkStateChanged fills in data's path index when it has none
	void NetworkStateChanged( NetworkStateChangedData& data );
	void NetworkStateChangedRemove( const NetworkStateChangedRemove_t& data );
	void NetworkStateChangedLog( const char* pszFieldName, const char* pszInfo );

public:
	CUtlVector<ChainUpdatePropagation_t> m_PropagationList;
	ChangeAccessorFieldPathIndex_t m_PathIndex;
	// AMNOTE: Checked by the owner, not by the methods above. A struct with its own chainer passes its state changes to it
	// only while this is set, which a networked vector does for its elements
	bool m_bForwardStateChanges;
};

inline void CNetworkVarChainer::Set( CEntityInstance* pInstance, bool bAssignChangeAccessorPathIds, void* pOwner )
{
	CEntityOwnerPtr::Set( pInstance );

	for ( ChainUpdatePropagation_t& target : m_PropagationList )
	{
		CUtlAbstractDelegate delegate = target.updateDelegate.GetAbstractDelegate();
		delegate.UnsafeThisPointerSlam( (uint8*)pOwner + target.m_nFieldOffset );
		target.updateDelegate.SetAbstractDelegate( delegate );
		target.updateDelegate( *this );
	}

	if ( pInstance && bAssignChangeAccessorPathIds )
		pInstance->AssignChangeAccessorPathIds();
}

inline void CNetworkVarChainer::NetworkStateChanged( NetworkStateChangedData& data )
{
	CEntityInstance* pEntity = *this;
	if ( !pEntity || ( pEntity->m_pEntity->m_flags & EF_IS_CONSTRUCTION_IN_PROGRESS ) != 0 )
		return;

	ChangeAccessorFieldPathIndex_t nPathIndex = data.m_nPathIndex.m_Value >= 0 ? data.m_nPathIndex : m_PathIndex;
	if ( data.m_bRequiresPathIndex && nPathIndex.m_Value < 0 )
		return;

	data.m_nPathIndex = nPathIndex;
	pEntity->NetworkStateChanged( data );
}

inline void CNetworkVarChainer::NetworkStateChangedRemove( const NetworkStateChangedRemove_t& data )
{
	CEntityInstance* pEntity = *this;
	if ( pEntity && ( pEntity->m_pEntity->m_flags & EF_IS_CONSTRUCTION_IN_PROGRESS ) == 0 )
		pEntity->NetworkStateChangedRemove( data );
}

inline void CNetworkVarChainer::NetworkStateChangedLog( const char* pszFieldName, const char* pszInfo )
{
	CEntityInstance* pEntity = *this;
	if ( pEntity && ( pEntity->m_pEntity->m_flags & EF_IS_CONSTRUCTION_IN_PROGRESS ) == 0 )
		pEntity->NetworkStateChangedLog( pszFieldName, pszInfo );
}

#endif // NETWORKVAR_H
