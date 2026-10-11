// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "World/RailheadSceneryRegistry.h"

#include "RailheadLogChannels.h"
#include "Experience/RailheadStretchDefinition.h"

#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadSceneryRegistry)

URailheadSceneryRegistry* URailheadSceneryRegistry::Get(const UWorld* World)
{
	return World ? World->GetSubsystem<URailheadSceneryRegistry>() : nullptr;
}

bool URailheadSceneryRegistry::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void URailheadSceneryRegistry::AddStretch(const URailheadStretchDefinition* Stretch)
{
	if (Stretch)
	{
		Stretches.Add(Stretch->ExperienceTag, Stretch);
		++Version;
		UE_LOG(LogRailhead, Log, TEXT("Scenery: %s is ready (%d layers)."), *Stretch->GetDisplayName().ToString(), Stretch->Scenery.Num());
	}
}

const URailheadStretchDefinition* URailheadSceneryRegistry::FindStretch(const FGameplayTag& Tag) const
{
	const TObjectPtr<const URailheadStretchDefinition>* Found = Stretches.Find(Tag);
	return Found ? Found->Get() : nullptr;
}

void URailheadSceneryRegistry::Prune(const FGameplayTagContainer& Keep)
{
	for (auto It = Stretches.CreateIterator(); It; ++It)
	{
		if (!Keep.HasTagExact(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}
