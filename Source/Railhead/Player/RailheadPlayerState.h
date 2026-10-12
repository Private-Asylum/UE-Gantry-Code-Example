// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"

#include "RailheadPlayerState.generated.h"

/** A crew member's lasting details: the colour of their overalls, chosen by the game mode on joining. */
UCLASS()
class RAILHEAD_API ARailheadPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Colour of their overalls, so the crew can tell each other apart. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Railhead|Crew")
	FLinearColor CrewColor = FLinearColor(0.2f, 0.3f, 0.6f);

	/** Shovels of coal this crew member has put in the firebox. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Railhead|Crew")
	int32 Shovels = 0;
};
