// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Experience/RailheadStage_Weather.h"

#include "World/RailheadLine.h"

#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadStage_Weather)

URailheadStage_Weather::URailheadStage_Weather()
{
	NetExecution = EGantryStageNetExecution::ClientOnly;
}

FString URailheadStage_Weather::GetStageDisplayName_Implementation() const
{
	return TEXT("Weather");
}

void URailheadStage_Weather::BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished)
{
	StartStage(OnFinished);
	if (ARailheadLine* Line = ARailheadLine::Find(Context.World))
	{
		Line->AddWeather(this, Weather);
	}
	FinishStage(true);
}

void URailheadStage_Weather::EndStage(const FGantryExperienceStageContext& Context)
{
	if (ARailheadLine* Line = ARailheadLine::Find(Context.World))
	{
		Line->RemoveWeather(this);
	}
	Super::EndStage(Context);
}
