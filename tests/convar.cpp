#include "tier1/convar.h"

// Every type TranslateConVarType has
#define INSTANTIATE_CONVAR( T ) \
	template class CConVarRef<T>; \
	template class CConVar<T>;

// CUtlString's GetAs and SetAs are specializations
#define INSTANTIATE_CONVAR_ACCESSORS( T ) \
	INSTANTIATE_CONVAR( T ) \
	template const T ConVarRefAbstract::GetAs<T>( CSplitScreenSlot ) const; \
	template void ConVarRefAbstract::SetAs<T>( const T &, CSplitScreenSlot );

INSTANTIATE_CONVAR_ACCESSORS( bool )
INSTANTIATE_CONVAR_ACCESSORS( int16 )
INSTANTIATE_CONVAR_ACCESSORS( uint16 )
INSTANTIATE_CONVAR_ACCESSORS( int32 )
INSTANTIATE_CONVAR_ACCESSORS( uint32 )
INSTANTIATE_CONVAR_ACCESSORS( int64 )
INSTANTIATE_CONVAR_ACCESSORS( uint64 )
INSTANTIATE_CONVAR_ACCESSORS( float32 )
INSTANTIATE_CONVAR_ACCESSORS( float64 )
INSTANTIATE_CONVAR( CUtlString )
INSTANTIATE_CONVAR_ACCESSORS( Color )
INSTANTIATE_CONVAR_ACCESSORS( Vector2D )
INSTANTIATE_CONVAR_ACCESSORS( Vector )
INSTANTIATE_CONVAR_ACCESSORS( Vector4D )
INSTANTIATE_CONVAR_ACCESSORS( QAngle )
INSTANTIATE_CONVAR_ACCESSORS( VectorWS )

class CConCommandOwner
{
public:
	void Command( const CCommandContext &context, const CCommand &command );
	int Completion( const CCommand &command, CUtlVector<CUtlString> &completions );
};

template class CConCommandMemberAccessor<CConCommandOwner>;
