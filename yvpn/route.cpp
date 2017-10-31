#include "stdafx.h"
#include "route.h"

char szOldRoute[20] = { 0 };
MIB_IPFORWARDROW oldRoute = { 0 };

PMIB_IPFORWARDTABLE MyGetIpForwardTable(BOOL bOrder)
{
	PMIB_IPFORWARDTABLE pIpRouteTab = NULL;
	DWORD dwActualSize = 0;

	// ��ѯ���軺�����Ĵ�С  
	if (::GetIpForwardTable(pIpRouteTab, &dwActualSize, bOrder) == ERROR_INSUFFICIENT_BUFFER)
	{
		// ΪMIB_IPFORWARDTABLE�ṹ�����ڴ�  
		pIpRouteTab = (PMIB_IPFORWARDTABLE)::GlobalAlloc(GPTR, dwActualSize);
		// ��ȡ·�ɱ�  
		if (::GetIpForwardTable(pIpRouteTab, &dwActualSize, bOrder) == NO_ERROR)
			return pIpRouteTab;
		::GlobalFree(pIpRouteTab);
	}
	return NULL;
}

void MyFreeIpForwardTable(PMIB_IPFORWARDTABLE pIpRouteTab)
{
	if (pIpRouteTab != NULL)
		::GlobalFree(pIpRouteTab);
}

int AddDefaultRoute()
{
	DWORD dwRet = CreateIpForwardEntry(&oldRoute);
	return dwRet;
}

int RemoveDefaultRoute(char *lpszCurrentVpnAddress)
{
	PMIB_IPFORWARDTABLE pIpRouteTable = MyGetIpForwardTable(TRUE);
	if (pIpRouteTable == NULL)
	{
		return -1;
	}
	DWORD i, dwCurrIndex;
	struct in_addr inadDest;
	struct in_addr inadMask;
	struct in_addr inadGateway;
	PMIB_IPADDRTABLE pIpAddrTable = NULL;

	char szDestIp[128];
	char szMaskIp[128];
	char szGatewayIp[128];
	for (i = 0; i < pIpRouteTable->dwNumEntries; i++)
	{
		dwCurrIndex = pIpRouteTable->table[i].dwForwardIfIndex;

		// Ŀ�ĵ�ַ  
		inadDest.s_addr = pIpRouteTable->table[i].dwForwardDest;
		// ��������  
		inadMask.s_addr = pIpRouteTable->table[i].dwForwardMask;
		// ���ص�ַ  
		inadGateway.s_addr = pIpRouteTable->table[i].dwForwardNextHop;

		strcpy(szDestIp, inet_ntoa(inadDest));
		strcpy(szMaskIp, inet_ntoa(inadMask));
		strcpy(szGatewayIp, inet_ntoa(inadGateway));
		if ((szGatewayIp,lpszCurrentVpnAddress) == 0)
		{
			continue;
		}
		if (strcmp(szDestIp, "0.0.0.0") == 0 && strcmp(szMaskIp, "0.0.0.0") == 0)
		{
			DWORD dwRet = DeleteIpForwardEntry(&pIpRouteTable->table[i]);
			if (dwRet != NO_ERROR)
			{
				MyFreeIpForwardTable(pIpRouteTable);
				return dwRet;
			}
			strcpy(szOldRoute, szGatewayIp);
			memcpy(&oldRoute, &pIpRouteTable->table[i], sizeof(oldRoute));
			break;
		}
	}
	MyFreeIpForwardTable(pIpRouteTable);
	return 1;
}