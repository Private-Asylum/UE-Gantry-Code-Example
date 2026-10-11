// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "World/RailheadLoadingScreenSubsystem.h"

#include "RailheadLogChannels.h"
#include "UI/RailheadLoadFeed.h"
#include "UI/SRailheadLoadingScreen.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Experience/GantryExperienceSubsystem.h"
#include "GantryStreamingSubsystem.h"
#include "HAL/PlatformTime.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadLoadingScreenSubsystem)

CSV_DECLARE_CATEGORY_EXTERN(Railhead);

void URailheadLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &URailheadLoadingScreenSubsystem::HandleWorldInitialized);
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &URailheadLoadingScreenSubsystem::Tick));
}

void URailheadLoadingScreenSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitHandle);
	FTSTicker::RemoveTicker(TickHandle);
	Close();

	Super::Deinitialize();
}

void URailheadLoadingScreenSubsystem::HandleWorldInitialized(UWorld* World, const UWorld::InitializationValues Values)
{
	// Only this game instance's own game worlds, and only where there is a screen to cover.
	if (!World || !World->IsGameWorld() || World->GetGameInstance() != GetGameInstance() || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// The curtain goes up now, before anything of the new world can be seen.
	Open(World, true);

	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(World))
	{
		Experiences->OnExperienceStateChanged.AddUniqueDynamic(this, &URailheadLoadingScreenSubsystem::HandleStateChanged);
	}
}

void URailheadLoadingScreenSubsystem::HandleStateChanged(EGantryExperienceState OldState, EGantryExperienceState NewState)
{
	if (Screen.IsValid() || (NewState != EGantryExperienceState::Resolving && NewState != EGantryExperienceState::Loading))
	{
		return;
	}

	UWorld* World = GetGameInstance()->GetWorld();
	Open(World, ShownWorld.Get() != World);
}

void URailheadLoadingScreenSubsystem::Open(UWorld* World, bool bBlocking)
{
	UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient();
	if (!Viewport || !World)
	{
		return;
	}

	Close();
	CSV_EVENT(Railhead, TEXT("LoadingScreenOpen"));
	Feed = MakeShared<FRailheadLoadFeed>();
	Feed->MinimumSeconds = MinimumSeconds;
	OpenedAt = FPlatformTime::Seconds();
	Feed->Begin(OpenedAt);
	Screen = SNew(SRailheadLoadingScreen).Feed(Feed).bBlocking(bBlocking);
	ScreenWorld = World;
	bScreenBlocking = bBlocking;
	Viewport->AddViewportWidgetContent(Screen.ToSharedRef(), 1000);
}

void URailheadLoadingScreenSubsystem::Close()
{
	if (Screen.IsValid())
	{
		if (UGameViewportClient* Viewport = GetGameInstance() ? GetGameInstance()->GetGameViewportClient() : nullptr)
		{
			Viewport->RemoveViewportWidgetContent(Screen.ToSharedRef());
		}
	}
	Screen.Reset();
	Feed.Reset();
	ScreenWorld.Reset();
}

bool URailheadLoadingScreenSubsystem::IsActivationOver() const
{
	const UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(ScreenWorld.Get());
	if (!Experiences)
	{
		return true;
	}

	switch (Experiences->GetExperienceState())
	{
	case EGantryExperienceState::Ready:
	case EGantryExperienceState::Failed:
		return true;

	case EGantryExperienceState::None:
	case EGantryExperienceState::Deactivating:
		// The curtain waits for the world's first experience; it may not have been chosen yet
		// (a client waits for the server's decision).
		return !bScreenBlocking;

	default:
		return false;
	}
}

bool URailheadLoadingScreenSubsystem::Tick(float DeltaSeconds)
{
	if (!Screen.IsValid())
	{
		return true;
	}

	CSV_SCOPED_TIMING_STAT(Railhead, LoadingFeed);
	const double Now = FPlatformTime::Seconds();
	if (Now - CollectedAt >= CollectSeconds)
	{
		CollectedAt = Now;
		Feed->Collect();
	}
	Feed->Step(Now);

	const bool bOver = IsActivationOver();
	const double Shown = Now - OpenedAt;

	// Gantry's answer as well as the feed's: anything holding the screen (the crew getting ready)
	// keeps it up after the experience is done.
	UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
	const TArray<FString> Reasons = bOver && Streaming ? Streaming->GetLoadingScreenReasons() : TArray<FString>();
	const bool bHeld = !Reasons.IsEmpty();
	if (!ScreenWorld.IsValid() || (Feed->IsDone(bOver, Now) && !bHeld) || Shown >= MaximumSeconds)
	{
		UE_LOG(LogRailhead, Log, TEXT("Loading screen: %s lifted after %.1f s, %d lines%s%s."), bScreenBlocking ? TEXT("curtain") : TEXT("panel"),
			Shown, Feed->GetLines().Num(), bOver ? TEXT("") : TEXT(", without a finished activation"),
			bHeld ? *FString::Printf(TEXT(", still held (%s)"), *FString::Join(Reasons, TEXT("; "))) : TEXT(""));
		CSV_EVENT(Railhead, TEXT("LoadingScreenClose"));
		if (bScreenBlocking)
		{
			ShownWorld = ScreenWorld;
		}
		Close();
	}
	return true;
}
