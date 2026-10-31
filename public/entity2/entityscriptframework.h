#ifndef ENTITYSCRIPTFRAMEWORK_H
#define ENTITYSCRIPTFRAMEWORK_H

#ifdef _WIN32
#pragma once
#endif

#include "entity2/entitycomponent.h"
#include "entity2/ientity2_entityio.h"
#include "tier0/utlvectorultraconservative.h"

class CScriptComponent : public CEntityComponent
{
public:
	virtual const char* GetComponentName() = 0;

public:
	HSCRIPT m_hScope;
	CUtlVectorUltraConservative<HSCRIPT *> m_FuncHandles;

	// CScriptScope::Flags_t: m_hScope, m_FuncHandles and m_flags are the component's CScriptScopeT members
	int m_flags;
	CEntityOwnerPtr m_pOwner;

	CUtlVectorUltraConservative<CEntityIOOutput> m_Outputs;

	CUtlSymbolLarge m_scriptClassName;
};

#endif // ENTITYSCRIPTFRAMEWORK_H
