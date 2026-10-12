// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

/**
 * The menu Esc brings up aboard: carry on, settings, back to the main menu, or quit. The train
 * keeps running behind it; this is a crew game, and nobody pauses a train for one member.
 */
class SRailheadPauseMenu : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadPauseMenu) {}
		/** Any object in the world, for the session subsystem and the world's description. */
		SLATE_ARGUMENT(TWeakObjectPtr<UObject>, Context)
		/** Back to work was pressed. */
		SLATE_EVENT(FSimpleDelegate, OnResume)
		/** Settings was pressed. */
		SLATE_EVENT(FSimpleDelegate, OnSettings)
	SLATE_END_ARGS()

	/** Builds the menu: the world's description and the four buttons. */
	void Construct(const FArguments& InArgs);

private:

	/** Any object in the world, for the session subsystem. */
	TWeakObjectPtr<UObject> Context;
};
