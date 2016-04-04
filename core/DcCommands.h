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
#ifndef DcCommandsH
#define DcCommandsH

#include "utility.h"

//---------------------------------------------------------------------------
struct User;
struct PrcsdUsrCmd;
struct PassBf;
//---------------------------------------------------------------------------

class clsDcCommands
{
	private:
		struct PassBf
		{
			PassBf * pPrev, * pNext;
			
			int iCount;
			
			Hash128 m_ui128IpHash;
			
			explicit PassBf(const uint8_t * ui128Hash);
			~PassBf(void) { };
			
			DISALLOW_COPY_AND_ASSIGN(PassBf);
		};
		
		PassBf * PasswdBfCheck;
		
		DISALLOW_COPY_AND_ASSIGN(clsDcCommands);
		
		static void BotINFO(User * pUser, const char * sData, const uint32_t ui32Len);
		static void ConnectToMe(User * pUser, char * sData, const uint32_t ui32LenLen, const bool bCheck, const bool bMulti);
		void GetINFO(User * pUser, const char * sData, const uint32_t ui32Len);
		static bool GetNickList(User * pUser, const char * sData, const uint32_t ui32Len, const bool bCheck);
		static void Key(User * pUser, char * sData, const uint32_t ui32Len);
		static void Kick(User * pUser, char * sData, const uint32_t ui32Len);
		static bool SearchDeflood(User * pUser, const char * sData, const uint32_t ui32Len, const bool bCheck, const bool bMulti);
		static void Search(User * pUser, char * sData, uint32_t ui32Len, const bool bCheck, const bool bMulti);
		static bool MyINFODeflood(User * pUser, const char * sData, const uint32_t ui32Len, const bool bCheck);
		static bool MyINFO(User * pUser, const char * sData, const uint32_t ui32Len);
		void MyPass(User * pUser, char * sData, const uint32_t ui32Len);
		static void OpForceMove(User * pUser, char * sData, const uint32_t ui32Len);
		static void RevConnectToMe(User * pUser, char * sData, const uint32_t ui32Len, const bool bCheck);
		static void SR(User * pUser, char * sData, const uint32_t ui32Len, const bool bCheck);
		void Supports(User * pUser, char * sData, const uint32_t ui32Len);
		static void To(User * pUser, char * sData, const uint32_t ui32Len, const bool bCheck);
		static void ValidateNick(User * pUser, char * sData, const uint32_t ui32Len);
		static void Version(User * pUser, char * sData, const uint32_t ui32Len);
		static bool ChatDeflood(User * pUser, char * sData, const uint32_t ui32Len, const bool bCheck);
		static void Chat(User * pUser, char * sData, const uint32_t ui32Len, const bool bCheck);
		static void Close(User * pUser, char * sData, const uint32_t ui32Len);
		
		void Unknown(User * pUser, char * sData, const uint32_t ui32Len, const bool bMyNick = false);
		void MyNick(User * pUser, char * sData, const uint32_t ui32Len);
		
		static bool ValidateUserNick(User * pUser, const char * sNick, const size_t szNickLen, const bool ValidateNick);
		
		static bool ValidateUserNickFinally(bool pIsNotReg, User * pUser, const size_t szNickLen, const bool ValidateNick); // [+] FlylinkDC++
#ifdef USE_FLYLINKDC_EXT_JSON
		bool ExtJSONDeflood(User * pUser, const char * sData, const uint32_t ui32Len, const bool bCheck);
		static bool SetExtJSON(User * pUser, const char * sData, const uint32_t ui32Len);
		static bool CheckExtJSON(User * pUser, const char * sData, const uint32_t ui32Len);
#endif
		
		PassBf * Find(const uint8_t * ui128IpHash);
		void Remove(PassBf * PassBfItem);
		
		static bool CheckIPPort(const User * pUser, char * sIP, bool &bWrongPort, uint16_t &ui16Port, uint8_t &ui8AfterPortLen, char cPortEnd);
		static bool GetPort(char * sData, uint16_t &ui16Port, uint8_t &ui8AfterPortLen, char cPortEnd);
		static void SendIncorrectPortMsg(User * pUser, const bool bCTM);
		static void SendIncorrectIPMsg(User * pUser, char * sBadIP, const bool bCTM);
		static void SendIPFixedMsg(User * pUser, const char * sBadIP, const char * sRealIP);
		
		static PrcsdUsrCmd * AddSearch(User * pUser, PrcsdUsrCmd * cmdSearch, const char * sSearch, const size_t szLen, const bool bActive);
	public:
		static clsDcCommands * mPtr;
		
		clsDcCommands();
		~clsDcCommands();
		
		void PreProcessData(User * pUser, char * sData, const bool bCheck, const uint32_t ui32Len);
		void ProcessCmds(User * pUser);
		
		static void SRFromUDP(User * pUser, const char * sData, const size_t szLen);
		
		uint32_t iStatChat, iStatCmdUnknown, iStatCmdTo, iStatCmdMyInfo, iStatCmdSearch, iStatCmdSR, iStatCmdRevCTM;
		uint32_t iStatCmdOpForceMove, iStatCmdMyPass, iStatCmdValidate, iStatCmdKey, iStatCmdGetInfo, iStatCmdGetNickList;
		uint32_t iStatCmdConnectToMe, iStatCmdVersion, iStatCmdKick, iStatCmdSupports, iStatBotINFO, iStatZPipe;
		uint32_t iStatCmdMultiSearch, iStatCmdMultiConnectToMe, iStatCmdClose;
#ifdef USE_FLYLINKDC_EXT_JSON
		uint32_t iStatCmdExtJSON;
#endif
};
//---------------------------------------------------------------------------

#endif
