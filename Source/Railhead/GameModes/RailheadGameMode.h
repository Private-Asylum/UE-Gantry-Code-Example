// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/GameModeBase.h"

#include "RailheadGameMode.generated.h"

// @doc
/**
 * Railhead's rules, on the server only: the train and the line are built at the origin before
 * anyone joins, and crew join aboard in their own colour.
 *
 * Deliberately small. Which stretch the line runs, and when it changes, is Gantry's business,
 * driven by ARailheadLine; the game mode never chooses an experience. Gantry's startup ladder
 * does (the map's entry in MapExperiences, or ?Experience= on the URL).
 */
UCLASS()
class RAILHEAD_API ARailheadGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ARailheadGameMode();

	/** Builds the train and the line, before any player logs in. */
	virtual void PreInitializeComponents() override;

	/** Gives each new crew member the next colour of overalls. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Spawns crew on the train's wagons instead of at player starts. */
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** The train to build. A subclass can swap in its own. */
	UPROPERTY(EditDefaultsOnly, Category = "Railhead")
	TSubclassOf<class ARailheadTrain> TrainClass;

	/** The line to build. */
	UPROPERTY(EditDefaultsOnly, Category = "Railhead")
	TSubclassOf<class ARailheadLine> LineClass;

private:

	/** How many crew have joined, for their colour and their spot on the train. */
	int32 Joined = 0;
};
