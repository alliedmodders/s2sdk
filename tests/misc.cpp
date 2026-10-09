#include "tier0/tslist.h"
#include "tier0/basetypes.h"
#include "tier0/platform.h"
#include "tier0/vprof.h"
#include "tier1/bufferstring.h"
#include "tier1/fmtstr.h"
#include "tier1/smartptr.h"
#include "tier1/refcount.h"
#include "tier1/functors.h"
#include "tier1/utlstring.h"
#include "tier1/rangecheckedvar.h"
#include "tier1/utlsoacontainer.h"
#include "tier1/keyvalues3.h"
#include "mathlib/vec3d.h"
#include "ehandle.h"
#include "variant.h"
#include "appframework/IAppSystem.h"
#include "tier1/tier1.h"
#include "entity2/entityinstance.h"
#include "entity2/entitykeyvalues.h"
#include "engine/eventdispatcher.h"
#include "schemasystem/schematypes.h"
#include "networksystem/netmessage.h"
#include "iloopmode.h"
#include "source2_steam_stats.pb.h"

template class CBufferStringN<MAX_PATH>;
template class CFmtStrN<256>;
template class CUtlConstStringBase<char>;
template class CHandle<CEntityInstance>;
template class CSmartPtr<CRefCounted<>>;
template class CTSQueue<CUtlString>;
template class CTSPool<CUtlString>;
template class CTSList<CUtlString>;
template class CTSListWithFreeList<CUtlString>;

struct TSSimpleListNode_t : TSLNodeBase_t
{
	int m_nValue;
};

template class CTSSimpleList<TSSimpleListNode_t>;

class CRefCountedBase1 {};
class CRefCountedBase2 {};
class CRefCountedBase3 {};
class CRefCountedBase4 {};
class CRefCountedBase5 {};

template class CRefCountServiceBase<CRefMT>;
template class CRefCountServiceBase<CRefST>;
template class CRefCounted<>;
template class CRefCounted<CRefCountServiceST>;
template class CRefCounted1<IRefCounted>;
template class CRefCounted2<CRefCountedBase1, CRefCountedBase2>;
template class CRefCounted3<CRefCountedBase1, CRefCountedBase2, CRefCountedBase3>;
template class CRefCounted4<CRefCountedBase1, CRefCountedBase2, CRefCountedBase3, CRefCountedBase4>;
template class CRefCounted5<CRefCountedBase1, CRefCountedBase2, CRefCountedBase3, CRefCountedBase4, CRefCountedBase5>;
template class CBaseAutoPtr<IRefCounted>;
template class CRefPtr<IRefCounted>;
template class CAutoRef<>;
template class CPlainAutoPtr<CUtlString>;
template class CArrayAutoPtr<CUtlString>;
template class CLateBoundPtr<CUtlString>;

struct IntHandleType_t;

template class CBaseIntHandle<uint16>;
template class CBaseIntHandle<uint32>;
template class CBaseIntHandle<uint64>;
template class CIntHandle16<IntHandleType_t>;
template class CIntHandle32<IntHandleType_t>;
template class CIntHandle64<IntHandleType_t>;
template class CRangeCheckedVar<int, 0, 10, 5>;
template class CStridedPtr<float>;
template class CStridedConstPtr<float>;
template class Vec3D<float>;
template class Vec3D<int>;
template class CVariantBase<>;
template class VProfScopeHelper<0, false>;
template class CKeyValues3ClusterImpl<KV3_CLUSTER_MAX_ELEMENTS, KeyValues3>;
template class CKeyValues3ClusterImpl<KV3_TABLE_INIT_SIZE, CKeyValues3Table>;
template class CKeyValues3ClusterImpl<KV3_ARRAY_INIT_SIZE, CKeyValues3Array>;
template class CEntityKeyComplex<CUtlString>;
template struct CEventDispatcher_Identified<CEventIDManager_Default>;
template struct CEventDispatcher<CEventIDManager_Default>;
template class CSchemaPtrMap<int, CUtlString *>;
template class CNetMessagePB<0, CMsgSource2SystemSpecs, SG_GENERIC, BUF_RELIABLE, false>;
template class CNetMessagePBView<CMsgSource2SystemSpecs>;
template struct CBaseCmdKeyValues<CMsgSource2SystemSpecs>;
template class CBaseAppSystem<IAppSystem>;
template class CTier0AppSystem<IAppSystem>;
template class CTier1AppSystem<IAppSystem>;

#ifdef PLATFORM_WINDOWS
template class CDynamicFunction<void ( * )()>;
#endif
