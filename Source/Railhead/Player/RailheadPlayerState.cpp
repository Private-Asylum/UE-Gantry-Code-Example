// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Player/RailheadPlayerState.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadPlayerState)

void ARailheadPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARailheadPlayerState, CrewColor);
	DOREPLIFETIME(ARailheadPlayerState, Shovels);
}
