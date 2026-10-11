// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Experience/RailheadStage_Scenery.h"

#include "RailheadTags.h"
#include "World/RailheadSceneryRegistry.h"
#include "Experience/RailheadStretchDefinition.h"

#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GantryAssetManager.h"
#include "Tags/GantryGaugeTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadStage_Scenery)

URailheadStage_Scenery::URailheadStage_Scenery()
{
	NetExecution = EGantryStageNetExecution::ClientOnly;
}

FString URailheadStage_Scenery::GetStageDisplayName_Implementation() const
{
	return TEXT("Scenery");
}

FGameplayTag URailheadStage_Scenery::GetStageTag_Implementation() const
{
	return RailheadTags::Stage_Scenery;
}

void URailheadStage_Scenery::BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished)
{
	StartStage(OnFinished);

	// The route's own activation has no stretch to dress: nothing to do.
	const URailheadStretchDefinition* Stretch = Cast<URailheadStretchDefinition>(Context.Experience);
	if (!Stretch)
	{
		FinishStage(true);
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Stretch->GetSceneryAssets(Paths);

	TWeakObjectPtr<URailheadStage_Scenery> WeakThis(this);
	TWeakObjectPtr<UWorld> WeakWorld(Context.World);
	TWeakObjectPtr<const URailheadStretchDefinition> WeakStretch(Stretch);

	auto HandOver = [WeakThis, WeakWorld, WeakStretch]()
	{
		URailheadStage_Scenery* Stage = WeakThis.Get();
		if (!Stage || !Stage->IsRunning())
		{
			return;
		}

		if (URailheadSceneryRegistry* Registry = URailheadSceneryRegistry::Get(WeakWorld.Get()))
		{
			Registry->AddStretch(WeakStretch.Get());
		}
		Stage->FinishStage(true);
	};

	// Named for what it holds; it sits under the stage's scope, which already says "Scenery".
	LoadHandle = UGantryAssetManager::Get().RequestAsyncLoadTracked(
		Paths,
		Stretch->GetDisplayName().ToString(),
		GantryGaugeTags::Load_Scope_Content,
		FStreamableDelegate::CreateLambda(HandOver),
		Context.LoadScope,
		true,
		FStreamableDelegate::CreateLambda([WeakThis]()
		{
			if (URailheadStage_Scenery* Stage = WeakThis.Get())
			{
				Stage->FinishStage(false);
			}
		}));
}

void URailheadStage_Scenery::EndStage(const FGantryExperienceStageContext& Context)
{
	if (LoadHandle.IsValid())
	{
		LoadHandle->ReleaseHandle();
		LoadHandle.Reset();
	}

	Super::EndStage(Context);
}
