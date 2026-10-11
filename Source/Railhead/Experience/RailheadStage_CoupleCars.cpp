// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Experience/RailheadStage_CoupleCars.h"

#include "RailheadLogChannels.h"
#include "Train/RailheadCar.h"
#include "Train/RailheadTrain.h"

#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GantryAssetManager.h"
#include "Tags/GantryGaugeTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadStage_CoupleCars)

URailheadStage_CoupleCars::URailheadStage_CoupleCars()
{
	NetExecution = EGantryStageNetExecution::ServerOnly;
}

FString URailheadStage_CoupleCars::GetStageDisplayName_Implementation() const
{
	return TEXT("Couple cars");
}

void URailheadStage_CoupleCars::BeginStage(const FGantryExperienceStageContext& Context, FOnStageFinished OnFinished)
{
	StartStage(OnFinished);

	TArray<FSoftObjectPath> Paths;
	for (const TSoftClassPtr<ARailheadCar>& Car : Cars)
	{
		if (!Car.IsNull())
		{
			Paths.Add(Car.ToSoftObjectPath());
		}
	}
	if (Paths.IsEmpty())
	{
		FinishStage(true);
		return;
	}

	TWeakObjectPtr<URailheadStage_CoupleCars> WeakThis(this);
	TWeakObjectPtr<UWorld> WeakWorld(Context.World);
	LoadHandle = UGantryAssetManager::Get().RequestAsyncLoadTracked(
		Paths,
		TEXT("Cars"),
		GantryGaugeTags::Load_Scope_Content,
		FStreamableDelegate::CreateLambda([WeakThis, WeakWorld]()
		{
			if (URailheadStage_CoupleCars* Stage = WeakThis.Get(); Stage && Stage->IsRunning())
			{
				Stage->Couple(WeakWorld.Get());
				Stage->FinishStage(true);
			}
		}),
		Context.LoadScope,
		true,
		FStreamableDelegate::CreateLambda([WeakThis]()
		{
			if (URailheadStage_CoupleCars* Stage = WeakThis.Get())
			{
				Stage->FinishStage(false);
			}
		}));
}

void URailheadStage_CoupleCars::Couple(UWorld* World)
{
	ARailheadTrain* Train = ARailheadTrain::Find(World);
	if (!Train)
	{
		UE_LOG(LogRailhead, Warning, TEXT("Couple cars: there is no train to couple to."));
		return;
	}

	for (const TSoftClassPtr<ARailheadCar>& CarClass : Cars)
	{
		UClass* Class = CarClass.Get();
		if (!Class)
		{
			continue;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Owner = Train;
		if (ARailheadCar* Car = World->SpawnActor<ARailheadCar>(Class, Train->GetActorTransform(), Params))
		{
			Train->CoupleCar(Car);
			Coupled.Add(Car);
			UE_LOG(LogRailhead, Log, TEXT("Couple cars: %s coupled (%d car(s) behind the wagons)."), *Class->GetName(), Train->GetCoupledCars().Num());
		}
	}
}

void URailheadStage_CoupleCars::EndStage(const FGantryExperienceStageContext& Context)
{
	ARailheadTrain* Train = ARailheadTrain::Find(Context.World);
	for (const TWeakObjectPtr<ARailheadCar>& Weak : Coupled)
	{
		if (ARailheadCar* Car = Weak.Get())
		{
			if (Train)
			{
				Train->UncoupleCar(Car);
			}
			Car->Destroy();
		}
	}
	Coupled.Reset();

	if (LoadHandle.IsValid())
	{
		LoadHandle->ReleaseHandle();
		LoadHandle.Reset();
	}

	Super::EndStage(Context);
}
