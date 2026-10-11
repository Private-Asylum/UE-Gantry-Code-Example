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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool IsMenuController() const override { return true; }

	/** Where the camera stands, relative to the train, in centimetres, and where it looks. */
	UPROPERTY(EditDefaultsOnly, Category = "Railhead|Menu")
	FVector CameraLocation = FVector(5200.0, -1500.0, 520.0);

	UPROPERTY(EditDefaultsOnly, Category = "Railhead|Menu")
	FVector CameraTarget = FVector(1200.0, 0.0, 260.0);

private:

	TSharedPtr<SRailheadMainMenu> Menu;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> TrackCamera;
};
