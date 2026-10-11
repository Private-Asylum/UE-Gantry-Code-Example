// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Railhead's code-side gameplay tags. Experience identities (Experience.*) live in
 * DefaultGameplayTags.ini instead, because data assets name them, not code.
 */
namespace RailheadTags
{
	/** Railhead.Stage.Scenery: the stage that loads a stretch's scenery. */
	RAILHEAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stage_Scenery);

	/** Railhead.Message.Line: everything the line announces (listen here, partially, for all of it). */
	RAILHEAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Line);

	/** Railhead.Message.Line.Tunnel: the train has gone into a tunnel; the stretch beyond is loading. */
	RAILHEAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Line_Tunnel);

	/** Railhead.Message.Line.Daylight: the train has come out of a tunnel into a new stretch. */
	RAILHEAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Line_Daylight);

	/** Railhead.HUD.Notes: the HUD's notes column, a UI extension slot routes and mods fill. */
	RAILHEAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HUD_Notes);
}
