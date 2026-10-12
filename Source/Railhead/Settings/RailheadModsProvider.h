// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GantrySettingsProvider.h"

#include "RailheadModsProvider.generated.h"

// @doc
/**
 * Railhead's Mods page: one toggle per mod Gantry found, turning it on or off for this player.
 * A change takes effect when the settings are applied: an enabled mod mounts, a disabled one is
 * taken out (or, while the train runs on it, at the next start). A mod Gantry refused is listed
 * with the reason, and cannot be turned on.
 *
 * The page is built again whenever Gantry reports on its mods (a mount, an unmount, a mod dropped
 * in), so it always lists what is there. The first player's only: mods belong to the machine.
 *
 * Listed in Project Settings > Plugins > Gantry Settings > Providers.
 */
UCLASS()
class URailheadModsProvider : public UGantrySettingsProvider
{
	GENERATED_BODY()

public:

	/** The Mods page, when there are mods and this is the first player. */
	virtual TArray<UGantrySettingsPage*> CreatePages_Implementation(UGantrySettingsRegistry* Registry) override;

private:

	/** Builds the page again on the registry, a frame after Gantry's report. */
	void Rebuild();

	/** The registry this provider built for. */
	TWeakObjectPtr<UGantrySettingsRegistry> BuiltFor;

	/** Set once this provider has handed over to a new one. */
	bool bRetired = false;
};
