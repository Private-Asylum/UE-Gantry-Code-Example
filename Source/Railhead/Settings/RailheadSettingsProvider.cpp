// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Settings/RailheadSettingsProvider.h"

#include "GantrySetting.h"
#include "GantrySettingKeyBinding.h"
#include "GantrySettingsPage.h"
#include "GantrySettingsRegistry.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadSettingsProvider)

#define LOCTEXT_NAMESPACE "RailheadSettings"

namespace RailheadSettings
{
	UGameUserSettings* UserSettings()
	{
		return GEngine ? GEngine->GetGameUserSettings() : nullptr;
	}

	/** Reads one of the user settings as text, or nothing when there are none (a server). */
	template <typename FunctionType>
	FString Read(FunctionType&& Function)
	{
		const UGameUserSettings* Settings = UserSettings();
		return Settings ? Function(*Settings) : FString();
	}

	/** Writes one of the user settings, when there are any. */
	template <typename FunctionType>
	void Write(FunctionType&& Function)
	{
		if (UGameUserSettings* Settings = UserSettings())
		{
			Function(*Settings);
		}
	}

	/** A setting that takes effect when the user settings are applied. */
	void AppliesUserSettings(UGantrySetting* Setting, bool bChangesDisplay = false)
	{
		Setting->bAppliesUserSettings = true;
		Setting->bNeedsVideoConfirm = bChangesDisplay;
	}

	FString ResolutionValue(const FIntPoint& Resolution)
	{
		return FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
	}

	bool ParseResolution(const FString& Value, FIntPoint& Out)
	{
		FString X;
		FString Y;
		if (!Value.Split(TEXT("x"), &X, &Y) || !X.IsNumeric() || !Y.IsNumeric())
		{
			return false;
		}
		Out = FIntPoint(FCString::Atoi(*X), FCString::Atoi(*Y));
		return Out.X > 0 && Out.Y > 0;
	}

	/** Fullscreen offers what the display supports; a window, sizes that fit on the desktop. The current size always. */
	TArray<FGantrySettingOption> ResolutionOptions()
	{
		const UGameUserSettings* Settings = UserSettings();
		TArray<FIntPoint> Resolutions;
		if (Settings && Settings->GetFullscreenMode() == EWindowMode::Fullscreen)
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
		}
		else
		{
			UKismetSystemLibrary::GetConvenientWindowedResolutions(Resolutions);
		}
		if (Settings)
		{
			Resolutions.AddUnique(Settings->GetScreenResolution());
		}
		Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X < B.X : A.Y < B.Y; });

		TArray<FGantrySettingOption> Options;
		for (const FIntPoint& Resolution : Resolutions)
		{
			Options.Add({ ResolutionValue(Resolution), FText::Format(LOCTEXT("ResolutionOption", "{0} x {1}"),
				FText::AsNumber(Resolution.X, &FNumberFormattingOptions::DefaultNoGrouping()),
				FText::AsNumber(Resolution.Y, &FNumberFormattingOptions::DefaultNoGrouping())) });
		}
		return Options;
	}
}

TArray<UGantrySettingsPage*> URailheadSettingsProvider::CreatePages_Implementation(UGantrySettingsRegistry* Registry)
{
	// The display belongs to the machine, not to a player: a second player on the same screen has no Video page.
	const ULocalPlayer* Player = Registry ? Registry->GetLocalPlayer() : nullptr;
	TArray<UGantrySettingsPage*> Pages;
	if (!Player || Player->IsPrimaryPlayer())
	{
		Pages.Add(CreateVideoPage(Registry));
	}
	Pages.Add(CreateCrewPage(Registry));
	return Pages;
}

