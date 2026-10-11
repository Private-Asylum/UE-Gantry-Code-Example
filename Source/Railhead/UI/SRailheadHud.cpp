// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadHud.h"

#include "Character/RailheadCharacter.h"
#include "Player/RailheadPlayerState.h"
#include "World/RailheadLine.h"
#include "Train/RailheadTrain.h"
#include "UI/RailheadStyle.h"

#include "Experience/GantryExperienceSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GantrySetting.h"
#include "GantrySettingsRegistry.h"
#include "GantrySettingsSubsystem.h"
#include "GantryStreamingSubsystem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadHud"

namespace RailheadHud
{
	/** Seconds a stretch's title stays up, and how long it takes to fade. */
	constexpr double TitleSeconds = 4.0;
	constexpr double TitleFadeSeconds = 1.0;
}

void SRailheadHud::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	Owner = InArgs._Owner;

	auto Panel = [](const TSharedRef<SWidget>& Content)
	{
		return SNew(SBorder)
			.BorderImage(Fill())
			.BorderBackgroundColor(Soot)
			.Padding(FMargin(16.0f, 10.0f))
			[
				Content
			];
	};

	ChildSlot
	[
		SNew(SOverlay)

		// Where the train is.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(32.0f)
		[
			Panel(SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(20, true)).ColorAndOpacity(Brass).Text(this, &SRailheadHud::GetStretchText)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock).Font(Font(14)).ColorAndOpacity(Paper).Text(this, &SRailheadHud::GetStatusText)
				])
		]

		// How it is running.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(32.0f)
		[
			Panel(SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(30, true)).ColorAndOpacity(Paper).Text(this, &SRailheadHud::GetSpeedText)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						SNew(STextBlock).Font(Font(13, true)).ColorAndOpacity(Dim).Text(LOCTEXT("Boiler", "BOILER"))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(220.0f).HeightOverride(14.0f)
						[
							SNew(SProgressBar)
							.Style(&Gauge())
							.Percent(this, &SRailheadHud::GetPressure)
							.FillColorAndOpacity(this, &SRailheadHud::GetPressureColor)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock).Font(Font(13)).ColorAndOpacity(Paper).Text(this, &SRailheadHud::GetPressureText)
					]
				])
		]

		// Performance readings (the Show Stats setting), then the notes column: whatever is in the
		// Railhead.HUD.Notes UI extension slot, put there by routes and mods the HUD never names.
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(32.0f)
		[
			SNew(SBox).WidthOverride(320.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox).Visibility(this, &SRailheadHud::GetStatsVisibility)
					[
						Panel(SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Paper).Text(this, &SRailheadHud::GetStatsText))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SAssignNew(NotesBox, SVerticalBox)
				]
			]
		]

		// The crew.
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(32.0f)
		[
			Panel(SNew(STextBlock).Font(Font(13)).ColorAndOpacity(Paper).Text(this, &SRailheadHud::GetCrewText))
		]

		// A dot to aim with, and what the crew member can do with what is under it.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(4.0f).HeightOverride(4.0f)
			[
				SNew(SImage).Image(Fill()).ColorAndOpacity(Paper)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.0f, 90.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock).Font(Font(16, true)).ColorAndOpacity(Paper).ShadowOffset(FVector2D(1.0, 1.0)).Text(this, &SRailheadHud::GetPromptText)
		]

		// A stretch's name as the train comes out into it.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.0f, 120.0f, 0.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(Font(18)).ColorAndOpacity(this, &SRailheadHud::GetTitleColor).ShadowOffset(FVector2D(2.0, 2.0))
				.ShadowColorAndOpacity(this, &SRailheadHud::GetTitleShadow)
				.Text_Lambda([this]() { return TitleRoute; })
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(Font(46, true)).ColorAndOpacity(this, &SRailheadHud::GetTitleColor).ShadowOffset(FVector2D(2.0, 2.0))
				.ShadowColorAndOpacity(this, &SRailheadHud::GetTitleShadow)
				.Text_Lambda([this]() { return TitleStretch; })
			]
		]
	];
}

void SRailheadHud::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	Now = InCurrentTime;
}

void SRailheadHud::ShowTitle(const FText& Route, const FText& Stretch)
{
	TitleRoute = Route;
	TitleStretch = Stretch;
	TitleShownAt = Now;
}

float SRailheadHud::GetTitleOpacity() const
{
	const double Age = Now - TitleShownAt;
	return static_cast<float>(FMath::Clamp((RailheadHud::TitleSeconds + RailheadHud::TitleFadeSeconds - Age) / RailheadHud::TitleFadeSeconds, 0.0, 1.0));
}

