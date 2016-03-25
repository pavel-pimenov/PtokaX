/*
 * PtokaX - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2002-2005  Ptaczek, Ptaczek at PtokaX dot org
 * Copyright (C) 2004-2015  Petr Kozelka, PPK at PtokaX dot org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
#include "stdinc.h"
//---------------------------------------------------------------------------
#include "UdpDebug.h"
//---------------------------------------------------------------------------
#include "LanguageManager.h"
#include "ServerManager.h"
#include "SettingManager.h"
#include "User.h"
#include "utility.h"
//---------------------------------------------------------------------------
#ifdef _WIN32
#pragma hdrstop
#else
#include <string>
#include <syslog.h>
#endif
//---------------------------------------------------------------------------
clsUdpDebug * clsUdpDebug::mPtr = NULL;
//---------------------------------------------------------------------------

clsUdpDebug::UdpDbgItem::UdpDbgItem() : pPrev(NULL), pNext(NULL), sNick(NULL),
#ifdef _WIN32
	s(INVALID_SOCKET),
#else
	s(-1),
#endif
	sas_len(0), ui32Hash(0), bIsScript(false), bAllData(true)
{
	memset(&sas_to, 0, sizeof(sockaddr_storage));
}
//---------------------------------------------------------------------------

clsUdpDebug::UdpDbgItem::~UdpDbgItem()
{
	free(sNick);
	
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
#endif
}
//---------------------------------------------------------------------------

clsUdpDebug::clsUdpDebug() : sDebugBuffer(NULL), sDebugHead(NULL), pDbgItemList(NULL)
{
	// ...
}
//---------------------------------------------------------------------------

clsUdpDebug::~clsUdpDebug()
{
	free(sDebugBuffer);
	
	UdpDbgItem * cur = NULL,
	             * next = pDbgItemList;
	             
	while (next != NULL)
	{
		cur = next;
		next = cur->pNext;
		
		delete cur;
	}
}
//---------------------------------------------------------------------------

void clsUdpDebug::Broadcast(const char * msg, const size_t szMsgLen) const
{
#ifndef _WIN32
	{
		std::string l_str(msg, szMsgLen);
		//printf("%s\r", l_str.c_str());
		syslog(LOG_NOTICE, "%s", l_str.c_str());
	}
#endif
	
	if (pDbgItemList == NULL)
	{
		return;
	}
	
	((uint16_t *)sDebugBuffer)[1] = (uint16_t)szMsgLen;
	memcpy(sDebugHead, msg, szMsgLen);
	size_t szLen = (sDebugHead - sDebugBuffer) + szMsgLen;
	
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL && pNext->bAllData == true)
	{
		pCur = pNext;
		pNext = pCur->pNext;
#ifdef _WIN32
		sendto(pCur->s, sDebugBuffer, (int)szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#else
		sendto(pCur->s, sDebugBuffer, szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#endif
		clsServerManager::ui64BytesSent += szLen;
	}
}
//---------------------------------------------------------------------------

void clsUdpDebug::BroadcastFormat(const char * sFormatMsg, ...) const
{

#ifndef _WIN32
	{
		std::string l_str;
		l_str.resize(65535);
		va_list vlArgs;
		va_start(vlArgs, sFormatMsg);
		vsprintf(&l_str[0], sFormatMsg, vlArgs);
		va_end(vlArgs);
		//printf("%s\r",l_str.c_str());
		syslog(LOG_NOTICE, "%s", l_str.c_str());
	}
#endif
	
	if (pDbgItemList == NULL)
	{
		return;
	}
	va_list vlArgs;
	va_start(vlArgs, sFormatMsg);
	
	const int iRet = vsprintf(sDebugHead, sFormatMsg, vlArgs);
	
	va_end(vlArgs);
	
	if (iRet < 0 || iRet >= 65535)
	{
		AppendDebugLogFormat("[ERR] vsprintf wrong value %d in clsUdpDebug::Broadcast\n", iRet);
		
		return;
	}
	
	((uint16_t *)sDebugBuffer)[1] = (uint16_t)iRet;
	size_t szLen = (sDebugHead - sDebugBuffer) + iRet;
	
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL && pNext->bAllData == true)
	{
		pCur = pNext;
		pNext = pCur->pNext;
#ifdef _WIN32
		sendto(pCur->s, sDebugBuffer, (int)szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#else
		sendto(pCur->s, sDebugBuffer, szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#endif
		clsServerManager::ui64BytesSent += szLen;
	}
}
//---------------------------------------------------------------------------

void clsUdpDebug::CreateBuffer()
{
	if (sDebugBuffer != NULL)
	{
		return;
	}
	
	sDebugBuffer = (char *)malloc(4 + 256 + 65535);
	if (sDebugBuffer == NULL)
	{
		AppendDebugLog("%s - [MEM] Cannot allocate 4+256+65535 bytes for sDebugBuffer in clsUdpDebug::CreateBuffer\n");
		
		exit(EXIT_FAILURE);
	}
	
	UpdateHubName();
}
//---------------------------------------------------------------------------

bool clsUdpDebug::New(User * pUser, const uint16_t ui16Port)
{
	UdpDbgItem * pNewDbg = new(std::nothrow) UdpDbgItem();
	if (pNewDbg == NULL)
	{
		AppendDebugLog("%s - [MEM] Cannot allocate pNewDbg in clsUdpDebug::New\n");
		return false;
	}
	
	// initialize dbg item
	pNewDbg->sNick = (char *)malloc(pUser->ui8NickLen + 1);
	if (pNewDbg->sNick == NULL)
	{
		AppendDebugLogFormat("[MEM] Cannot allocate %" PRIu8 " bytes for sNick in clsUdpDebug::New\n", pUser->ui8NickLen + 1);
		
		delete pNewDbg;
		return false;
	}
	
	memcpy(pNewDbg->sNick, pUser->sNick, pUser->ui8NickLen);
	pNewDbg->sNick[pUser->ui8NickLen] = '\0';
	
	pNewDbg->ui32Hash = pUser->ui32NickHash;
	
	struct in6_addr i6addr;
	memcpy(&i6addr, &pUser->ui128IpHash, 16);
	
	bool bIPv6 = (IN6_IS_ADDR_V4MAPPED(&i6addr) == 0);
	
	if (bIPv6 == true)
	{
		((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_port = htons(ui16Port);
		memcpy(((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_addr.s6_addr, pUser->ui128IpHash, 16);
		pNewDbg->sas_len = sizeof(struct sockaddr_in6);
	}
	else
	{
		((struct sockaddr_in *)&pNewDbg->sas_to)->sin_family = AF_INET;
		((struct sockaddr_in *)&pNewDbg->sas_to)->sin_port = htons(ui16Port);
		((struct sockaddr_in *)&pNewDbg->sas_to)->sin_addr.s_addr = inet_addr(pUser->sIP);
		pNewDbg->sas_len = sizeof(struct sockaddr_in);
	}
	
	pNewDbg->s = socket((bIPv6 == true ? AF_INET6 : AF_INET), SOCK_DGRAM, IPPROTO_UDP);
#ifdef _WIN32
	if (pNewDbg->s == INVALID_SOCKET)
	{
		int iErr = WSAGetLastError();
#else
	if (pNewDbg->s == -1)
	{
#endif
		pUser->SendFormat("clsUdpDebug::New1", true, "*** [ERR] %s: %s (%d).|", clsLanguageManager::mPtr->sTexts[LAN_UDP_SCK_CREATE_ERR],
#ifdef _WIN32
		                  WSErrorStr(iErr), iErr);
#else
		                  ErrnoStr(errno), errno);
#endif
		delete pNewDbg;
		return false;
	}
	
	// set non-blocking
#ifdef _WIN32
	uint32_t block = 1;
	if (SOCKET_ERROR == ioctlsocket(pNewDbg->s, FIONBIO, (unsigned long *)&block))
	{
		int iErr = WSAGetLastError();
#else
	int oldFlag = fcntl(pNewDbg->s, F_GETFL, 0);
	if (fcntl(pNewDbg->s, F_SETFL, oldFlag | O_NONBLOCK) == -1)
	{
#endif
		pUser->SendFormat("clsUdpDebug::New2", true, "*** [ERR] %s: %s (%d).|", clsLanguageManager::mPtr->sTexts[LAN_UDP_NON_BLOCK_FAIL],
#ifdef _WIN32
		                  WSErrorStr(iErr), iErr);
#else
		                  ErrnoStr(errno), errno);
#endif
		delete pNewDbg;
		return false;
	}
	
	pNewDbg->pPrev = NULL;
	pNewDbg->pNext = NULL;
	
	if (pDbgItemList == NULL)
	{
		CreateBuffer();
		pDbgItemList = pNewDbg;
	}
	else
	{
		pDbgItemList->pPrev = pNewDbg;
		pNewDbg->pNext = pDbgItemList;
		pDbgItemList = pNewDbg;
	}
	
	pNewDbg->bIsScript = false;
	
	int iLen = sprintf(sDebugHead, "[HUB] Subscribed, users online: %u", clsServerManager::ui32Logged);
	if (iLen < 0 || iLen > 65535)
	{
		AppendDebugLogFormat("[ERR] sprintf wrong value %d in clsUdpDebug::New\n", iLen);
		
		return true;
	}
	
	// create packet
	((uint16_t *)sDebugBuffer)[1] = (uint16_t)iLen;
	size_t szLen = (sDebugHead - sDebugBuffer) + iLen;
#ifdef _WIN32
	sendto(pNewDbg->s, sDebugBuffer, (int)szLen, 0, (struct sockaddr *)&pNewDbg->sas_to, pNewDbg->sas_len);
#else
	sendto(pNewDbg->s, sDebugBuffer, szLen, 0, (struct sockaddr *)&pNewDbg->sas_to, pNewDbg->sas_len);
#endif
	clsServerManager::ui64BytesSent += szLen;
	
	return true;
}
//---------------------------------------------------------------------------

bool clsUdpDebug::New(const char * sIP, const uint16_t ui16Port, const bool bAllData, char * sScriptName)
{
	UdpDbgItem * pNewDbg = new(std::nothrow) UdpDbgItem();
	if (pNewDbg == NULL)
	{
		AppendDebugLog("%s - [MEM] Cannot allocate pNewDbg in clsUdpDebug::New\n");
		return false;
	}
	
	// initialize dbg item
	size_t szNameLen = strlen(sScriptName);
	pNewDbg->sNick = (char *)malloc(szNameLen + 1);
	if (pNewDbg->sNick == NULL)
	{
		AppendDebugLogFormat("[MEM] Cannot allocate %" PRIu64 " bytes for sNick in clsUdpDebug::New\n", (uint64_t)(szNameLen + 1));
		
		delete pNewDbg;
		return false;
	}
	
	memcpy(pNewDbg->sNick, sScriptName, szNameLen);
	pNewDbg->sNick[szNameLen] = '\0';
	
	pNewDbg->ui32Hash = 0;
	
	uint8_t ui128IP[16];
	HashIP(sIP, ui128IP);
	
	struct in6_addr i6addr;
	memcpy(&i6addr, &ui128IP, 16);
	
	bool bIPv6 = (IN6_IS_ADDR_V4MAPPED(&i6addr) == 0);
	
	if (bIPv6 == true)
	{
		((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_port = htons(ui16Port);
		memcpy(((struct sockaddr_in6 *)&pNewDbg->sas_to)->sin6_addr.s6_addr, ui128IP, 16);
		pNewDbg->sas_len = sizeof(struct sockaddr_in6);
	}
	else
	{
		((struct sockaddr_in *)&pNewDbg->sas_to)->sin_family = AF_INET;
		((struct sockaddr_in *)&pNewDbg->sas_to)->sin_port = htons(ui16Port);
		memcpy(&((struct sockaddr_in *)&pNewDbg->sas_to)->sin_addr.s_addr, ui128IP + 12, 4);
		pNewDbg->sas_len = sizeof(struct sockaddr_in);
	}
	
	pNewDbg->s = socket((bIPv6 == true ? AF_INET6 : AF_INET), SOCK_DGRAM, IPPROTO_UDP);
	
#ifdef _WIN32
	if (pNewDbg->s == INVALID_SOCKET)
	{
#else
	if (pNewDbg->s == -1)
	{
#endif
		delete pNewDbg;
		return false;
	}
	
	// set non-blocking
#ifdef _WIN32
	uint32_t block = 1;
	if (SOCKET_ERROR == ioctlsocket(pNewDbg->s, FIONBIO, (unsigned long *)&block))
	{
#else
	int oldFlag = fcntl(pNewDbg->s, F_GETFL, 0);
	if (fcntl(pNewDbg->s, F_SETFL, oldFlag | O_NONBLOCK) == -1)
	{
#endif
		delete pNewDbg;
		return false;
	}
	
	pNewDbg->pPrev = NULL;
	pNewDbg->pNext = NULL;
	
	if (pDbgItemList == NULL)
	{
		CreateBuffer();
		pDbgItemList = pNewDbg;
	}
	else
	{
		pDbgItemList->pPrev = pNewDbg;
		pNewDbg->pNext = pDbgItemList;
		pDbgItemList = pNewDbg;
	}
	
	pNewDbg->bIsScript = true;
	pNewDbg->bAllData = bAllData;
	
	int iLen = sprintf(sDebugHead, "[HUB] Subscribed, users online: %u", clsServerManager::ui32Logged);
	if (iLen < 0 || iLen > 65535)
	{
		AppendDebugLogFormat("[ERR] sprintf wrong value %d in clsUdpDebug::New2\n", iLen);
		
		return true;
	}
	
	// create packet
	((uint16_t *)sDebugBuffer)[1] = (uint16_t)iLen;
	size_t szLen = (sDebugHead - sDebugBuffer) + iLen;
#ifdef _WIN32
	sendto(pNewDbg->s, sDebugBuffer, (int)szLen, 0, (struct sockaddr *)&pNewDbg->sas_to, pNewDbg->sas_len);
#else
	sendto(pNewDbg->s, sDebugBuffer, szLen, 0, (struct sockaddr *)&pNewDbg->sas_to, pNewDbg->sas_len);
#endif
	clsServerManager::ui64BytesSent += szLen;
	
	return true;
}
//---------------------------------------------------------------------------

void clsUdpDebug::DeleteBuffer()
{
	safe_free(sDebugBuffer);
	sDebugHead = NULL;
}
//---------------------------------------------------------------------------

bool clsUdpDebug::Remove(User * pUser)
{
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pCur->pNext;
		
		if (pCur->bIsScript == false && pCur->ui32Hash == pUser->ui32NickHash && strcasecmp(pCur->sNick, pUser->sNick) == 0)
		{
			if (pCur->pPrev == NULL)
			{
				if (pCur->pNext == NULL)
				{
					pDbgItemList = NULL;
					DeleteBuffer();
				}
				else
				{
					pCur->pNext->pPrev = NULL;
					pDbgItemList = pCur->pNext;
				}
			}
			else if (pCur->pNext == NULL)
			{
				pCur->pPrev->pNext = NULL;
			}
			else
			{
				pCur->pPrev->pNext = pCur->pNext;
				pCur->pNext->pPrev = pCur->pPrev;
			}
			
			delete pCur;
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------

void clsUdpDebug::Remove(const char * sScriptName)
{
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pCur->pNext;
		
		if (pCur->bIsScript == true && strcasecmp(pCur->sNick, sScriptName) == 0)
		{
			if (pCur->pPrev == NULL)
			{
				if (pCur->pNext == NULL)
				{
					pDbgItemList = NULL;
					DeleteBuffer();
				}
				else
				{
					pCur->pNext->pPrev = NULL;
					pDbgItemList = pCur->pNext;
				}
			}
			else if (pCur->pNext == NULL)
			{
				pCur->pPrev->pNext = NULL;
			}
			else
			{
				pCur->pPrev->pNext = pCur->pNext;
				pCur->pNext->pPrev = pCur->pPrev;
			}
			
			delete pCur;
			return;
		}
	}
}
//---------------------------------------------------------------------------

bool clsUdpDebug::CheckUdpSub(User * pUser, bool bSndMess/* = false*/) const
{
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pCur->pNext;
		
		if (pCur->bIsScript == false && pCur->ui32Hash == pUser->ui32NickHash && strcasecmp(pCur->sNick, pUser->sNick) == 0)
		{
			if (bSndMess == true)
			{
				pUser->SendFormat("clsUdpDebug::CheckUdpSub", true, "<%s> *** %s %hu. %s.|", clsSettingManager::mPtr->sPreTexts[clsSettingManager::SETPRETXT_HUB_SEC], clsLanguageManager::mPtr->sTexts[LAN_YOU_SUBSCRIBED_UDP_DBG],
				                  ntohs(pCur->sas_to.ss_family == AF_INET6 ? ((struct sockaddr_in6 *)&pCur->sas_to)->sin6_port : ((struct sockaddr_in *)&pCur->sas_to)->sin_port), clsLanguageManager::mPtr->sTexts[LAN_TO_UNSUB_UDP_DBG]);
			}
			
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------

void clsUdpDebug::Send(const char * sScriptName, char * sMessage, const size_t szMsgLen) const
{
	if (pDbgItemList == NULL)
	{
		return;
	}
	
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pCur->pNext;
		
		if (pCur->bIsScript == true && strcasecmp(pCur->sNick, sScriptName) == 0)
		{
			// create packet
			((uint16_t *)sDebugBuffer)[1] = (uint16_t)szMsgLen;
			memcpy(sDebugHead, sMessage, szMsgLen);
			size_t szLen = (sDebugHead - sDebugBuffer) + szMsgLen;
			
#ifdef _WIN32
			sendto(pCur->s, sDebugBuffer, (int)szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#else
			sendto(pCur->s, sDebugBuffer, szLen, 0, (struct sockaddr *)&pCur->sas_to, pCur->sas_len);
#endif
			clsServerManager::ui64BytesSent += szLen;
			
			return;
		}
	}
}
//---------------------------------------------------------------------------

void clsUdpDebug::Cleanup()
{
	UdpDbgItem * pCur = NULL,
	             * pNext = pDbgItemList;
	             
	while (pNext != NULL)
	{
		pCur = pNext;
		pNext = pCur->pNext;
		
		delete pCur;
	}
	
	pDbgItemList = NULL;
}
//---------------------------------------------------------------------------

void clsUdpDebug::UpdateHubName()
{
	if (sDebugBuffer == NULL)
	{
		return;
	}
	
	((uint16_t *)sDebugBuffer)[0] = (uint16_t)clsSettingManager::mPtr->ui16TextsLens[SETTXT_HUB_NAME];
	memcpy(sDebugBuffer + 4, clsSettingManager::mPtr->sTexts[SETTXT_HUB_NAME], clsSettingManager::mPtr->ui16TextsLens[SETTXT_HUB_NAME]);
	
	sDebugHead = sDebugBuffer + 4 + clsSettingManager::mPtr->ui16TextsLens[SETTXT_HUB_NAME];
}
//---------------------------------------------------------------------------
