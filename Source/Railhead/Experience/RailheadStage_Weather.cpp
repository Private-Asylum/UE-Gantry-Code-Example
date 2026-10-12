// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Experience/RailheadStage_Weather.h"

#include "World/RailheadLine.h"
#include "World/RailheadLineMessage.h"
#include "RailheadTags.h"

#include "Engine/World.h"
#include "Messages/GantryMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadStage_Weather)

URailheadStage_Weather::URailheadStage_Weather()
{
	// Everywhere: the server slows the train, the machines that draw lay the sky.
	NetExecution = EGantryStageNetExecution::Everywhere;
}

FString URailheadStage_Weather::GetStageDisplayName_Implementation() const
{
	return TEXT("Weather");
}

void URailheadStage_Weather::BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished)
{
	StartStage(OnFinished);
	UWorld* World = Context.World;
	if (ARailheadLine* Line = ARailheadLine::Find(World); Line && World->GetNetMode() != NM_DedicatedServer)
	{
		Line->AddWeather(this, Weather);
	}

	// Only the server drives the train; it runs the hook, so only it listens.
	UGantryMessageSubsystem* Messages = UGantryMessageSubsystem::Get(World);
	if (Messages && World->GetNetMode() != NM_Client && !FMath::IsNearlyEqual(TrainSpeedScale, 1.0f))
	{
		const float Scale = TrainSpeedScale;
		SpeedListener = Messages->RegisterHookListener<FRailheadLineSpeed>(RailheadTags::Hook_Line_Speed,
			[Scale](FGantryMessageRouter::FHookCall&, FRailheadLineSpeed& Speed) { Speed.TargetSpeed *= Scale; },
			/*Priority=*/ 0, EGantryTagMatch::Exact, this);
	}
	FinishStage(true);
}

void URailheadStage_Weather::EndStage(const FGantryExperienceStageContext& Context)
{
	SpeedListener.Unregister();
	if (ARailheadLine* Line = ARailheadLine::Find(Context.World))
	{
		Line->RemoveWeather(this);
	}
	Super::EndStage(Context);
}
