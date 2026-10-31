#ifndef UTLVECTORULTRACONSERVATIVE_H
#define UTLVECTORULTRACONSERVATIVE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utlvector.h"

//-----------------------------------------------------------------------------
// The CUtlVectorUltra Conservative class:
// A array class with a very conservative allocation scheme, with customizable allocator
// Especialy useful if you have a lot of vectors that are sparse, or if you're
// carefully packing holders of vectors
// An empty vector holds a null m_pData, as the engine's do; the first element allocates it
//-----------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200) // warning C4200: nonstandard extension used : zero-sized array in struct/union
#endif

class CUtlVectorUltraConservativeAllocator
{
public:
	static void *Alloc( size_t nSize )
	{
		return malloc( nSize );
	}

	static void *Realloc( void *pMem, size_t nSize )
	{
		return realloc( pMem, nSize );
	}

	static void Free( void *pMem )
	{
		free( pMem );
	}

	static size_t GetSize( void *pMem )
	{
		return mallocsize( pMem );
	}

};

template <typename T, typename A = CUtlVectorUltraConservativeAllocator >
class CUtlVectorUltraConservative : private A
{
public:
	CUtlVectorUltraConservative()
	{
		m_pData = nullptr;
	}

	~CUtlVectorUltraConservative()
	{
		RemoveAll();
	}

	int Count() const
	{
		return m_pData ? m_pData->m_Size : 0;
	}

	static int InvalidIndex()
	{
		return -1;
	}

	inline bool IsValidIndex( int i ) const
	{
		return (i >= 0) && (i < Count());
	}

	T& operator[]( int i )
	{
		Assert( IsValidIndex( i ) );
		return Base()[i];
	}

	const T& operator[]( int i ) const
	{
		Assert( IsValidIndex( i ) );
		return Base()[i];
	}

	T& Element( int i )
	{
		Assert( IsValidIndex( i ) );
		return Base()[i];
	}

	const T& Element( int i ) const
	{
		Assert( IsValidIndex( i ) );
		return Base()[i];
	}

	T* Base() { return m_pData ? reinterpret_cast<T *>( m_pData->m_Elements ) : nullptr; }
	const T* Base() const { return m_pData ? reinterpret_cast<const T *>( m_pData->m_Elements ) : nullptr; }

	void EnsureCapacity( int num )
	{
		int nCurCount = Count();
		if ( num <= nCurCount )
		{
			return;
		}
		if ( !m_pData )
		{
			m_pData = (Data_t *)A::Alloc( sizeof(Data_t) + ( num * sizeof(T) ) );
			m_pData->m_Size = 0;
		}
		else
		{
			size_t nNeeded = sizeof(Data_t) + ( num * sizeof(T) );
			size_t nHave = A::GetSize( m_pData );
			if ( nNeeded > nHave )
			{
				m_pData = (Data_t *)A::Realloc( m_pData, nNeeded );
			}
		}
	}

	int AddToTail( const T& src )
	{
		int iNew = Count();
		EnsureCapacity( Count() + 1 );
		CopyConstruct( &Base()[iNew], src );
		m_pData->m_Size++;
		return iNew;
	}

	void RemoveAll()
	{
		if ( !m_pData )
			return;

		for (int i = m_pData->m_Size; --i >= 0; )
		{
			Destruct( &Base()[i] );
		}

		A::Free( m_pData );
		m_pData = nullptr;
	}

	void PurgeAndDeleteElements()
	{
		if ( m_pData )
		{
			if constexpr ( std::is_pointer_v<T> )
			{
				for( int i=0; i < m_pData->m_Size; i++ )
				{
					delete Element(i);
				}
			}
			RemoveAll();
		}
	}

	void FastRemove( int elem )
	{
		Assert( IsValidIndex(elem) );

		Destruct( &Element(elem) );
		if (Count() > 0)
		{
			if ( elem != m_pData->m_Size -1 )
				memcpy( (void *)&Element(elem), (void *)&Element(m_pData->m_Size-1), sizeof(T) );
			--m_pData->m_Size;
		}
		if ( !m_pData->m_Size )
		{
			A::Free( m_pData );
			m_pData = nullptr;
		}
	}

	void Remove( int elem )
	{
		Destruct( &Element(elem) );
		ShiftElementsLeft(elem);
		--m_pData->m_Size;
		if ( !m_pData->m_Size )
		{
			A::Free( m_pData );
			m_pData = nullptr;
		}
	}

	int Find( const T& src ) const
	{
		int nCount = Count();
		for ( int i = 0; i < nCount; ++i )
		{
			if (Element(i) == src)
				return i;
		}
		return -1;
	}

	bool FindAndRemove( const T& src )
	{
		int elem = Find( src );
		if ( elem != -1 )
		{
			Remove( elem );
			return true;
		}
		return false;
	}


	bool FindAndFastRemove( const T& src )
	{
		int elem = Find( src );
		if ( elem != -1 )
		{
			FastRemove( elem );
			return true;
		}
		return false;
	}

	// The elements follow the count at T's alignment; they're raw storage, constructed in place, so T can have a
	// destructor
	struct Data_t
	{
		int m_Size;
		alignas( T ) uint8 m_Elements[];
	};

	Data_t *m_pData;
private:
	void ShiftElementsLeft( int elem, int num = 1 )
	{
		int Size = Count();
		Assert( IsValidIndex(elem) || ( Size == 0 ) || ( num == 0 ));
		int numToMove = Size - elem - num;
		if ((numToMove > 0) && (num > 0))
		{
			Q_memmove( &Element(elem), &Element(elem+num), numToMove * sizeof(T) );

#ifdef _DEBUG
			Q_memset( &Element(Size-num), 0xDD, num * sizeof(T) );
#endif
		}
	}
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // UTLVECTORULTRACONSERVATIVE_H
