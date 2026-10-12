// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"

#include "RailheadLineMessage.generated.h"

// @doc
/**
 * What the line announces on Railhead.Message.Line.Tunnel and .Daylight, on every machine that
 * shows the line: which stretch the train is in, and which it is heading for. Anything can listen
 * (the HUD's title card does) without the line knowing who.
 */
USTRUCT(BlueprintType, meta = (GantryAPI, GantryAPIBlueprint))
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

// @doc
/**
 * The hook the line runs on Railhead.Hook.Line.Speed, on the server, every tick: the speed the
 * train is heading for, after the boiler and the tunnel have had their say. Listeners change
 * Target Speed (snow on the rails slows it, a downhill stretch speeds it up); the line eases the
 * train towards whatever they leave.
 */
USTRUCT(BlueprintType, meta = (GantryAPI, GantryAPIBlueprint))
struct FRailheadLineSpeed
{
	GENERATED_BODY()

	/** Metres per second the train heads for; change it. */
	UPROPERTY(BlueprintReadWrite, Category = "Railhead", meta = (UIMin = "0.0", UIMax = "60.0", Units = "MetersPerSecond"))
	float TargetSpeed = 0.0f;

	/** The stretch the train is in. */
	UPROPERTY(BlueprintReadOnly, Category = "Railhead")
	FGameplayTag Stretch;

	/** Whether the train is in a tunnel. */
	UPROPERTY(BlueprintReadOnly, Category = "Railhead")
	bool bInTunnel = false;
};
