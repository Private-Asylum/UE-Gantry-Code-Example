// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Player/RailheadPlayerController.h"

#include "UI/SRailheadPauseMenu.h"
#include "UI/SRailheadSettings.h"

#include "Player/RailheadInputConfig.h"

#include "EnhancedInputComponent.h"
#include "Engine/GameViewportClient.h"
#include "Experience/GantryExperienceSubsystem.h"
#include "GantrySettingsRegistry.h"
#include "GantrySettingsSubsystem.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadPlayerController)

ARailheadPlayerController::ARailheadPlayerController()
{
}

void ARailheadPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}
	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		Experiences->CallOrRegister_OnExperienceReady(
			FGantryOnExperienceReadyNative::FDelegate::CreateUObject(this, &ARailheadPlayerController::HandleExperienceReady));
	}
	UpdateInputMode();
}

void ARailheadPlayerController::HandleExperienceReady(const UGantryExperienceDefinition* Experience)
{
	const UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
	const URailheadInputConfig* Config = Experiences ? Cast<URailheadInputConfig>(Experiences->FindExperienceData(URailheadInputConfig::StaticClass())) : nullptr;
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (bMenuBound || !Config || !Config->Menu || !Input)
	{
		return;
	}
	Input->BindAction(Config->Menu, ETriggerEvent::Started, this, &ARailheadPlayerController::ToggleMenu);
	bMenuBound = true;
}

void ARailheadPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideWidget(Settings);
	HideWidget(PauseMenu);
	Settings.Reset();
	PauseMenu.Reset();
	Super::EndPlay(EndPlayReason);
}

void ARailheadPlayerController::ShowWidget(const TSharedRef<SWidget>& Widget, int32 ZOrder)
{
	if (ULocalPlayer* Player = GetLocalPlayer(); Player && Player->ViewportClient)
	{
		Player->ViewportClient->AddViewportWidgetContent(Widget, ZOrder);
	}
}

void ARailheadPlayerController::HideWidget(const TSharedPtr<SWidget>& Widget)
{
	ULocalPlayer* Player = GetLocalPlayer();
	if (Widget.IsValid() && Player && Player->ViewportClient)
	{
		Player->ViewportClient->RemoveViewportWidgetContent(Widget.ToSharedRef());
	}
}

void ARailheadPlayerController::UpdateInputMode()
{
	if (IsMenuController() || PauseMenu.IsValid() || Settings.IsValid())
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		SetShowMouseCursor(true);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
}

void ARailheadPlayerController::ToggleMenu()
{
	if (Settings.IsValid())
	{
		// Escape on the settings screen is Revert and close: nothing applied by accident.
		if (UGantrySettingsRegistry* Registry = UGantrySettingsSubsystem::GetRegistry(GetLocalPlayer()))
		{
			Registry->RevertChanges();
		}
		HideWidget(Settings);
		Settings.Reset();
	}
	else if (PauseMenu.IsValid())
	{
		HideWidget(PauseMenu);
		PauseMenu.Reset();
	}
	else if (!IsMenuController())
	{
		TWeakObjectPtr<ARailheadPlayerController> WeakThis(this);
		PauseMenu = SNew(SRailheadPauseMenu)
			.Context(this)
			.OnResume_Lambda([WeakThis]() { if (WeakThis.IsValid()) { WeakThis->ToggleMenu(); } })
			.OnSettings_Lambda([WeakThis]() { if (WeakThis.IsValid()) { WeakThis->OpenSettings(); } });
		ShowWidget(PauseMenu.ToSharedRef(), 40);
	}
	UpdateInputMode();
}

void ARailheadPlayerController::OpenSettings()
{
	UGantrySettingsRegistry* Registry = UGantrySettingsSubsystem::GetRegistry(GetLocalPlayer());
	if (Settings.IsValid() || !Registry)
	{
		return;
	}
	TWeakObjectPtr<ARailheadPlayerController> WeakThis(this);
	Settings = SNew(SRailheadSettings)
		.Registry(Registry)
		.OnClosed_Lambda([WeakThis]()
		{
			if (ARailheadPlayerController* PC = WeakThis.Get())
			{
				PC->HideWidget(PC->Settings);
				PC->Settings.Reset();
				PC->UpdateInputMode();
			}
		});
	ShowWidget(Settings.ToSharedRef(), 60);
	UpdateInputMode();
}
