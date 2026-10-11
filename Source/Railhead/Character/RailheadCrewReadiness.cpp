// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Character/RailheadCrewReadiness.h"

#include "Character/RailheadCharacter.h"
#include "RailheadLogChannels.h"
#include "Player/RailheadInputConfig.h"
#include "Train/RailheadTrain.h"

#include "Experience/GantryExperienceSubsystem.h"
#include "GantryStreamingSubsystem.h"
#include "Tags/GantryCoreTags.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadCrewReadiness)

URailheadCrewReadiness::URailheadCrewReadiness(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FeatureName = TEXT("Crew");
}

void URailheadCrewReadiness::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	if (UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get())
	{
		Streaming->AddLoadingHold(this);
	}
	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		TWeakObjectPtr<URailheadCrewReadiness> WeakThis(this);
		Experiences->CallOrRegister_OnExperienceReady(FGantryOnExperienceReadyNative::FDelegate::CreateWeakLambda(this,
			[WeakThis](const UGantryExperienceDefinition*)
			{
				if (URailheadCrewReadiness* Readiness = WeakThis.Get())
				{
					Readiness->CheckInitState();
				}
			}));
	}
	GetWorld()->GetTimerManager().SetTimer(RetryTimer, FTimerDelegate::CreateUObject(this, &URailheadCrewReadiness::Retry), 0.25f, true);
}

void URailheadCrewReadiness::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get())
	{
		Streaming->RemoveLoadingHold(this);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void URailheadCrewReadiness::Retry()
{
	CheckInitState();
	if (K2_HasReachedInitState(GantryCoreTags::InitState_GameplayReady))
	{
		GetWorld()->GetTimerManager().ClearTimer(RetryTimer);
	}
}

bool URailheadCrewReadiness::IsLocalCrew() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn && Pawn->IsLocallyControlled();
}

const URailheadInputConfig* URailheadCrewReadiness::FindInputConfig() const
{
	const UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
	return Experiences ? Cast<URailheadInputConfig>(Experiences->FindExperienceData(URailheadInputConfig::StaticClass())) : nullptr;
}

bool URailheadCrewReadiness::K2_CanChangeInitState_Implementation(FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return false;
	}
	if (DesiredState == GantryCoreTags::InitState_Spawned)
	{
		return true;
	}
	if (DesiredState == GantryCoreTags::InitState_DataAvailable)
	{
		// Every crew member needs its player state; this machine's own also needs its controls.
		return Pawn->GetPlayerState() && (!IsLocalCrew() || (Pawn->InputComponent && FindInputConfig()));
	}
	if (DesiredState == GantryCoreTags::InitState_DataInitialized)
	{
		return true;
	}
	if (DesiredState == GantryCoreTags::InitState_GameplayReady)
	{
		const UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
		return Experiences && Experiences->IsExperienceReady() && ARailheadTrain::Find(GetWorld());
	}
	return false;
}

void URailheadCrewReadiness::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == GantryCoreTags::InitState_DataInitialized && IsLocalCrew())
	{
		ARailheadCharacter* Crew = Cast<ARailheadCharacter>(GetOwner());
		if (const URailheadInputConfig* Config = FindInputConfig(); Crew && Config)
		{
			Crew->BindCrewInput(*Config);
			UE_LOG(LogRailhead, Log, TEXT("Crew: controls bound for %s."), *GetNameSafe(Crew->GetPlayerState()));
		}
	}
	if (DesiredState == GantryCoreTags::InitState_GameplayReady && IsLocalCrew())
	{
		UE_LOG(LogRailhead, Log, TEXT("Crew: ready to play."));
	}
	Super::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

bool URailheadCrewReadiness::ShouldHoldLoadingScreen_Implementation(FString& OutReason) const
{
	if (!IsLocalCrew() || K2_HasReachedInitState(GantryCoreTags::InitState_GameplayReady))
	{
		return false;
	}
	OutReason = TEXT("Crew getting ready");
	return true;
}
