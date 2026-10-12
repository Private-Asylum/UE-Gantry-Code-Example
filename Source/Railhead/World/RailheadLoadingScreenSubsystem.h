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

	/** Watches for new worlds and starts ticking. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Stops both and takes any screen down. */
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

	/**
	 * A world of this game instance has initialised: the curtain goes up, and its experience
	 * subsystem's state changes are watched from here on.
	 */
	void HandleWorldInitialized(UWorld* World, const UWorld::InitializationValues Values);

	/** An activation starting opens the screen (if it is not up already). */
	UFUNCTION()
	void HandleStateChanged(EGantryExperienceState OldState, EGantryExperienceState NewState);

	/**
	 * Reads Gantry's snapshot, steps the feed, and lifts the screen once the feed is done and
	 * nothing holds it, or MaximumSeconds have passed.
	 * @return Always true: the ticker keeps running.
	 */
	bool Tick(float DeltaSeconds);

	/**
	 * Puts a new screen up, with a new feed, replacing any screen already up.
	 * @param World The world the screen covers.
	 * @param bBlocking The curtain (the world's first activation), or the panel alone.
	 */
	void Open(UWorld* World, bool bBlocking);

	/** Takes the screen down and drops its feed. */
	void Close();

	/** Whether there is nothing left to wait for. */
	bool IsActivationOver() const;

	/** What the screen shows, while it is up. */
	TSharedPtr<FRailheadLoadFeed> Feed;

	/** The screen, while it is up. */
	TSharedPtr<SRailheadLoadingScreen> Screen;

	/** The world the screen covers. */
	TWeakObjectPtr<UWorld> ScreenWorld;

	/** Whether the screen up is the curtain. */
	bool bScreenBlocking = false;

	/** When the screen went up, and when the snapshot was last read, in platform seconds. */
	double OpenedAt = 0.0;
	double CollectedAt = 0.0;

	/** The world has been shown: later activations in it get the panel, not the curtain. */
	TWeakObjectPtr<UWorld> ShownWorld;

	/** The listener on new worlds, and the ticker. */
	FDelegateHandle WorldInitHandle;
	FTSTicker::FDelegateHandle TickHandle;
};
