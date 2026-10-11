// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/GantryExperienceStage.h"

#include "RailheadStage_Scenery.generated.h"

struct FStreamableHandle;

// @doc
/**
 * Loads the scenery of the stretch being activated and registers it for the line to draw.
 *
 * Lives on the route experience, not on each stretch: Gantry runs every experience's stages in the
 * activation chain (base, route, stretch), and this stage reads the stretch it is activating for
 * from the context. One stage serves every stretch on the route.
 *
 * Client Only: a dedicated server shows nobody anything, so it never loads a mesh. The load is a
 * tracked one under the stage's own scope, which is what the tunnel's speed and the loading screen
 * read.
 */
UCLASS(meta = (DisplayName = "Railhead Scenery", ShortTooltip = "Loads the stretch's scenery meshes, tracked, for the line to draw."))
class RAILHEAD_API URailheadStage_Scenery : public UGantryExperienceStage
{
	GENERATED_BODY()

public:

	URailheadStage_Scenery();

	/** "Scenery". */
	virtual FString GetStageDisplayName_Implementation() const override;

	/** Railhead.Stage.Scenery. */
	virtual FGameplayTag GetStageTag_Implementation() const override;

	/** Loads the stretch's scenery assets, then registers the stretch (URailheadSceneryRegistry) and finishes. */
	virtual void BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished) override;

	/** Lets go of the loaded assets; the line keeps what it still shows. */
	virtual void EndStage(const FGantryExperienceStageContext& Context) override;

private:

	/** Keeps the scenery in memory while the stage is up. */
	TSharedPtr<FStreamableHandle> LoadHandle;
};
