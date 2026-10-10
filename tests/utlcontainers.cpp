#include "tier1/utlmap.h"
#include "tier1/utlrbtree.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utlhashtable.h"
#include "tier1/utldict.h"
#include "tier1/UtlStringMap.h"
#include "tier1/UtlSortVector.h"
#include "tier1/utlqueue.h"
#include "tier1/utlstack.h"
#include "tier1/utlpriorityqueue.h"
#include "tier1/utltshash.h"
#include "tier1/utlhash.h"
#include "tier1/mempool.h"
#include "tier1/memblockallocator.h"
#include "tier1/utldelegate.h"
#include "tier1/utlstring.h"
#include "tier1/utlsymbollarge.h"
#include "tier1/utlmultilist.h"
#include "tier1/utlhandletable.h"
#include "tier1/utlhashdict.h"
#include "tier1/utlfixedmemory.h"
#include "tier1/utlcommon.h"
#include "tier1/utlscratchmemory.h"
#include "tier1/circularbuffer.h"
#include "tier1/utlntree.h"
#include "tier1/utlintrusivelist.h"
#include "tier1/memstack.h"
#include "tier0/utlblockvector.h"

#ifdef _MSC_VER
// Copy constructors are declared without a definition to make containers non-copyable
#pragma warning( disable : 4661 )
#endif

template class CUtlOrderedMap<int, CUtlString>;
template class CUtlOrderedMap<CUtlString, CUtlString>;
template class CUtlOrderedMap<CUtlString, int>;
template class CUtlOrderedMap<const char *, CUtlString, CDefFastCaselessStringLess, uint16>;
template class CUtlOrderedMap<CUtlSymbolLarge, CUtlString, CDefLess<CUtlSymbolLarge>, uint16>;
template class CUtlRBTree<CUtlString>;
template class CUtlDict<CUtlString>;
template class CUtlStringMap<CUtlString>;
template class CUtlOrderedMapBase<int, CUtlString, CDefLess<int>>;
template class CUtlOrderedMapBase<CUtlString, CUtlString, CDefLess<CUtlString>>;
template class CUtlOrderedMapBase<CUtlString, int, CDefLess<CUtlString>>;
template class CUtlOrderedMapBase<const char *, CUtlString, CDefFastCaselessStringLess, uint16>;
template class CUtlOrderedMapBase<CUtlSymbolLarge, CUtlString, CDefLess<CUtlSymbolLarge>, uint16>;
template class CUtlHashDict<CUtlString>;

// Members taking the key's alternate type need a key that has one
template class CUtlHashtable<CUtlString, CUtlString>;
template class CUtlHashtable<CUtlString, empty_t>;
template class CUtlStableHashtable<CUtlString, CUtlString>;

template class CUtlLinkedList<CUtlString>;
template class CUtlLinkedList<CUtlString, unsigned short, true>;
template class CUtlFixedLinkedList<CUtlString>;
template class CUtlPtrLinkedList<CUtlString>;
template class CUtlNTree<CUtlString>;
template class CUtlMemoryStack<CUtlString, int, 1024>;
template class CUtlQueue<CUtlString>;
template class CUtlQueueFixed<CUtlString, 4>;
template class CUtlStack<CUtlString>;
template class CUtlPriorityQueue<CUtlString>;
template class CUtlTSHash<CUtlString, 16, uint>;
template class CUtlHash<CUtlString>;
template class CUtlMemoryPool<CUtlString>;
template class CUtlMemoryBlockAllocator<byte>;
template class CUtlMultiList<CUtlString, int>;
template class CUtlHashFixed<CUtlString, 16>;
template class CUtlScalarHash<int>;
template class CUtlFixedMemory<CUtlString>;
template class CUtlKeyValuePair<CUtlString, CUtlString>;
template class CUtlKeyValuePair<CUtlString, empty_t>;
template class CUtlScratchMemoryPoolFixedGrowable<1024>;
template class CObjectPool<CUtlString>;
template class CFixedBudgetMemoryPool<16, 16>;
template class CFixedSizeCircularBuffer<CUtlString, 16>;
template class CUtlDelegate<bool ( int, const CUtlString & )>;
template class FastDelegate0<>;
template class FastDelegate1<int>;
template class FastDelegate2<int, const CUtlString &, bool>;
template class FastDelegate3<int, int, int>;
template class FastDelegate4<int, int, int, int>;
template class FastDelegate5<int, int, int, int, int>;
template class FastDelegate6<int, int, int, int, int, int>;
template class FastDelegate7<int, int, int, int, int, int, int>;
template class FastDelegate8<int, int, int, int, int, int, int, int>;

// CUtlHandle converts between objects and handles through their own members
class CHandleObject
{
public:
	UtlHandle_t GetHandle() const { return UTLHANDLE_INVALID; }
	static CHandleObject *GetPtrFromHandle( UtlHandle_t h ) { return nullptr; }
	static bool IsHandleValid( UtlHandle_t h ) { return false; }
};

template class CUtlHandleTable<CHandleObject, 16>;
template class CUtlHandle<CHandleObject>;

// The SDK has no allocator with the interface CAlignedMemPool takes
class CPoolAllocator
{
public:
	void *Alloc( size_t nSize ) { return malloc( nSize ); }
	void Free( void *p ) { free( p ); }
};

template class CAlignedMemPool<16, 16, 1024, CPoolAllocator>;

// A tree with other memory than the default
class CRBTreeAllocator : public CMemAllocAllocator
{
};

template class CUtlRBTree<CUtlString, int, bool ( * )( const CUtlString &, const CUtlString & ), CUtlLeanVector<UtlRBTreeNode_t<CUtlString, int>, int, CRBTreeAllocator>>;
template class CUtlRBTree<CUtlString, int, bool ( * )( const CUtlString &, const CUtlString & ), CUtlBlockVector<UtlRBTreeNode_t<CUtlString, int>, int>>;

// Intrusive lists link the nodes through their own members
struct IntrusiveNode_t
{
	IntrusiveNode_t *m_pNext;
	IntrusiveNode_t *m_pPrev;
	const char *m_Name;
};

template class CUtlIntrusiveList<IntrusiveNode_t>;
template class CUtlIntrusiveDList<IntrusiveNode_t>;
template class CUtlIntrusiveDListWithTailPtr<IntrusiveNode_t>;
template class CUtlIntrusiveListWithTailPtr<IntrusiveNode_t>;

struct CUtlStringLess
{
	bool Less( const CUtlString &a, const CUtlString &b, void *pCtx ) { return a < b; }
};

template class CUtlSortVector<CUtlString, CUtlStringLess>;

template class CUtlSymbolTableLargeBase<false, 2048, CThreadNullMutex>;
template class CUtlSymbolTableLargeBase<true, 2048, CThreadNullMutex>;
template class CUtlSymbolTableLargeBase<false, 2048, CAtomicMutex>;
template class CUtlSymbolTableLargeBase<true, 2048, CAtomicMutex>;
