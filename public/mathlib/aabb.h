#ifndef AABB_H
#define AABB_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "mathlib/vector4d.h"
#include "mathlib/mathlib.h"
#include "datamap.h"

//--------------------------------------------------------------------------------------
// AABB
//--------------------------------------------------------------------------------------
struct AABB_t
{
	DECLARE_BYTESWAP_DATADESC();

	Vector				m_vMinBounds;
	Vector				m_vMaxBounds;

	Vector GetCenter() const { return ( m_vMaxBounds + m_vMinBounds ) / 2.0f; }

	float GetMinDistToPoint( const Vector &vPoint ) const
	{
		return CalcDistanceToAABB( m_vMinBounds, m_vMaxBounds, vPoint );
	}

	void CreatePlanesFrom( Vector4D *pPlanes ) const
	{
		// X
		pPlanes[0] = Vector4D( 1, 0, 0, -m_vMaxBounds.x  );
		pPlanes[1] = Vector4D( -1, 0, 0, m_vMinBounds.x );

		// Y
		pPlanes[2] = Vector4D( 0, 1, 0, -m_vMaxBounds.y );
		pPlanes[3] = Vector4D( 0, -1, 0, m_vMinBounds.y );

		// Z
		pPlanes[4] = Vector4D( 0, 0, 1, -m_vMaxBounds.z );
		pPlanes[5] = Vector4D( 0, 0, -1, m_vMinBounds.z );
	}

	// set the aabb to be invalid (max < min )
	void MakeInvalid( void )
	{
		m_vMinBounds.Init( FLT_MAX, FLT_MAX, FLT_MAX );
		m_vMaxBounds.Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	}

};

#endif // AABB_H
