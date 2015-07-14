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
#ifndef serviceLoopH
#define serviceLoopH
#include "CriticalSection.h"
//---------------------------------------------------------------------------
struct User;
//---------------------------------------------------------------------------

class clsServiceLoop {
private:
    struct AcceptedSocket {
        AcceptedSocket();


#ifdef _WIN32
        SOCKET s;
#else
		int s;
#endif

        sockaddr_storage addr;

        AcceptedSocket * pNext;
        DISALLOW_COPY_AND_ASSIGN(AcceptedSocket);
    };

    uint64_t ui64LstUptmTck;

    CriticalSection csAcceptQueue;

	AcceptedSocket * pAcceptedSocketsS, * pAcceptedSocketsE;

#ifndef _WIN32
      uint64_t ui64LastSecond;
#endif

	char msg[1024];

    DISALLOW_COPY_AND_ASSIGN(clsServiceLoop);

    void AcceptUser(AcceptedSocket * AccptSocket);
protected:
public:
    static clsServiceLoop * mPtr;

	clsServiceLoop();
	~clsServiceLoop();

#ifdef _WIN32
    static UINT_PTR srvLoopTimer;
#else
	uint64_t ui64LastRegToHublist;
#endif

    double dLoggedUsers, dActualSrvLoopLogins;

    uint32_t ui32LastSendRest,  ui32SendRestsPeak,  ui32LastRecvRest,  ui32RecvRestsPeak,  ui32LoopsForLogins;
    bool bRecv;

#ifdef _WIN32
	void AcceptSocket(const SOCKET &s, const sockaddr_storage &addr);
#else
	void AcceptSocket(const int &s, const sockaddr_storage &addr);
#endif
	void ReceiveLoop();
	void SendLoop();
	void Looper();
};
//---------------------------------------------------------------------------

#endif

