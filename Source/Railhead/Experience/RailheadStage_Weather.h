// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/GantryExperienceStage.h"
#include "Messages/GantryMessageTypes.h"
#include "World/RailheadWeather.h"

#include "RailheadStage_Weather.generated.h"

// @doc
/**
 * Lays weather over the line while the experience runs, and takes it away when it ends: snow,
 * rain, haze, a darker sun. What a route or a mod brings to change the sky without code; the line
 * eases in and out of it (ARailheadLine::AddWeather).
 *
 * Weather can slow the train too (Train Speed Scale): on the server, the stage listens to the
 * line's speed hook (Railhead.Hook.Line.Speed) while it runs. The sky is laid on every machine
 * that draws, which a dedicated server does not.
 */
UCLASS(meta = (GantryAPI, DisplayName = "Weather", ShortTooltip = "Lays weather over the line while the experience runs."))
class RAILHEAD_API URailheadStage_Weather : public UGantryExperienceStage
{
	GENERATED_BODY()

public:

	URailheadStage_Weather();

	/** "Weather". */
	virtual FString GetStageDisplayName_Implementation() const override;

	/**
	 * Lays the weather on the line where the line is drawn, listens to the speed hook on the server
	 * (when Train Speed Scale changes anything), and finishes.
	 */
	virtual void BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished) override;

	/** Takes the weather away again and stops listening to the speed hook. */
	virtual void EndStage(const FGantryExperienceStageContext& Context) override;

	/** The weather. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage", meta = (GantryAPI))
	FRailheadWeather Weather;

	/** What the weather does to the train's speed: 1 leaves it, 0.8 is a fifth slower (snow on the rails). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage", meta = (GantryAPI, ClampMin = "0.1", ClampMax = "2.0", UIMin = "0.1", UIMax = "2.0"))
	float TrainSpeedScale = 1.0f;

private:

	/** The server's listener on the line's speed hook, while the stage runs. */
	FGantryMessageListenerHandle SpeedListener;
};