FSlateColor SRailheadHud::GetTitleColor() const
{
	return RailheadStyle::Brass.CopyWithNewOpacity(GetTitleOpacity());
}

FLinearColor SRailheadHud::GetTitleShadow() const
{
	// The shadow fades with the text; left alone it would stay behind, black.
	return FLinearColor(0.0f, 0.0f, 0.0f, 0.8f * GetTitleOpacity());
}

FText SRailheadHud::DescribeStretch(const FGameplayTag& Stretch, bool bWithRoute) const
{
	// Names do not change mid-session; ask Gantry's tree once per stretch, not several times a frame.
	TMap<FGameplayTag, FText>& Cache = bWithRoute ? RouteNames : StretchNames;
	if (const FText* Known = Cache.Find(Stretch))
	{
		return *Known;
	}
	const FText Name = LookUpStretch(Stretch, bWithRoute);
	if (Stretch.IsValid())
	{
		Cache.Add(Stretch, Name);
	}
	return Name;
}

FText SRailheadHud::LookUpStretch(const FGameplayTag& Stretch, bool bWithRoute) const
{
	const APlayerController* PC = Owner.Get();
	UGantryExperienceSubsystem* Experiences = PC ? UGantryExperienceSubsystem::Get(PC) : nullptr;
	FGantryExperienceInfo Info;
	if (!Experiences || !Stretch.IsValid() || !Experiences->GetExperienceInfo(Stretch, Info))
	{
		return FText::FromString(Stretch.ToString());
	}

	FGantryExperienceInfo Route;
	if (bWithRoute && !Info.Parents.IsEmpty() && Experiences->GetExperienceInfo(Info.Parents[0], Route))
	{
		return FText::Format(LOCTEXT("RouteAndStretch", "{0}  ·  {1}"), Route.DisplayName, Info.DisplayName);
	}
	return Info.DisplayName;
}

FText SRailheadHud::GetStretchText() const
{
	const APlayerController* PC = Owner.Get();
	const ARailheadLine* Line = PC ? ARailheadLine::Find(PC->GetWorld()) : nullptr;
	if (!Line || Line->GetState().Phase == ERailheadLinePhase::Waiting)
	{
		return LOCTEXT("NoStretch", "Railhead");
	}
	return DescribeStretch(Line->GetState().Stretch, true);
}

FText SRailheadHud::GetStatusText() const
{
	const APlayerController* PC = Owner.Get();
	const ARailheadLine* Line = PC ? ARailheadLine::Find(PC->GetWorld()) : nullptr;
	if (!Line)
	{
		return FText::GetEmpty();
	}

	const FRailheadLineState& State = Line->GetState();
	switch (State.Phase)
	{
	case ERailheadLinePhase::Waiting:
		return LOCTEXT("Waiting", "Getting up steam");

	case ERailheadLinePhase::Running:
		return FText::Format(LOCTEXT("TunnelAhead", "Tunnel in {0} m"), FText::AsNumber(FMath::RoundToInt(Line->GetMetersToTunnel())));

	case ERailheadLinePhase::Tunnel:
	{
		UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
		const int32 Percent = Streaming ? FMath::RoundToInt(Streaming->GetOverallProgress() * 100.0f) : 100;
		return FText::Format(LOCTEXT("InTunnel", "In the tunnel  ·  {0} ahead, {1}% loaded"), DescribeStretch(State.NextStretch, false), FText::AsNumber(Percent));
	}

	case ERailheadLinePhase::Leaving:
		return FText::Format(LOCTEXT("Leaving", "Daylight ahead  ·  {0}"), DescribeStretch(State.Stretch, false));
	}
	return FText::GetEmpty();
}

FText SRailheadHud::GetSpeedText() const
{
	const APlayerController* PC = Owner.Get();
	const ARailheadLine* Line = PC ? ARailheadLine::Find(PC->GetWorld()) : nullptr;
	const int32 Kmh = Line ? FMath::RoundToInt(Line->GetSpeed() * 3.6f) : 0;
	return FText::Format(LOCTEXT("Speed", "{0} km/h"), FText::AsNumber(Kmh));
}

TOptional<float> SRailheadHud::GetPressure() const
{
	const APlayerController* PC = Owner.Get();
	const ARailheadTrain* Train = PC ? ARailheadTrain::Find(PC->GetWorld()) : nullptr;
	return Train ? Train->GetPressure() : 0.0f;
}

FSlateColor SRailheadHud::GetPressureColor() const
{
	const float Pressure = GetPressure().Get(0.0f);
	if (Pressure < 0.25f)
	{
		return FLinearColor(0.75f, 0.12f, 0.06f);
	}
	return Pressure > 0.95f ? FLinearColor(0.95f, 0.95f, 0.95f) : RailheadStyle::Brass;
}

