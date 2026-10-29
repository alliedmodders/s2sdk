#ifndef NETWORKVAR_H
#define NETWORKVAR_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utldelegate.h"
#include "tier0/utlvector.h"
#include "tier2/fieldpath.h"
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

// AMNOTE: game's template also takes the field's NetworkVar tag class and an optional fixed element count;
// Prefer to use CSchemaType_Atomic_CollectionOfT manipulator functions instead of direct access methods here for correct networking support.
template <class T>
class CNetworkUtlVectorBase
{
public:
	int Count() const { return m_Vector.Count(); }
	bool IsValidIndex( int i ) const { return m_Vector.IsValidIndex( i ); }
	const T &operator[]( int i ) const { return m_Vector[i]; }
	const T &Element( int i ) const { return m_Vector[i]; }
	const T *Base() const { return m_Vector.Base(); }
	const CUtlVector<T> &Get() const { return m_Vector; }

private:
	CUtlVector<T> m_Vector;
};

template <class T>
class CUtlVectorEmbeddedNetworkVar_Vector
{
protected:
	CUtlVector<T> m_Vector;
};

template <class T, bool bChained>
class CUtlVectorEmbeddedNetworkVar_ChainingHelper : public CUtlVectorEmbeddedNetworkVar_Vector<T>
{
};

template <class T>
class CUtlVectorEmbeddedNetworkVar_ChainingHelper<T, true> : public CUtlVectorEmbeddedNetworkVar_Vector<T>
{
protected:
	// Registered with the owner's chainer, which points m_pOwnerEntity at the owner's entity whenever its chain changes
	CNetworkVarChainer::ChainUpdatePropagation_t m_PropagationEntry;
};

// AMNOTE: Prefer to use CSchemaType_Atomic_CollectionOfT manipulator functions instead of direct access methods here for correct networking support.
template <class T, bool bChained = false>
class CUtlVectorEmbeddedNetworkVar : public CUtlVectorEmbeddedNetworkVar_ChainingHelper<T, bChained>
{
	typedef CUtlVectorEmbeddedNetworkVar_ChainingHelper<T, bChained> BaseClass;

public:
	int Count() const { return BaseClass::m_Vector.Count(); }
	bool IsValidIndex( int i ) const { return BaseClass::m_Vector.IsValidIndex( i ); }
	const T &operator[]( int i ) const { return BaseClass::m_Vector[i]; }
	const T &Element( int i ) const { return BaseClass::m_Vector[i]; }
	const T *Base() const { return BaseClass::m_Vector.Base(); }
	const CUtlVector<T> &Get() const { return BaseClass::m_Vector; }

private:
	CEntityOwnerPtr m_pOwnerEntity;
	// Called when the element count changes
	CUtlDelegate<void()> *m_pArraySizeChangedDelegate;
	CFieldPath m_PathToVector;
	// The vector field's local offsets, reported with an element's own path index when the element changes
	CUtlVector<uint32> m_LocalOffsets;
	// Written by the field path resolve: the field isn't networked, so changes aren't reported
	bool m_bIsNetworkVarExcluded;
	bool m_bResolved : 1;
	// The last resolve failed (logged); a later one that succeeds clears it
	bool m_bResolveFailed : 1;
	bool m_unk101 : 1;
	// Whether the owner's fields are all trivial POD types, valid once m_bTrivialPODChecked is set; element changes are
	// reported with the element's own path only when it is
	bool m_bTrivialPODChecked : 1;
	bool m_bTrivialPOD : 1;
};

#endif // NETWORKVAR_H
