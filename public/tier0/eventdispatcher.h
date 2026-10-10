#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#ifdef _WIN32
#pragma once
#endif

#include <tier0/utldelegate.h>
#include <tier0/utlmap.h>

struct SchemaClassInfoData_t;

enum EventMapRegistrationType_t : int32
{
	EVENT_MAP_REGISTER = 0,
	EVENT_MAP_UNREGISTER,
};

struct CEventDispatcher_Base
{
	struct EventListenerInfo_t
	{
		CUtlAbstractDelegate m_Delegate;
		const char *m_pszName;
		int32 m_nPriority;
		uint8 m_nDelegateParamCount;
		bool m_bDelegateReturnsVoid;
	};

	struct DelegateIterator_Base_t
	{
		const CUtlVector< EventListenerInfo_t > *pListeners;
		CUtlVectorFixedGrowable< int, 4 > skipListeners;
		int nCurrent;
		DelegateIterator_Base_t *pNext;
		bool bIteratingForward;
		bool bIsInListenerTelemetryScope;
	};

	CThreadFastMutex m_Lock;
	DelegateIterator_Base_t *m_pActiveIterators;
};

struct CEventID_SchemaBinding
{
	int8 unused;
};

struct CEventIDManager_SchemaBinding : CEventID_SchemaBinding
{
};

struct CEventIDManager_Default : CEventIDManager_SchemaBinding
{
};

template <typename T>
struct CEventDispatcher_Identified : CEventDispatcher_Base
{
	CUtlOrderedMap< const SchemaClassInfoData_t*, CCopyableUtlVector<CEventDispatcher_Base::EventListenerInfo_t>, CDefLess<const SchemaClassInfoData_t*>, unsigned int> m_EventListenerMap;
};

template <typename T>
struct CEventDispatcher : CEventDispatcher_Identified<T>
{
};

#endif // EVENTDISPATCHER_H