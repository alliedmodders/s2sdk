//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef RECIPIENTFILTER_H
#define RECIPIENTFILTER_H
#ifdef _WIN32
#pragma once
#endif

#include "irecipientfilter.h"
#include "const.h"
#include "bitvec.h"
#include "playerslot.h"

//-----------------------------------------------------------------------------
// Purpose: A generic filter for determining whom to send message/sounds etc. to and
//  providing a bit of additional state information
//-----------------------------------------------------------------------------
class CRecipientFilter : public IRecipientFilter
{
public:
	CRecipientFilter() :
		m_nPredictedPlayerSlot( -1 ),
		m_nBufType( BUF_DEFAULT ),
		m_bInitMessage( false ),
		m_bUsingPredictionRules( false ),
		m_bIgnorePredictionCull( false )
	{
	}

	virtual			~CRecipientFilter() {}

	virtual NetChannelBufType_t	GetNetworkBufType( void ) const { return m_nBufType; }
	virtual bool	IsInitMessage( void ) const { return m_bInitMessage; }

	virtual const CPlayerBitVec &GetRecipients( void ) const { return m_Recipients; }
	virtual CPlayerSlot GetPredictedPlayerSlot( void ) const { return m_nPredictedPlayerSlot; }

public:

	void			CopyFrom( const CRecipientFilter &src )
	{
		m_Recipients = src.m_Recipients;
		m_nBufType = src.GetNetworkBufType();
		m_bInitMessage = src.IsInitMessage();
		m_bUsingPredictionRules = src.m_bUsingPredictionRules;
		m_bIgnorePredictionCull = src.m_bIgnorePredictionCull;
	}

	void			Reset( void )
	{
		m_Recipients.ClearAll();
		m_nBufType = BUF_DEFAULT;
		m_bInitMessage = false;
		m_bUsingPredictionRules = false;
		m_bIgnorePredictionCull = false;
	}

	void			MakeInitMessage( void ) { m_bInitMessage = true; }

	void			MakeReliable( void ) { m_nBufType = BUF_RELIABLE; }

	// AMNOTE: The game adds only connected players, the engine skips slots without a connected client
	void			AddAllPlayers( void )
	{
		m_Recipients.SetAll();
		m_nPredictedPlayerSlot.Invalidate();
	}

	// AMNOTE: The game adds only a slot with a connected player, and with prediction rules it sets the predicted player slot instead for the suppressed host
	void			AddRecipient( CPlayerSlot slot )
	{
		if ( slot.IsValid() )
			m_Recipients.Set( slot.Get() );
	}

	void			RemoveAllRecipients( void )
	{
		m_Recipients.ClearAll();
	}

	void			RemoveRecipient( CPlayerSlot slot )
	{
		if ( slot.IsValid() )
			m_Recipients.Clear( slot.Get() );
	}

	// AMNOTE: The game sets and clears only slots with a connected player
	void			AddPlayersFromBitMask( const CPlayerBitVec &playerbits )
	{
		for ( int i = 0; i < m_Recipients.GetNumDWords(); i++ )
			m_Recipients.SetDWord( i, m_Recipients.GetDWord( i ) | playerbits.GetDWord( i ) );
	}

	void			RemovePlayersFromBitMask( const CPlayerBitVec &playerbits )
	{
		for ( int i = 0; i < m_Recipients.GetNumDWords(); i++ )
			m_Recipients.SetDWord( i, m_Recipients.GetDWord( i ) & ~playerbits.GetDWord( i ) );
	}

	bool			IsUsingPredictionRules( void ) const { return m_bUsingPredictionRules; }

	bool			IgnorePredictionCull( void ) const { return m_bIgnorePredictionCull; }
	void			SetIgnorePredictionCull( bool ignore ) { m_bIgnorePredictionCull = ignore; }

private:

	CPlayerBitVec		m_Recipients;
	CPlayerSlot			m_nPredictedPlayerSlot;
	NetChannelBufType_t	m_nBufType;
	bool				m_bInitMessage;
	// If using prediction rules, the filter itself suppresses local player
	bool				m_bUsingPredictionRules;
	// If ignoring prediction cull, then external systems can determine
	//  whether this is a special case where culling should not occur
	bool				m_bIgnorePredictionCull;
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player
//-----------------------------------------------------------------------------
class CSingleUserRecipientFilter : public CRecipientFilter
{
public:
	CSingleUserRecipientFilter( CPlayerSlot slot )
	{
		AddRecipient( slot );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player ( reliable )
//-----------------------------------------------------------------------------
class CReliableSingleUserRecipientFilter : public CSingleUserRecipientFilter
{
public:
	CReliableSingleUserRecipientFilter( CPlayerSlot slot ) :
		CSingleUserRecipientFilter( slot )
	{
		MakeReliable();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players
//-----------------------------------------------------------------------------
class CBroadcastRecipientFilter : public CRecipientFilter
{
public:
	CBroadcastRecipientFilter( void )
	{
		AddAllPlayers();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players ( reliable )
//-----------------------------------------------------------------------------
class CReliableBroadcastRecipientFilter : public CBroadcastRecipientFilter
{
public:
	CReliableBroadcastRecipientFilter( void )
	{
		MakeReliable();
	}
};

#endif // RECIPIENTFILTER_H
