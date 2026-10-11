#ifndef NS_ADDRESS_H
#define NS_ADDRESS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/netadr.h"
#include "steam/steamclientpublic.h"

enum ENSAddressType
{
	kAddressDirect,
	kAddressP2P,
	kAddressProxiedGameServer,
	kAddressProxiedClient,
	kAddressLoopback,

	kAddressMax
};

class ns_address
{
public:
	const netadr_t &GetAddress() const { return m_Address; }
	const CSteamID& GetSteamID() const { return m_ID; }
	const uint16 GetRemotePort() const { return m_nRemotePort; }
	ENSAddressType GetAddressType() const { return m_AddressType; }
private:
	netadr_t m_Address;
	CSteamID m_ID;
	uint16 m_nRemotePort;
	int m_Unknown;
	ENSAddressType m_AddressType;
};

#endif // NS_ADDRESS_H
