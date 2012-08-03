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
* all respects for all other code used.  Additionally, the Source.Python
* Development Team grants this exception to all derivative works.  
*/

//---------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------
#include "export_main.h"
#include "utility/sp_util.h"
#include "core/sp_python.h"
#include "utility/wrap_macros.h"
#include "eiface.h"

//---------------------------------------------------------------------------------
// Namespaces to use.
//---------------------------------------------------------------------------------
using namespace boost::python;

//---------------------------------------------------------------------------------
// External variables to use.
//---------------------------------------------------------------------------------
extern IVEngineServer* engine;

//---------------------------------------------------------------------------------
// Returns the engine instance.
//---------------------------------------------------------------------------------
IVEngineServer* GetEngine( void )
{
	return engine;
}

//---------------------------------------------------------------------------------
// IVEngineServer overloads
//---------------------------------------------------------------------------------
DECLARE_CLASS_METHOD_OVERLOAD(IVEngineServer, PrecacheModel, 1, 2);
DECLARE_CLASS_METHOD_OVERLOAD(IVEngineServer, PrecacheSentenceFile, 1, 2);
DECLARE_CLASS_METHOD_OVERLOAD(IVEngineServer, PrecacheDecal, 1, 2);
DECLARE_CLASS_METHOD_OVERLOAD(IVEngineServer, PrecacheGeneric, 1, 2);

