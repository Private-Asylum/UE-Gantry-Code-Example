// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "GameModes/RailheadMenuGameMode.h"

#include "Player/RailheadMenuPlayerController.h"
#include "Train/RailheadTrain.h"

#include "GameFramework/HUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadMenuGameMode)

ARailheadMenuGameMode::ARailheadMenuGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = ARailheadMenuPlayerController::StaticClass();
	HUDClass = AHUD::StaticClass();
	DefaultPawnClass = nullptr;
}

void ARailheadMenuGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ARailheadTrain* Train = ARailheadTrain::Find(GetWorld()); Train && Train->GetPressure() < 0.65f)
	{
		Train->Stoke();
	}
}
