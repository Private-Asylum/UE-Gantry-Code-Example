// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GantryLoadingHold.h"
#include "Readiness/GantryInitStateComponent.h"

#include "RailheadCrewReadiness.generated.h"

class URailheadInputConfig;

/**
 * Gets a crew member ready in Gantry's init-state steps (the four InitState tags), each when it can:
 *
 *  - Spawned:          as it begins play.
 *  - DataAvailable:    it has a player state and, for the local player, the controls the
 *                      experience brings (URailheadInputConfig in its Data) have loaded.
 *  - DataInitialized:  the controls are bound.
 *  - GameplayReady:    the experience is ready and the train is here to stand on.
 *
 * Until the local player's crew member is ready it holds the loading screen (a Gantry loading
 * hold), so the screen never lifts on a crew member who cannot move yet. It tries again when the
 * experience becomes ready, when the player state arrives, and a few times a second until done.
 */
UCLASS()
class URailheadCrewReadiness : public UGantryInitStateComponent, public IGantryLoadingHold
{
	GENERATED_BODY()

public:

	URailheadCrewReadiness(const FObjectInitializer& ObjectInitializer);

	/* IGantryLoadingHold. */
	virtual bool ShouldHoldLoadingScreen_Implementation(FString& OutReason) const override;
	/* IGantryLoadingHold. */

	/* IGameFrameworkInitStateInterface. */
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	/* IGameFrameworkInitStateInterface. */

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool K2_CanChangeInitState_Implementation(FGameplayTag CurrentState, FGameplayTag DesiredState) const override;

private:

	/** The controls the active experiences bring, once loaded. */
	const URailheadInputConfig* FindInputConfig() const;

	/** Whether the owning pawn is this machine's player. */
	bool IsLocalCrew() const;

	/** Tries to move on, and stops retrying once ready. */
	void Retry();

	FTimerHandle RetryTimer;
};
