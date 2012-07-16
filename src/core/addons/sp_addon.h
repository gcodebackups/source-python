/**
* =============================================================================
* Source Python
* Copyright (C) 2012 Source Python Development Team.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
* As a special exception, the Source Python Team gives you permission 
* to link the code of this program (as well as its derivative works) to 
* "Half-Life 2," the "Source Engine," and any Game MODs that run on software
* by the Valve Corporation.  You must obey the GNU General Public License in
* all respects for all other code used.  Additionally, the Eventscripts
* Development Team grants this exception to all derivative works.  
*/
#ifndef _sp_ADDON_H
#define _sp_ADDON_H

//---------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------
#include "core/sp_python.h"
#include "utllinkedlist.h"
#include "igameevents.h"

//---------------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------------
#define MAX_ADDON_NAME 64

//---------------------------------------------------------------------------------
// Addon manager.
//---------------------------------------------------------------------------------
class CAddonManager
{
	private:
		python::object m_SpPy;

	public:
		CAddonManager( void );
		~CAddonManager( void );

		// Addon functionality.
		bool LoadAddon( const char* szName );
		bool UnloadAddon( const char* szName );
		bool ReloadAddon( const char* szName );
		void PrintAddons( void );

		// Required by IGameEventListener2.
		void FireGameEvent( IGameEvent* event );
};

//---------------------------------------------------------------------------------
// Static singleton.
//---------------------------------------------------------------------------------
extern CAddonManager g_AddonManager;

#endif // _sp_ADDON_H