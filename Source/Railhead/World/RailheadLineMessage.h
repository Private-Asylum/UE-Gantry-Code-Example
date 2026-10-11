// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"

#include "RailheadLineMessage.generated.h"

/**
 * What the line announces on Railhead.Message.Line.Tunnel and .Daylight, on every machine that
 * shows the line: which stretch the train is in, and which it is heading for. Anything can listen
 * (the HUD's title card does) without the line knowing who.
 */
USTRUCT(BlueprintType)
struct FRailheadLineMessage
{
	GENERATED_BODY()

	/** The stretch the train is in (for Daylight, the one it has just come out into). */
	UPROPERTY(BlueprintReadOnly, Category = "Railhead")
	FGameplayTag Stretch;

	/** The stretch the tunnel leads to (Tunnel only). */
	UPROPERTY(BlueprintReadOnly, Category = "Railhead")
	FGameplayTag NextStretch;
};