UGantrySettingsPage* URailheadSettingsProvider::CreateVideoPage(UGantrySettingsRegistry* Registry)
{
	using namespace RailheadSettings;
	UGantrySettingsPage* Page = NewPage(Registry, LOCTEXT("Video", "Video"), 0);
	Page->Description = LOCTEXT("VideoDescription", "The display and how much the game draws. Shared by everyone on this machine.");

	UGantrySettingChoice* WindowMode = Page->AddSetting<UGantrySettingChoice>(TEXT("Video.WindowMode"), LOCTEXT("WindowMode", "Window Mode"));
	WindowMode->Description = LOCTEXT("WindowModeDescription", "Fullscreen, a borderless window the size of the desktop, or a window.");
	WindowMode->Options = {
		{ LexToString(static_cast<int32>(EWindowMode::Fullscreen)), LOCTEXT("Fullscreen", "Fullscreen") },
		{ LexToString(static_cast<int32>(EWindowMode::WindowedFullscreen)), LOCTEXT("WindowedFullscreen", "Windowed Fullscreen") },
		{ LexToString(static_cast<int32>(EWindowMode::Windowed)), LOCTEXT("Windowed", "Windowed") }
	};
	WindowMode->BindValue(
		[] { return Read([](const UGameUserSettings& Settings) { return LexToString(static_cast<int32>(Settings.GetFullscreenMode())); }); },
		[](const FString& Value) { Write([&Value](UGameUserSettings& Settings) { Settings.SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(FCString::Atoi(*Value))); }); });
	AppliesUserSettings(WindowMode, true);

	// Its options depend on the window mode, so it is told when that changes.
	UGantrySettingChoice* Resolution = Page->AddSetting<UGantrySettingChoice>(TEXT("Video.Resolution"), LOCTEXT("Resolution", "Resolution"));
	Resolution->Description = LOCTEXT("ResolutionDescription", "The size of the picture, in pixels.");
	Resolution->DependsOn.Add(WindowMode->Id);
	Resolution->BindOptions(&ResolutionOptions);
	Resolution->BindValue(
		[] { return Read([](const UGameUserSettings& Settings) { return ResolutionValue(Settings.GetScreenResolution()); }); },
		[](const FString& Value)
		{
			FIntPoint Size;
			if (ParseResolution(Value, Size))
			{
				Write([&Size](UGameUserSettings& Settings) { Settings.SetScreenResolution(Size); });
			}
		});
	Resolution->BindEditState([](const UGantrySetting&, FGantrySettingEditState& State)
	{
		const UGameUserSettings* Settings = UserSettings();
		if (Settings && Settings->GetFullscreenMode() == EWindowMode::WindowedFullscreen)
		{
			State.Disable(LOCTEXT("DesktopResolution", "Windowed fullscreen always uses the desktop's resolution."));
		}
	});
	AppliesUserSettings(Resolution, true);

	// -1 when the quality groups differ: none of the options, which a choice shows as the raw value.
	UGantrySettingChoice* Quality = Page->AddSetting<UGantrySettingChoice>(TEXT("Video.OverallQuality"), LOCTEXT("OverallQuality", "Graphics Quality"));
	Quality->Description = LOCTEXT("OverallQualityDescription", "Sets every graphics quality group at once.");
	Quality->Options = {
		{ TEXT("0"), LOCTEXT("Low", "Low") },
		{ TEXT("1"), LOCTEXT("Medium", "Medium") },
		{ TEXT("2"), LOCTEXT("High", "High") },
		{ TEXT("3"), LOCTEXT("Epic", "Epic") }
	};
	Quality->DefaultValue = TEXT("3");
	Quality->BindValue(
		[] { return Read([](const UGameUserSettings& Settings) { return LexToString(Settings.GetOverallScalabilityLevel()); }); },
		[](const FString& Value) { Write([&Value](UGameUserSettings& Settings) { Settings.SetOverallScalabilityLevel(FCString::Atoi(*Value)); }); });
	AppliesUserSettings(Quality);

	UGantrySettingNumber* Scale = Page->AddSetting<UGantrySettingNumber>(TEXT("Video.ResolutionScale"), LOCTEXT("ResolutionScale", "Resolution Scale"));
	Scale->Description = LOCTEXT("ResolutionScaleDescription", "Draws the world at a fraction of the screen's resolution: softer, and faster.");
	Scale->Minimum = 50.0;
	Scale->Maximum = 100.0;
	Scale->Step = 5.0;
	Scale->bHasDefault = true;
	Scale->Default = 100.0;
	Scale->Format = EGantrySettingNumberFormat::Percent;
	Scale->BindValue(
		[]
		{
			return Read([](const UGameUserSettings& Settings)
			{
				float Normalized = 0.0f;
				float Value = 0.0f;
				float Min = 0.0f;
				float Max = 0.0f;
				Settings.GetResolutionScaleInformationEx(Normalized, Value, Min, Max);
				return LexToString(FMath::RoundToInt(Value));
			});
		},
		[](const FString& Value) { Write([&Value](UGameUserSettings& Settings) { Settings.SetResolutionScaleValueEx(FCString::Atof(*Value)); }); });
	AppliesUserSettings(Scale);

	UGantrySettingChoice* FrameRate = Page->AddSetting<UGantrySettingChoice>(TEXT("Video.FrameRateLimit"), LOCTEXT("FrameRateLimit", "Frame Rate Limit"));
	FrameRate->Description = LOCTEXT("FrameRateLimitDescription", "The most frames a second the game draws.");
	FrameRate->Options = {
		{ TEXT("30"), LOCTEXT("Fps30", "30") },
		{ TEXT("60"), LOCTEXT("Fps60", "60") },
		{ TEXT("120"), LOCTEXT("Fps120", "120") },
		{ TEXT("144"), LOCTEXT("Fps144", "144") },
		{ TEXT("0"), LOCTEXT("Unlimited", "Unlimited") }
	};
	FrameRate->DefaultValue = TEXT("0");
	FrameRate->BindValue(
		[] { return Read([](const UGameUserSettings& Settings) { return LexToString(FMath::RoundToInt(Settings.GetFrameRateLimit())); }); },
		[](const FString& Value) { Write([&Value](UGameUserSettings& Settings) { Settings.SetFrameRateLimit(FCString::Atof(*Value)); }); });
	AppliesUserSettings(FrameRate);

	UGantrySettingToggle* VSync = Page->AddSetting<UGantrySettingToggle>(TEXT("Video.VSync"), LOCTEXT("VSync", "Vertical Sync"));
	VSync->Description = LOCTEXT("VSyncDescription", "Waits for the display before showing each frame: no tearing, a little more latency.");
	VSync->DefaultValue = TEXT("False");
	VSync->BindValue(
		[] { return Read([](const UGameUserSettings& Settings) { return FString(Settings.IsVSyncEnabled() ? TEXT("True") : TEXT("False")); }); },
		[](const FString& Value) { Write([&Value](UGameUserSettings& Settings) { Settings.SetVSyncEnabled(Value.ToBool()); }); });
	AppliesUserSettings(VSync);

	// Nothing bound: kept in the save. The HUD shows Gantry's runtime readings while it is on.
	UGantrySettingToggle* Stats = Page->AddSetting<UGantrySettingToggle>(TEXT("Video.ShowStats"), LOCTEXT("ShowStats", "Show Performance"));
	Stats->Description = LOCTEXT("ShowStatsDescription", "Frame rate and frame times in a corner of the screen.");
	Stats->DefaultValue = TEXT("False");

	return Page;
}

