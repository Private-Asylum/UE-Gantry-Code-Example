// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/RailheadHUD.h"

#include "World/RailheadLine.h"
#include "UI/SRailheadHud.h"
#include "UI/SRailheadMismatch.h"
#include "UI/RailheadHudNote.h"
#include "World/RailheadLineMessage.h"
#include "RailheadLogChannels.h"
#include "RailheadTags.h"
#include "Messages/GantryMessageSubsystem.h"
#include "UIExtension/GantryUIExtensionSubsystem.h"

#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Experience/GantryExperienceDefinition.h"
#include "Experience/GantryExperienceSubsystem.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadHUD)

ARailheadHUD::ARailheadHUD()
{
}

void ARailheadHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	ULocalPlayer* Player = PC ? PC->GetLocalPlayer() : nullptr;
	if (!Player || !Player->ViewportClient)
	{
		return;
	}

	Widget = SNew(SRailheadHud).Owner(PC);
	Widget->SetVisibility(EVisibility::Collapsed);
	Player->ViewportClient->AddViewportWidgetContent(Widget.ToSharedRef(), 10);

	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		Experiences->CallOrRegister_OnExperienceReady_LowPriority(
			FGantryOnExperienceReadyNative::FDelegate::CreateUObject(this, &ARailheadHUD::HandleFirstReady));
		if (GetNetMode() == NM_Client)
		{
			Experiences->OnExperienceFailed.AddDynamic(this, &ARailheadHUD::HandleExperienceFailed);
		}
	}

	// Each stretch's title, when the line says the train has come out into it.
	if (UGantryMessageSubsystem* Messages = UGantryMessageSubsystem::Get(this))
	{
		TWeakObjectPtr<ARailheadHUD> WeakThis(this);
		DaylightListener = Messages->RegisterListener<FRailheadLineMessage>(RailheadTags::Message_Line_Daylight,
			[WeakThis](FGameplayTag, const FRailheadLineMessage& Message)
			{
				if (ARailheadHUD* HUD = WeakThis.Get())
				{
					HUD->HandleDaylight(Message.Stretch);
				}
			}, EGantryTagMatch::Exact, this);
	}

	// This player's notes: whatever is in the slot for them.
	if (UGantryUIExtensionSubsystem* Slots = UGantryUIExtensionSubsystem::Get(this))
	{
		TWeakObjectPtr<ARailheadHUD> WeakThis(this);
		NotesSlot = Slots->RegisterSlot(RailheadTags::HUD_Notes, Player, EGantryTagMatch::Exact,
			[WeakThis](EGantryUIExtensionAction Action, const FGantryUIExtensionEntry& Entry)
			{
				if (ARailheadHUD* HUD = WeakThis.Get())
				{
					HUD->HandleNote(Action, Entry);
				}
			});
	}
}

void ARailheadHUD::HandleExperienceFailed(FGameplayTag ExperienceTag, const FString& Reason)
{
	APlayerController* PC = GetOwningPlayerController();
	ULocalPlayer* Player = PC ? PC->GetLocalPlayer() : nullptr;
	if (Mismatch.IsValid() || !Player || !Player->ViewportClient)
	{
		return;
	}

	UE_LOG(LogRailhead, Warning, TEXT("HUD: can't ride %s here: %s"), *ExperienceTag.ToString(), *Reason);
	Mismatch = SNew(SRailheadMismatch).Context(this).Reason(FText::FromString(Reason));
	Player->ViewportClient->AddViewportWidgetContent(Mismatch.ToSharedRef(), 40);
	PC->SetShowMouseCursor(true);
	PC->SetInputMode(FInputModeUIOnly());
}

void ARailheadHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		Experiences->OnExperienceFailed.RemoveDynamic(this, &ARailheadHUD::HandleExperienceFailed);
	}
	DaylightListener.Unregister();
	NotesSlot.Unregister();

	const APlayerController* PC = GetOwningPlayerController();
	const ULocalPlayer* Player = PC ? PC->GetLocalPlayer() : nullptr;
	if (Widget.IsValid() && Player && Player->ViewportClient)
	{
		Player->ViewportClient->RemoveViewportWidgetContent(Widget.ToSharedRef());
	}
	Widget.Reset();
	if (Mismatch.IsValid() && Player && Player->ViewportClient)
	{
		Player->ViewportClient->RemoveViewportWidgetContent(Mismatch.ToSharedRef());
	}
	Mismatch.Reset();

	Super::EndPlay(EndPlayReason);
}

void ARailheadHUD::HandleFirstReady(const UGantryExperienceDefinition* Experience)
{
	if (Widget.IsValid())
	{
		Widget->SetVisibility(EVisibility::HitTestInvisible);
	}
}

void ARailheadHUD::HandleDaylight(FGameplayTag Stretch)
{
	UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
	FGantryExperienceInfo Info;
	FGantryExperienceInfo Route;
	if (Widget.IsValid() && Experiences && Experiences->GetExperienceInfo(Stretch, Info))
	{
		if (!Info.Parents.IsEmpty())
		{
			Experiences->GetExperienceInfo(Info.Parents[0], Route);
		}
		Widget->ShowTitle(Route.DisplayName, Info.DisplayName);
		UE_LOG(LogRailhead, Log, TEXT("HUD: into daylight at %s."), *Info.DisplayName.ToString());
	}
}

void ARailheadHUD::HandleNote(EGantryUIExtensionAction Action, const FGantryUIExtensionEntry& Entry)
{
	const URailheadHudNote* Note = Cast<URailheadHudNote>(Entry.ContentObject);
	if (!Widget.IsValid() || !Note)
	{
		return;
	}
	if (Action == EGantryUIExtensionAction::Added)
	{
		Widget->AddNote(Entry.Id, Note->Title, Note->Body, Entry.Priority);
	}
	else
	{
		Widget->RemoveNote(Entry.Id);
	}
}
