// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Player/RailheadPlayerController.h"

#include "RailheadMenuPlayerController.generated.h"

class ACameraActor;
class SRailheadMainMenu;

// @doc
/**
 * The main menu's controller: the menu on screen, and a camera alongside the locomotive, so the
 * route running behind it reads as a train in motion. Settings opens over the menu; Escape closes
 * it.
 */
UCLASS()
class RAILHEAD_API ARailheadMenuPlayerController : public ARailheadPlayerController
{
	GENERATED_BODY()

protected:

	/** Locally: spawns the camera beside the locomotive and puts the menu on screen. */
	virtual void BeginPlay() override;

	/** Takes the menu off the screen. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Always: the main menu is always up. */
	virtual bool IsMenuController() const override { return true; }

	/** Where the camera stands, relative to the train, in centimetres. */
	UPROPERTY(EditDefaultsOnly, Category = "Railhead|Menu")
	FVector CameraLocation = FVector(5200.0, -1500.0, 520.0);

	/** The point the camera looks at, relative to the train, in centimetres. */
	UPROPERTY(EditDefaultsOnly, Category = "Railhead|Menu")
	FVector CameraTarget = FVector(1200.0, 0.0, 260.0);

private:

	/** The main menu on screen. */
	TSharedPtr<SRailheadMainMenu> Menu;

	/** The camera beside the locomotive, spawned when play begins on the local controller. */
	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> TrackCamera;
};