FText SRailheadHud::GetPressureText() const
{
	const float Pressure = GetPressure().Get(0.0f);
	if (Pressure < 0.25f)
	{
		return LOCTEXT("PressureLow", "Low! Coal to the firebox");
	}
	if (Pressure > 0.95f)
	{
		return LOCTEXT("PressureFull", "Blowing off");
	}
	return FText::Format(LOCTEXT("PressurePercent", "{0}%"), FText::AsNumber(FMath::RoundToInt(Pressure * 100.0f)));
}

FText SRailheadHud::GetPromptText() const
{
	const APlayerController* PC = Owner.Get();
	const ARailheadCharacter* Crew = PC ? Cast<ARailheadCharacter>(PC->GetPawn()) : nullptr;
	if (!Crew)
	{
		return FText::GetEmpty();
	}

	switch (Crew->GetOffer())
	{
	case ERailheadInteraction::TakeCoal:
		return LOCTEXT("TakeCoal", "[E]  Take a shovel of coal");
	case ERailheadInteraction::Stoke:
		return LOCTEXT("Stoke", "[E]  Stoke the firebox");
	default:
		return Crew->IsCarryingCoal() ? LOCTEXT("Carrying", "Coal in hand: the firebox is in the cab") : FText::GetEmpty();
	}
}

FText SRailheadHud::GetCrewText() const
{
	const APlayerController* PC = Owner.Get();
	const AGameStateBase* GameState = PC && PC->GetWorld() ? PC->GetWorld()->GetGameState() : nullptr;
	const ARailheadPlayerState* Mine = PC ? PC->GetPlayerState<ARailheadPlayerState>() : nullptr;
	const int32 Crew = GameState ? GameState->PlayerArray.Num() : 1;
	return FText::Format(LOCTEXT("Crew", "Crew {0}  ·  your shovels {1}"), FText::AsNumber(Crew), FText::AsNumber(Mine ? Mine->Shovels : 0));
}


void SRailheadHud::AddNote(int32 Id, const FText& Title, const FText& Body, int32 Priority)
{
	Notes.Add({ Id, Title, Body, Priority });
	Notes.StableSort([](const FNote& A, const FNote& B) { return A.Priority > B.Priority; });
	RebuildNotes();
}

void SRailheadHud::RemoveNote(int32 Id)
{
	Notes.RemoveAll([Id](const FNote& Note) { return Note.Id == Id; });
	RebuildNotes();
}

void SRailheadHud::RebuildNotes()
{
	using namespace RailheadStyle;
	if (!NotesBox.IsValid())
	{
		return;
	}
	NotesBox->ClearChildren();
	for (const FNote& Note : Notes)
	{
		NotesBox->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderImage(Fill())
			.BorderBackgroundColor(Soot)
			.Padding(FMargin(14.0f, 8.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(14, true)).ColorAndOpacity(Brass).Text(Note.Title)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Paper).AutoWrapText(true).Text(Note.Body)
				]
			]
		];
	}
}

EVisibility SRailheadHud::GetStatsVisibility() const
{
	const APlayerController* PC = Owner.Get();
	const UGantrySettingsRegistry* Registry = PC ? UGantrySettingsSubsystem::GetRegistry(PC->GetLocalPlayer()) : nullptr;
	const UGantrySettingToggle* ShowStats = Registry ? Cast<UGantrySettingToggle>(Registry->FindSetting(TEXT("Video.ShowStats"))) : nullptr;
	return ShowStats && ShowStats->IsOn() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FText SRailheadHud::GetStatsText() const
{
	const UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
	if (!Streaming)
	{
		return FText::GetEmpty();
	}
	const FGantryRuntimeReadings Readings = Streaming->GetRuntimeReadings();
	FNumberFormattingOptions OneDecimal;
	OneDecimal.MinimumFractionalDigits = 1;
	OneDecimal.MaximumFractionalDigits = 1;
	return FText::Format(LOCTEXT("Stats", "{0} fps  ·  frame {1} ms\ngame {2} ms  ·  render {3} ms  ·  GPU {4} ms\nloads open: {5}"),
		FText::AsNumber(FMath::RoundToInt(Readings.FramesPerSecond)), FText::AsNumber(Readings.FrameMilliseconds, &OneDecimal),
		FText::AsNumber(Readings.GameThreadMilliseconds, &OneDecimal), FText::AsNumber(Readings.RenderThreadMilliseconds, &OneDecimal),
		FText::AsNumber(Readings.GPUMilliseconds, &OneDecimal), FText::AsNumber(Readings.OpenLoadScopes));
}

#undef LOCTEXT_NAMESPACE
