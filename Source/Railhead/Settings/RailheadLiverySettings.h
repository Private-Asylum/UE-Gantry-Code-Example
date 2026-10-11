// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "RailheadLiverySettings.generated.h"

// @doc
/**
 * The paint the train's cars are turned out in (Project Settings -> Game -> Livery).
 *
 * Config, so a mod can repaint the train without code or assets: its Config/<Mod>Game.ini sets
 * a key in this section, and when its game feature activates the value lands here and the cars
 * repaint at the next stretch. Two mods setting the same key: the last to activate wins, and
 * Gantry reports the clash.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Livery"))
class RAILHEAD_API URailheadLiverySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/** @return The settings. */
	static const URailheadLiverySettings* Get() { return GetDefault<URailheadLiverySettings>(); }

	/**
	 * The settings as the config has them now. A game feature's config is layered in when it
	 * activates, which changes the config but not these settings until they are read again.
	 * @return The settings, read again.
	 */
	static const URailheadLiverySettings* GetCurrent();

	/** The caboose's body. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Livery")
	FLinearColor Caboose = FLinearColor(0.45f, 0.05f, 0.03f);
};
