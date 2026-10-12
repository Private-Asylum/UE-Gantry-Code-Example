// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "GameModes/RailheadGameMode.h"

#include "Character/RailheadCharacter.h"
#include "Player/RailheadPlayerController.h"
#include "Player/RailheadPlayerState.h"
#include "UI/RailheadHUD.h"
#include "World/RailheadLine.h"
#include "RailheadLogChannels.h"
#include "Train/RailheadTrain.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadGameMode)

namespace RailheadGameMode
{
	/** Colours of overalls, handed out in turn. */
	const FLinearColor CrewColors[] =
	{
		FLinearColor(0.08f, 0.2f, 0.55f),
		FLinearColor(0.6f, 0.25f, 0.05f),
		FLinearColor(0.12f, 0.4f, 0.12f),
		FLinearColor(0.5f, 0.08f, 0.3f),
		FLinearColor(0.55f, 0.5f, 0.1f),
		FLinearColor(0.1f, 0.4f, 0.45f),
		FLinearColor(0.35f, 0.35f, 0.38f),
		FLinearColor(0.5f, 0.1f, 0.08f)
	};
}

ARailheadGameMode::ARailheadGameMode()
{
	DefaultPawnClass = ARailheadCharacter::StaticClass();
	PlayerControllerClass = ARailheadPlayerController::StaticClass();
	PlayerStateClass = ARailheadPlayerState::StaticClass();
	HUDClass = ARailheadHUD::StaticClass();
	GameStateClass = AGameStateBase::StaticClass();
	TrainClass = ARailheadTrain::StaticClass();
	LineClass = ARailheadLine::StaticClass();
}

void ARailheadGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Both stand at the origin for the whole session; the line moves the world past the train.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;
	if (!ARailheadTrain::Find(GetWorld()) && TrainClass)
	{
		GetWorld()->SpawnActor<ARailheadTrain>(TrainClass, FTransform::Identity, Params);
	}
	if (!ARailheadLine::Find(GetWorld()) && LineClass)
	{
		GetWorld()->SpawnActor<ARailheadLine>(LineClass, FTransform::Identity, Params);
	}
}

void ARailheadGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (ARailheadPlayerState* Crew = NewPlayer ? NewPlayer->GetPlayerState<ARailheadPlayerState>() : nullptr)
	{
		Crew->CrewColor = RailheadGameMode::CrewColors[Joined % UE_ARRAY_COUNT(RailheadGameMode::CrewColors)];
	}
	++Joined;

	Super::PostLogin(NewPlayer);
}

void ARailheadGameMode::RestartPlayer(AController* NewPlayer)
{
	const ARailheadTrain* Train = ARailheadTrain::Find(GetWorld());
	if (!Train || !NewPlayer)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	RestartPlayerAtTransform(NewPlayer, Train->GetCrewSpot(Joined - 1));
	UE_LOG(LogRailhead, Log, TEXT("Crew: %s is aboard."), *GetNameSafe(NewPlayer->PlayerState));
}
