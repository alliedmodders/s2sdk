#include "tier0/threadtools.h"
#include "tier0/utlstring.h"

template class CInterlockedIntT<int>;
template class CInterlockedIntT<unsigned>;
template class CInterlockedPtr<CUtlString>;
template class CThreadTerminalMutex<CThreadNullMutex>;
template class CAutoLockT<CAtomicMutex>;
template class CAutoLockT<CThreadNullMutex>;
template class CMessageQueue<CUtlString>;

bool g_bMutexCondition;

template class CThreadConditionalMutex<CThreadNullMutex, &g_bMutexCondition>;
