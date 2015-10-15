/*
 * PtokaX - hub server for Direct Connect peer to peer network.

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
#ifndef UDPThreadH
#define UDPThreadH
//---------------------------------------------------------------------------

class UDPThread
{
	private:
#ifdef _WIN32
		HANDLE hThreadHandle;
		
		SOCKET sock;
		
		unsigned int threadId;
#else
		pthread_t threadId;
		
		int sock;
#endif
		
		bool bTerminated;
		
		char rcvbuf[4096];
		
		DISALLOW_COPY_AND_ASSIGN(UDPThread);
	public:
		static UDPThread * mPtrIPv4;
		static UDPThread * mPtrIPv6;
		
		UDPThread();
		~UDPThread();
		
		bool Listen(const int iAddressFamily);
		void Resume();
		void Run();
		void Close();
		void WaitFor();
		
		static UDPThread * Create(const int iAddressFamily);
		static void Destroy(UDPThread * pUDPThread);
};
//---------------------------------------------------------------------------

#endif
