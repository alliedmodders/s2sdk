#ifndef ISCENESYSTEM_H
#define ISCENESYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "const.h"

struct vis_info_t
{
	uint32 m_uVisBitsBufSize;
	SpawnGroupHandle_t m_SpawnGroupHandle;
	CBitVec<4096> m_VisBits;
};

#endif // ISCENESYSTEM_H
