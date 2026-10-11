// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "Experience/GantryExperienceTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RailheadLoadingScreenSubsystem.generated.h"

class FRailheadLoadFeed;
class SRailheadLoadingScreen;

// @doc
/**
 * Puts the loading screen up whenever an experience activates, and takes it down when the feed
 * has been shown.
 *
 * The first activation in a world is covered completely: the curtain goes up as the world
 * initialises, so nothing is seen half built, and stays until the experience is ready on this
 * machine (on a client, the server's choice, resolved and loaded here) and the feed has had its
 * time. Every later activation, a stretch swapping in a tunnel, gets the panel alone, over the
 * running game. A failed activation ends the screen too, so the player can see what happened.
 * Dedicated servers have no screen.
 */
UCLASS()
class RAILHEAD_API URailheadLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** The feed is spread over this long, and the screen stays up at least this long. */
	UPROPERTY(EditAnywhere, Category = "Railhead|Loading", meta = (ClampMin = "0.0", Units = "s"))
	float MinimumSeconds = 5.0f;

	/** Seconds between reads of Gantry's load snapshot. */
	UPROPERTY(EditAnywhere, Category = "Railhead|Loading", meta = (ClampMin = "0.0", Units = "s"))
	float CollectSeconds = 0.05f;

	/** The longest the screen stays up; past this it lifts whatever state the world is in. */
	UPROPERTY(EditAnywhere, Category = "Railhead|Loading", meta = (ClampMin = "1.0", Units = "s"))
	float MaximumSeconds = 120.0f;

private:

	void HandleWorldInitialized(UWorld* World, const UWorld::InitializationValues Values);

	/** An activation starting opens the screen (if it is not up already). */
	UFUNCTION()
	void HandleStateChanged(EGantryExperienceState OldState, EGantryExperienceState NewState);

	bool Tick(float DeltaSeconds);
	void Open(UWorld* World, bool bBlocking);
	void Close();

	/** Whether there is nothing left to wait for. */
	bool IsActivationOver() const;

	TSharedPtr<FRailheadLoadFeed> Feed;
	TSharedPtr<SRailheadLoadingScreen> Screen;
	TWeakObjectPtr<UWorld> ScreenWorld;
	bool bScreenBlocking = false;
	double OpenedAt = 0.0;
	double CollectedAt = 0.0;

	/** The world has been shown: later activations in it get the panel, not the curtain. */
	TWeakObjectPtr<UWorld> ShownWorld;

	FDelegateHandle WorldInitHandle;
	FTSTicker::FDelegateHandle TickHandle;
};