//---------------------------------------------------------------------------------
// Wraps game events related structures.
//---------------------------------------------------------------------------------
DECLARE_SP_MODULE(Engine)
{
	// ----------------------------------------------------------
	// CBaseHandle
	// ----------------------------------------------------------
	typedef bool (CBaseHandle::*BaseHandleFn)(const CBaseHandle&) const;
	
	BaseHandleFn BaseHandleEq = &CBaseHandle::operator ==;
	BaseHandleFn BaseHandleNEq = &CBaseHandle::operator !=;
	BaseHandleFn BaseHandleLt = &CBaseHandle::operator <;

	BOOST_ABSTRACT_CLASS(CBaseHandle)

		CLASS_METHOD(CBaseHandle,
			IsValid,
			"Returns true if the handle has been initted with any values"
		)

		CLASS_METHOD(CBaseHandle,
			IsValid,
			"Returns true if the handle has been initted with any values"
		)

		CLASS_METHOD_TYPEDEF(
			__eq__,
			BaseHandleEq,
			"Returns true if the given CBaseHandle points to the same entity."
		)

		CLASS_METHOD_TYPEDEF(
			__ne__,
			BaseHandleNEq,
			"Returns true if the given CBaseHandle is not equal to this handle."
		)

		CLASS_METHOD_TYPEDEF(
			__lt__,
			BaseHandleLt,
			"Returns true if the given CBaseHandle is less than this handle."
		)

	BOOST_END_CLASS()

	// ----------------------------------------------------------
	// IHandleEntity.
	// ----------------------------------------------------------
	BOOST_ABSTRACT_CLASS(IHandleEntity)
		CLASS_METHOD(IHandleEntity,
			GetRefEHandle,
			"Returns the CBaseHandle instance for this entity.",
			reference_existing_object_policy()
		)
	BOOST_END_CLASS()


	// ----------------------------------------------------------
	// INetworkable interface.
	// ----------------------------------------------------------
	BOOST_ABSTRACT_CLASS(IServerNetworkable)
		
		CLASS_METHOD(IServerNetworkable,
			GetEntityHandle,
			"Returns the entity handle associated with the collideable.",
			reference_existing_object_policy()
		)

		CLASS_METHOD(IServerNetworkable,
			GetEdict,
			"Returns the edict for this entity.",
			reference_existing_object_policy()
		)
		
		CLASS_METHOD(IServerNetworkable,
			GetClassName,
			"Returns the classname for this object."
		)

	BOOST_END_CLASS()


	// ----------------------------------------------------------
	// IServerUnknown
	// ----------------------------------------------------------
	BOOST_ABSTRACT_CLASS_INHERITED(IServerUnknown, IHandleEntity)

		CLASS_METHOD(IServerUnknown,
			GetCollideable,
			"Returns the ICollideable object for this entity.",
			reference_existing_object_policy()
		)

		CLASS_METHOD(IServerUnknown,
			GetNetworkable,
			"Returns the IServerNetworkable object for this entity.",
			reference_existing_object_policy()
		)

	BOOST_END_CLASS()
	

	// ----------------------------------------------------------
	// The engine interface.
	// ----------------------------------------------------------
	BOOST_ABSTRACT_CLASS( IVEngineServer )
		
		CLASS_METHOD(IVEngineServer,
			ChangeLevel,
			"Tells the engine to change the level. If s2 is None, the engine will execute a \
			changelevel command. If s2 is a valid map, the engine will execute a changelevel2 \
			command",
			args("s1", "s2")
		)

		CLASS_METHOD(IVEngineServer,
			IsMapValid,
			"Returns true if filename refers to a valid map.",
			args("filename")
		)

		CLASS_METHOD(IVEngineServer,
			IsDedicatedServer,
			"Returns true if the engine is running in dedicated mode."
		)

		CLASS_METHOD(IVEngineServer,
			IsInEditMode,
			"Returns false if the engine is not in hammer editing mode."
		)

		CLASS_METHOD(IVEngineServer,
			GetLaunchOptions,
			"Returns a keyvalues structure containing launch options for srcds.",
			reference_existing_object_policy()
		)

		CLASS_METHOD_OVERLOAD(IVEngineServer,
			PrecacheModel,
			"Precaches a model and returns an integer containing its index.",
			args("s", "preload")
		)

		CLASS_METHOD_OVERLOAD(IVEngineServer,
			PrecacheSentenceFile,
			"Precaches a sentence file and returns an integer containing its index.",
			args("s", "preload")
		)

		CLASS_METHOD_OVERLOAD(IVEngineServer,
			PrecacheDecal,
			"Precaches a decal file and returns an integer containing its index.",
			args("name", "preload")
		)

		CLASS_METHOD_OVERLOAD(IVEngineServer,
			PrecacheGeneric,
			"Precaches a generic asset file and returns an integer containing its index.",
			args("s", "preload")
		)

		CLASS_METHOD(IVEngineServer,
			IsModelPrecached,
			"Returns true if the given model is precached.",
			args("s")
		)

		CLASS_METHOD(IVEngineServer,
			IsDecalPrecached,
			"Returns true if the given decal is precached.",
			args("s")
		)

		CLASS_METHOD(IVEngineServer,
			IsGenericPrecached,
			"Returns true if the given generic asset is precached.",
			args("s")
		)

		CLASS_METHOD(IVEngineServer,
			GetClusterForOrigin,
			"Returns the cluster number for the specified position.",
			args("origin")
		)

		CLASS_METHOD(IVEngineServer,
			GetPVSForCluster,
			"Gets the PVS bits for a specified cluster and copies the bits into outputpvs.",
			args("cluster", "outputpvslength", "outputpvs")
		)

		CLASS_METHOD(IVEngineServer,
			CheckOriginInPVS,
			"Check whether the specified origin is inside the PVS",
			args("org", "checkpvs", "checkpvssize")
		)

		CLASS_METHOD(IVEngineServer,
			CheckBoxInPVS,
			"Checks whether the specified worldspace bounding box is inside the specified PVS",
			args("mins", "maxs", "checkpvs", "checkpvssize")
		)

		CLASS_METHOD(IVEngineServer,
			GetPlayerUserId,
			"Returns the server assigned userid for this player. Returns -1 if the edict could not \
			be found.",
			args("playerInstance")
		)

		CLASS_METHOD(IVEngineServer,
			GetPlayerNetworkIDString,
			"Returns the player's network id as a string.",
			args("playerInstance")
		)

		CLASS_METHOD(IVEngineServer,
			IsUserIDInUse,
			"Returns true if the userid given is in use.",
			args("userID")
		)

		CLASS_METHOD(IVEngineServer,
			GetLoadingProgressForUserID,
			"Returns percentage complete a player is while loading.",
			args("userID")
		)

		CLASS_METHOD(IVEngineServer,
			GetEntityCount,
			"Returns the number of used edict slots."
		)

// 		CLASS_METHOD(IVEngineServer,
// 			GetPlayerNetInfo,
// 			"Returns stats info interface for a client netchannel.",
// 			args("playerIndex"),
// 			reference_existing_object_policy()
// 		)

		CLASS_METHOD(IVEngineServer,
			CreateEdict,
			"Creates an edict. If iForceEdictIndex is not -1, then it return the edict with that index.",
			args("iForceEdictIndex"),
			reference_existing_object_policy()
		)

		CLASS_METHOD(IVEngineServer,
			RemoveEdict,
			"Remove the specified edict and place back into the free edict list.",
			args("edictInstance")
		)

		// For stephen
		CLASS_METHOD(IVEngineServer,
			ServerCommand,
			"Issues a command to the command parser as if it was typed at the server console.",
			args("command")
		)

		CLASS_METHOD(IVEngineServer,
			ServerExecute,
			"Executes any commands currently in the command parser immediately (instead of once per frame).",
			args("command")
		)

		CLASS_METHOD(IVEngineServer,
			UserMessageBegin,
			"Begin a message from the server to the client.dll",
			args("filter", "msg_type", "msgname"),
			reference_existing_object_policy()
		)

		CLASS_METHOD(IVEngineServer,
			EntityMessageBegin,
			"Creates a usermessage using an entity as the source.",
			args("entindex", "ent_serverclass", "isReliable"),
			reference_existing_object_policy()
		)

		CLASS_METHOD(IVEngineServer,
			MessageEnd,
			"Finish entity or user message and dispatch it.",
			reference_existing_object_policy()
		)

	BOOST_END_CLASS()

	// ----------------------------------------------------------
	// Expose the global interface to the event manager.
	// ----------------------------------------------------------
	BOOST_FUNCTION(GetEngine, reference_existing_object_policy());

	// ----------------------------------------------------------
	// Expose some entity functions
	// ----------------------------------------------------------
	BOOST_FUNCTION(PEntityOfEntIndex, 
		"Returns the edict for an entity index.", 
		reference_existing_object_policy()
	);
}