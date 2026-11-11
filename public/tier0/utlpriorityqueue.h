//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef UTLPRIORITYQUEUE_H
#define UTLPRIORITYQUEUE_H
#ifdef _WIN32
#pragma once
#endif

#include "utlleanvector.h"

template < typename T >
class CDefUtlPriorityQueueLessFunc
{
public:
	bool operator()( const T &lhs, const T &rhs ) const { return lhs < rhs; }
};


// T is the type stored in the queue, it must include the priority
// The head of the list contains the element with GREATEST priority
// configure the LessFunc_t to get the desired queue order
template< class T, class LessFunc = CDefUtlPriorityQueueLessFunc< T > >
class CUtlPriorityQueue
{
public:
	// Less func typedef, a functor or a function pointer type
	// Returns true if the first parameter is "less priority" than the second
	// Items that are "less priority" sort toward the tail of the queue
	typedef LessFunc LessFunc_t;

	typedef T ElemType_t;

	CUtlPriorityQueue( int growSize = 0, int initSize = 0, const LessFunc_t &lessfunc = LessFunc_t() );
	CUtlPriorityQueue( T *pMemory, int numElements, const LessFunc_t &lessfunc = LessFunc_t() );

	// gets particular elements
	inline T const&	ElementAtHead() const { return m_heap.Element(0); }

	inline bool IsValidIndex(int index) const { return m_heap.IsValidIndex(index); }

	// O(lgn) to rebalance the heap
	void		RemoveAtHead();
	void		RemoveAt( int index );

	// Restores the heap order after the element's priority was changed, O(lgn)
	void		RevaluateElement( int index );

	// O(lgn) to rebalance heap
	void		Insert( T const &element );
	// Sets the less func
	void		SetLessFunc( const LessFunc_t &lessfunc ) { m_LessFunc = lessfunc; }

	// Returns the count of elements in the queue
	inline int	Count() const { return m_heap.Count(); }
	
	// doesn't deallocate memory
	void		RemoveAll() { m_heap.RemoveAll(); }

	// Memory deallocation
	void		Purge() { m_heap.Purge(); }

	inline const T &	Element( int index ) const { return m_heap.Element(index); }

protected:
	CUtlLeanVector<T, int>	m_heap;

	void		Swap( int index1, int index2 );
	int			PercolateDown( int index );
	int			PercolateUp( int index );

	// Used for sorting.
	LessFunc_t m_LessFunc;
};

template< class T, class LessFunc >
inline CUtlPriorityQueue<T, LessFunc>::CUtlPriorityQueue( int growSize, int initSize, const LessFunc_t &lessfunc ) :
	m_heap(growSize, initSize), m_LessFunc(lessfunc)
{
}

template< class T, class LessFunc >
inline CUtlPriorityQueue<T, LessFunc>::CUtlPriorityQueue( T *pMemory, int numElements, const LessFunc_t &lessfunc )	:
	m_heap(pMemory, numElements), m_LessFunc(lessfunc)
{
}

template< class T, class LessFunc >
void CUtlPriorityQueue<T, LessFunc>::RemoveAtHead()
{
	m_heap.FastRemove( 0 );

	if ( Count() > 0 )
		PercolateDown( 0 );
}


template< class T, class LessFunc >
void CUtlPriorityQueue<T, LessFunc>::RemoveAt( int index )
{
	Assert(m_heap.IsValidIndex(index));
	m_heap.FastRemove( index );

	// The element moved into the hole may belong above or below it
	if ( index < Count() )
		RevaluateElement( index );
}

template< class T, class LessFunc >
void CUtlPriorityQueue<T, LessFunc>::RevaluateElement( int nStartingIndex )
{
	if ( !m_heap.IsValidIndex( nStartingIndex ) )
	{
		Assert( 0 );
		return;
	}

	int index = PercolateDown( nStartingIndex );

	// If it stayed where it was, it was not smaller than its children, but it
	// could be larger than its parent, so treat it like an insertion.
	if ( index == nStartingIndex )
		PercolateUp( index );
}

template< class T, class LessFunc >
int CUtlPriorityQueue<T, LessFunc>::PercolateDown( int index )
{
	Assert( m_heap.IsValidIndex( index ) );

	int count = Count();
	int larger = index;
	for (;;)
	{
		int child = ((index+1) * 2) - 1;	// if we wasted an element, this math would be more compact (1 based array)
		if ( child >= count )
			break;

		if ( m_LessFunc( m_heap[index], m_heap[child] ) )
		{
			// mark the potential swap and check the other child
			larger = child;
		}

		// go to sibling
		child++;
		if ( child < count )
		{
			// If this child is larger, swap it instead
			if ( m_LessFunc( m_heap[larger], m_heap[child] ) )
				larger = child;
		}

		// Item has been filtered down to its proper place, terminate.
		if ( larger == index )
			break;

		// swap with the larger child
		Swap( index, larger );
		index = larger;
	}

	return index;
}

template< class T, class LessFunc >
int CUtlPriorityQueue<T, LessFunc>::PercolateUp( int index )
{
	Assert( m_heap.IsValidIndex( index ) );

	while ( index > 0 )
	{
		int parent = ((index+1) / 2) - 1;

		// Heap condition satisfied?  Then we're done
		if ( m_LessFunc( m_heap[index], m_heap[parent] ) )
			break;

		// swap with parent and repeat
		Swap( parent, index );
		index = parent;
	}

	return index;
}

template< class T, class LessFunc >
void CUtlPriorityQueue<T, LessFunc>::Insert( T const &element )
{
	PercolateUp( m_heap.AddToTail( element ) );
}

template< class T, class LessFunc >
void CUtlPriorityQueue<T, LessFunc>::Swap( int index1, int index2 )
{
	T tmp = m_heap[index1];
	m_heap[index1] = m_heap[index2];
	m_heap[index2] = tmp;
}

#endif // UTLPRIORITYQUEUE_H
