//========== Copyright © 2006, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

#ifndef CALLQUEUE_H
#define CALLQUEUE_H

#include <functional>
#include "tier0/tslist.h"
#include "jobthread.h"

#if defined( _WIN32 )
#pragma once
#endif

//-----------------------------------------------------

class CCallQueue
{
public:
	CCallQueue()
		: m_nRefCount( 0 ),
		m_bNoQueue( false ),
		m_bAbortProcessing( false ),
		m_bProcessing( false )
	{
	}

	~CCallQueue()
	{
		Flush();
	}

	int Count()
	{
		return m_queue.Count();
	}

	void CallQueued()
	{
		m_bProcessing = true;

		if ( m_queue.Count() )
		{
			m_queue.PushItem( NULL );

			std::function<void()> *pFunctor;

			while ( m_queue.PopItem( &pFunctor ) && pFunctor != NULL )
			{
				if ( !m_bAbortProcessing )
				{
					(*pFunctor)();
				}
				delete pFunctor;
			}
		}

		m_bAbortProcessing = false;
		m_bProcessing = false;
	}

	DLL_CLASS_IMPORT void ParallelCallQueued( const char *pszName, IThreadPool *pPool = NULL );

	template <typename FUNCTOR>
	void QueueCall( FUNCTOR &&functor )
	{
		if ( !m_bNoQueue )
		{
			m_queue.PushItem( new std::function<void()>( std::forward<FUNCTOR>( functor ) ) );
		}
		else
		{
			m_bProcessing = true;
			if ( !m_bAbortProcessing )
			{
				functor();
			}
			m_bAbortProcessing = false;
			m_bProcessing = false;
		}
	}

	void Flush()
	{
		m_queue.PushItem( NULL );

		std::function<void()> *pFunctor;

		while ( m_queue.PopItem( &pFunctor ) && pFunctor != NULL )
		{
			delete pFunctor;
		}

		m_bAbortProcessing = false;
	}

private:
	CTSQueue<std::function<void()> *> m_queue;
	int16 m_nRefCount;
	bool m_bNoQueue;
	bool m_bAbortProcessing;
	bool m_bProcessing;
};

#endif // CALLQUEUE_H