UGantrySettingsPage* URailheadSettingsProvider::CreateCrewPage(UGantrySettingsRegistry* Registry)
{
	// Nothing bound: these are the game's alone, so Gantry keeps them in the player's save. The
	// character reads them from the registry when it looks around.
	UGantrySettingsPage* Page = NewPage(Registry, LOCTEXT("Crew", "Crew"), 10);
	Page->Description = LOCTEXT("CrewDescription", "How you see and look around. Each player has their own.");

	UGantrySettingNumber* FieldOfView = Page->AddSetting<UGantrySettingNumber>(TEXT("Crew.FieldOfView"), LOCTEXT("FieldOfView", "Field of View"));
	FieldOfView->Description = LOCTEXT("FieldOfViewDescription", "How wide you see, in degrees.");
	FieldOfView->Minimum = 70.0;
	FieldOfView->Maximum = 110.0;
	FieldOfView->Step = 1.0;
	FieldOfView->bHasDefault = true;
	FieldOfView->Default = 95.0;
	FieldOfView->Format = EGantrySettingNumberFormat::Integer;

	UGantrySettingNumber* Sensitivity = Page->AddSetting<UGantrySettingNumber>(TEXT("Crew.LookSensitivity"), LOCTEXT("LookSensitivity", "Look Sensitivity"));
	Sensitivity->Description = LOCTEXT("LookSensitivityDescription", "How far the view turns for a move of the mouse or stick.");
	Sensitivity->Minimum = 0.2;
	Sensitivity->Maximum = 3.0;
	Sensitivity->Step = 0.05;
	Sensitivity->bHasDefault = true;
	Sensitivity->Default = 1.0;
	Sensitivity->Format = EGantrySettingNumberFormat::Number;
	Sensitivity->Decimals = 2;

	UGantrySettingToggle* InvertLook = Page->AddSetting<UGantrySettingToggle>(TEXT("Crew.InvertLook"), LOCTEXT("InvertLook", "Invert Look"));
	InvertLook->Description = LOCTEXT("InvertLookDescription", "Pushing forward looks down.");
	InvertLook->DefaultValue = TEXT("False");

	return Page;
}

TArray<UGantrySettingsPage*> URailheadControlsProvider::CreatePages_Implementation(UGantrySettingsRegistry* Registry)
{
	UGantrySettingsPage* Page = NewPage(Registry, LOCTEXT("Controls", "Controls"), 5);
	Page->Description = LOCTEXT("ControlsDescription", "The keys the crew uses. Each player has their own.");
	UGantrySettingKeyBinding::AddKeyBindings(Page);
	return { Page };
}

#undef LOCTEXT_NAMESPACE
