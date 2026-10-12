// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GantrySettingsProvider.h"

#include "RailheadSettingsProvider.generated.h"

// @doc
/**
 * Railhead's settings, declared in code for each local player: the Video page (the display and how
 * much the game draws, through the engine's user settings) and the Crew page (field of view and
 * look, kept in the player's save).
 *
 * Gantry provides the shapes and the bookkeeping; every value here is read and written by a
 * function bound to the setting. The Coast Line's own page is an asset its experience adds
 * (SP_Coast), to show the other way in.
 *
 * Listed in Project Settings > Plugins > Gantry Settings > Providers.
 */
UCLASS()
class URailheadSettingsProvider : public UGantrySettingsProvider
{
	GENERATED_BODY()

public:

	/** Video for the first player only (the display is the machine's), Crew for everyone. */
	virtual TArray<UGantrySettingsPage*> CreatePages_Implementation(UGantrySettingsRegistry* Registry) override;

private:

	/** Window mode, resolution, quality, resolution scale, frame rate limit, vertical sync. */
	static UGantrySettingsPage* CreateVideoPage(UGantrySettingsRegistry* Registry);

	/** Field of view, look sensitivity, inverted look. */
	static UGantrySettingsPage* CreateCrewPage(UGantrySettingsRegistry* Registry);
};

// @doc
/**
 * Railhead's Controls page: one key-binding setting per rebindable key, read from Enhanced Input's
 * player settings. Those keys exist only once their mapping context is registered for rebinding,
 * which EXP_Base's Add Input Mappings stage does; so the page arrives from the same experience,
 * from an Add Settings Pages stage after it, rather than from the project's list.
 */
UCLASS()
class URailheadControlsProvider : public UGantrySettingsProvider
{
	GENERATED_BODY()

public:

	/** The Controls page, for this player's rebindable keys. */
	virtual TArray<UGantrySettingsPage*> CreatePages_Implementation(UGantrySettingsRegistry* Registry) override;
};
