// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "RailheadLineRules.generated.h"

// @doc
/**
 * How a route drives the train. Experiences carry it in their Data: EXP_Base has the defaults and a
 * route brings its own to override them. The line asks Gantry for the nearest (Find Experience
 * Data, leaf first) whenever an experience becomes ready, so a stretch, its route and the base can
 * each have a say without naming each other.
 */
UCLASS(BlueprintType, meta = (GantryAPI, GantryAPIBlueprint))
class URailheadLineRules : public UDataAsset
{
	GENERATED_BODY()

public:

	/** How quickly the train gains and loses speed, in metres per second squared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line", meta = (ClampMin = "0.1", Units = "MetersPerSecondSquared"))
	float Acceleration = 1.5f;

	/**
	 * In a tunnel whose far side is still loading, the train slows to this fraction of its speed at
	 * the slowest.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Line", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TunnelCrawlFraction = 0.2f;
};
