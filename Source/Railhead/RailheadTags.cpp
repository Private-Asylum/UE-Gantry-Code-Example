// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "RailheadTags.h"

namespace RailheadTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stage_Scenery, "Railhead.Stage.Scenery", "Loads a stretch's scenery and hands it to the line.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Line, "Railhead.Message.Line", "Everything the line announces.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Line_Tunnel, "Railhead.Message.Line.Tunnel", "The train has gone into a tunnel.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Line_Daylight, "Railhead.Message.Line.Daylight", "The train has come out of a tunnel into a new stretch.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HUD_Notes, "Railhead.HUD.Notes", "The HUD's notes column, a UI extension slot.");
}
