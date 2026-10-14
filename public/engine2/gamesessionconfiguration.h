#ifndef GAMESESSIONCONFIGURATION_H
#define GAMESESSIONCONFIGURATION_H

#ifdef _WIN32
#pragma once
#endif

#include "netmessages.h"
#include "networkbasetypes.pb.h"

class GameSessionConfiguration_t : public CBaseCmdKeyValues<CSVCMsg_GameSessionConfiguration>
{
};

#endif // GAMESESSIONCONFIGURATION_H
