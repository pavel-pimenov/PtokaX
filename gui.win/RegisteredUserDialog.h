/*
 * PtokaX - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2004-2017  Petr Kozelka, PPK at PtokaX dot org

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

//------------------------------------------------------------------------------
#ifndef RegisteredUserDialogH
#define RegisteredUserDialogH
//------------------------------------------------------------------------------
struct RegUser;
//------------------------------------------------------------------------------

class clsRegisteredUserDialog
{
	public:
		static clsRegisteredUserDialog * mPtr;
		
		HWND m_hWndWindowItems[9];
		
		enum enmWindowItems
		{
			WINDOW_HANDLE,
			GB_NICK,
			EDT_NICK,
			GB_PASSWORD,
			EDT_PASSWORD,
			GB_PROFILE,
			CB_PROFILE,
			BTN_ACCEPT,
			BTN_DISCARD
		};
		
		clsRegisteredUserDialog();
		~clsRegisteredUserDialog();
		
		static LRESULT CALLBACK StaticRegisteredUserDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		void DoModal(HWND hWndParent, RegUser * pReg = NULL, const char * sNick = NULL);
		void UpdateProfiles();
		void RegChanged(RegUser * pReg);
		void RegDeleted(RegUser * pReg);
	private:
		RegUser * pRegToChange;
		
		LRESULT RegisteredUserDialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		bool OnAccept();
		DISALLOW_COPY_AND_ASSIGN(clsRegisteredUserDialog);
};
//------------------------------------------------------------------------------

#endif
