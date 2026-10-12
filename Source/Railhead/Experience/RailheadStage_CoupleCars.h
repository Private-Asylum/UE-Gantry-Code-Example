// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/GantryExperienceStage.h"

#include "RailheadStage_CoupleCars.generated.h"

class ARailheadCar;
struct FStreamableHandle;

// @doc
/**
 * Couples cars to the train while the experience runs, and takes them off when it ends: the
 * caboose on the base experience, a mod's van on the mod's own. Server Only: the server spawns the
 * cars and the train couples them; they replicate, attached, to every client.
 *
 * The car classes load as a tracked load under the stage's scope.
 */
UCLASS(meta = (GantryAPI, GantryAPIBlueprint, DisplayName = "Couple Cars", ShortTooltip = "Couples cars to the train while the experience runs."))
class RAILHEAD_API URailheadStage_CoupleCars : public UGantryExperienceStage
{
	GENERATED_BODY()

public:

	URailheadStage_CoupleCars();

	/** "Couple cars". */
	virtual FString GetStageDisplayName_Implementation() const override;

	/** Loads the car classes, spawns one of each and couples it, then finishes. */
	virtual void BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished) override;

	/** Uncouples and removes the cars it coupled. */
	virtual void EndStage(const FGantryExperienceStageContext& Context) override;

	/** The cars, front to back (tail cars go last whatever their place here). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<TSoftClassPtr<ARailheadCar>> Cars;

private:

	/** Spawns and couples the loaded cars. */
	void Couple(UWorld* World);

	/** The cars it coupled. */
	TArray<TWeakObjectPtr<ARailheadCar>> Coupled;

	/** Keeps the car classes in memory while the stage is up. */
	TSharedPtr<FStreamableHandle> LoadHandle;
};
