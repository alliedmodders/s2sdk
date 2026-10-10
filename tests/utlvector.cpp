#include "tier0/utlvector.h"
#include "tier0/utlleanvector.h"
#include "tier0/utlstring.h"
#include "tier0/utlblockvector.h"

template class CUtlVectorBase<CUtlString>;
template class CUtlVectorMemory_Growable<CUtlString>;
template class CCopyableUtlVector<CUtlString>;
template class CUtlLeanVectorImpl<CUtlLeanVectorBase<CUtlString, int, CMemAllocAllocator>, CUtlString, int>;
template class CUtlLeanVectorBase<CUtlString, int, CMemAllocAllocator>;
template class CUtlLeanVectorImpl<CUtlLeanVectorFixedGrowableBase<CUtlString, 4, int, CMemAllocAllocator>, CUtlString, int>;
template class CUtlLeanVectorFixedGrowableBase<CUtlString, 4, int, CMemAllocAllocator>;
template class CUtlLeanVectorImpl<CUtlLeanVectorBase<CUtlString, unsigned short, CMemAllocAllocator>, CUtlString, unsigned short>;
template class CUtlLeanVectorBase<CUtlString, unsigned short, CMemAllocAllocator>;
template class CUtlLeanVectorImpl<CUtlLeanVectorBase<CUtlString, unsigned int, CMemAllocAllocator>, CUtlString, unsigned int>;
template class CUtlLeanVectorBase<CUtlString, unsigned int, CMemAllocAllocator>;
template class CUtlLeanVectorImpl<CUtlLeanVectorFixedGrowableBase<CUtlString, 4, unsigned int, CMemAllocAllocator>, CUtlString, unsigned int>;
template class CUtlLeanVectorFixedGrowableBase<CUtlString, 4, unsigned int, CMemAllocAllocator>;
template class CUtlVectorMemory_Aligned<CUtlString, 16>;
template class CUtlVectorBase<CUtlString, int, CUtlVectorMemory_Aligned<CUtlString, 16>>;
template class CUtlVectorAutoPurge<CUtlString *>;

// Clang rejects its flexible array member for types with a destructor
template class CUtlVectorUltraConservative<CUtlString *>;

// CUtlVectorBase can only be instantiated whole with the allocators above, the others lack members it forwards to like SetGrowSize and Detach
template class CUtlVectorMemory_Fixed<CUtlString, 4>;
template class CUtlVectorMemory_FixedGrowable<CUtlString, 4>;
template class CUtlVectorMemory_Conservative<CUtlString>;
template class CUtlVectorMemory_RawAllocator<CUtlString, CMemAllocAllocator>;
template class CUtlVectorFixed<CUtlString, 4>;
template class CUtlVectorFixedGrowable<CUtlString, 4>;
template class CUtlVectorConservative<CUtlString>;
template class CUtlVectorRawAllocator<CUtlString>;
template class CUtlBlockVector<CUtlString>;
template class CUtlBlockVector<CUtlString, unsigned short>;
