//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base class for objects that are kept in synch between client and server
//
//=============================================================================

#ifndef SHAREDOBJECT_H
#define SHAREDOBJECT_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utlbuffer.h"
#include "tier0/utlvector.h"
#include "gcsdk/soid.h"

#include <string>

namespace GCSDK
{

class CSharedObject;
// AMNOTE: The game registers one factory per type id (its own CreateSharedObjectSubclass<T> instantiations)
// and creates the objects it parses from GC messages through that registry by type id
typedef CSharedObject *(*SOCreationFunc_t)( );

//----------------------------------------------------------------------------
// Purpose: Abstract base class for objects that are shared between the GC and
//			a gameserver/client. These can also be stored in the database.
//----------------------------------------------------------------------------
// AMNOTE: The game creates its shared objects through its own factories (see SOCreationFunc_t); an object
// constructed in a plugin has the plugin's vtables, not the game's
abstract_class CSharedObject
{
public:
	virtual ~CSharedObject() {}

	virtual int GetTypeID() const = 0;
	// AMNOTE: The client caches pass an empty owner
	virtual bool BParseFromMessage( SOID_t owner, const CUtlBuffer &buffer ) = 0;
	virtual bool BUpdateFromNetwork( const CSharedObject &objUpdate ) = 0;
	virtual bool BIsKeyLess( const CSharedObject &soRHS ) const = 0;
	virtual void Copy( const CSharedObject &soRHS ) = 0;
	virtual void Dump() const = 0;
	// AMNOTE: Returns the vecIndices entry of the object in vecObjects whose key equals this one's, or -1
	virtual int FindKeyIndex( const CUtlVector<CSharedObject *> &vecObjects, const CUtlVector<int> &vecIndices ) const = 0;
	// AMNOTE: bUnk is only read by some of the game's subclasses, which pass it on to their protobuf serialization
	virtual bool BAddToMessage( std::string *pBuffer, bool bUnk ) const = 0;
	virtual bool BAddDestroyToMessage( std::string *pBuffer ) const = 0;

	bool BIsKeyEqual( const CSharedObject &soRHS ) const { return !BIsKeyLess( soRHS ) && !soRHS.BIsKeyLess( *this ); }
};

typedef CUtlVectorFixedGrowable<CSharedObject *, 1> CSharedObjectVec;

} // namespace GCSDK

#endif // SHAREDOBJECT_H
