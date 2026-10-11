// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Player/RailheadMenuPlayerController.h"

#include "UI/SRailheadMainMenu.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadMenuPlayerController)

void ARailheadMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// The train stands at the origin; a camera beside the locomotive, looking back along it.
	const FRotator Look = (CameraTarget - CameraLocation).Rotation();
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	TrackCamera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, Look, Params);
	if (TrackCamera)
	{
		TrackCamera->GetCameraComponent()->SetFieldOfView(62.0f);
		TrackCamera->GetCameraComponent()->bConstrainAspectRatio = false;
		SetViewTarget(TrackCamera);
	}

	TWeakObjectPtr<ARailheadMenuPlayerController> WeakThis(this);
	Menu = SNew(SRailheadMainMenu)
		.Context(this)
		.OnSettings_Lambda([WeakThis]() { if (WeakThis.IsValid()) { WeakThis->OpenSettings(); } });
	ShowWidget(Menu.ToSharedRef(), 30);
	UpdateInputMode();
}

void ARailheadMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideWidget(Menu);
	Menu.Reset();
	Super::EndPlay(EndPlayReason);
}
