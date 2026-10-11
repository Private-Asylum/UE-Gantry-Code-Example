// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/GantryExperienceStage.h"
#include "World/RailheadWeather.h"

#include "RailheadStage_Weather.generated.h"

// @doc
/**
 * Lays weather over the line while the experience runs, and takes it away when it ends: snow,
 * rain, haze, a darker sun. What a route or a mod brings to change the sky without code; the line
 * eases in and out of it (ARailheadLine::AddWeather).
 *
 * Client Only: weather is something to see, and a dedicated server sees nothing.
 */
UCLASS(meta = (DisplayName = "Weather", ShortTooltip = "Lays weather over the line while the experience runs."))
class RAILHEAD_API URailheadStage_Weather : public UGantryExperienceStage
{
	GENERATED_BODY()

public:

	URailheadStage_Weather();

	/** "Weather". */
	virtual FString GetStageDisplayName_Implementation() const override;

	/** Hands the weather to the line and finishes. */
	virtual void BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished) override;

	/** Takes it away again. */
	virtual void EndStage(const FGantryExperienceStageContext& Context) override;

	/** The weather. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	FRailheadWeather Weather;
};
