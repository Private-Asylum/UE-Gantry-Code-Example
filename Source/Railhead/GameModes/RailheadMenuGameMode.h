// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameModes/RailheadGameMode.h"

#include "RailheadMenuGameMode.generated.h"

// @doc
/**
 * The main menu's rules: the same train and line as a ride, with nobody aboard. The menu map's
 * experience (Gantry's startup ladder: MapExperiences) runs the route behind the menu, the boiler
 * keeps itself fed, and the player watches from a camera alongside instead of walking.
 */
UCLASS()
class RAILHEAD_API ARailheadMenuGameMode : public ARailheadGameMode
{
	GENERATED_BODY()

public:

	ARailheadMenuGameMode();

	/** Nobody boards in the menu. */
	virtual void RestartPlayer(AController* NewPlayer) override {}

	/** Keeps the boiler fed, so the train never stops behind the menu. */
	virtual void Tick(float DeltaSeconds) override;
};
