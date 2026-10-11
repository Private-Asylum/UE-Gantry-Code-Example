// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadLoadingScreen.h"

#include "UI/RailheadLoadFeed.h"
#include "UI/RailheadStyle.h"

#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadLoadingScreen"

void SRailheadLoadingScreen::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	Feed = InArgs._Feed;

	// The feed: the last FeedLines lines shown, newest (and brightest) at the bottom.
	TSharedRef<SVerticalBox> Lines = SNew(SVerticalBox);
	for (int32 Slot = 0; Slot < FeedLines; ++Slot)
	{
		Lines->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Font(Font(13))
			.ColorAndOpacity(Slot == FeedLines - 1 ? Paper : Dim)
			.Text(this, &SRailheadLoadingScreen::GetFeedLine, Slot)
		];
	}

	ChildSlot
	[
		SNew(SOverlay)

		// The curtain: only for the first activation in a world.
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.Visibility(InArgs._bBlocking ? EVisibility::Visible : EVisibility::Collapsed)
			.BorderImage(Fill())
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.025f, 1.0f))
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(120.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(64, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Title", "RAILHEAD"))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.0f, 24.0f, 0.0f, 18.0f)
				[
					SNew(SCircularThrobber).Radius(28.0f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(18)).ColorAndOpacity(Paper).Text(this, &SRailheadLoadingScreen::GetCurtainStatus)
				]
			]
		]

		// The panel: top right, a fixed width so it does not jitter as lines change.
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 70.0f, 32.0f, 0.0f))
		[
			SNew(SBorder)
			.BorderImage(Fill())
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.82f))
			.Padding(FMargin(28.0f, 22.0f))
			[
				SNew(SBox)
				.WidthOverride(640.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Font(Font(26, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Loading", "LOADING"))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Dim).AutoWrapText(true).Text(this, &SRailheadLoadingScreen::GetSummary)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 10.0f)
					[
						SNew(SBox).HeightOverride(8.0f)
						[
							SNew(SProgressBar).Style(&Gauge()).FillColorAndOpacity(Ember).Percent(this, &SRailheadLoadingScreen::GetProgress)
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						Lines
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock).Font(Font(11)).ColorAndOpacity(Dim)
						.Text(LOCTEXT("Paced", "Paced for reading: the loads themselves take a few frames."))
					]
				]
			]
		]
	];
}

FText SRailheadLoadingScreen::GetFeedLine(int32 Slot) const
{
	// The window ends at the last line shown.
	const TArray<FRailheadLoadFeed::FLine>& All = Feed->GetLines();
	const int32 At = Feed->GetShown() - FeedLines + Slot;
	if (!All.IsValidIndex(At))
	{
		return FText::GetEmpty();
	}
	return FText::FromString(FString::ChrN(All[At].Depth * 3, TEXT(' ')) + All[At].Text);
}

FText SRailheadLoadingScreen::GetSummary() const
{
	const FRailheadLoadFeed::FCounts& Counts = Feed->GetCounts();
	return FText::Format(LOCTEXT("Summary", "Experiences {0}    Stages {1}    Features {2}    Assets {3}    Mods {4}    Paks {5}"),
		Counts.Experiences, Counts.Stages, Counts.Features, Counts.Assets, Counts.Mods, Counts.Paks);
}

FText SRailheadLoadingScreen::GetCurtainStatus() const
{
	return FText::FromString(Feed->GetNewestShown());
}

TOptional<float> SRailheadLoadingScreen::GetProgress() const
{
	return Feed->GetShownFraction();
}

#undef LOCTEXT_NAMESPACE
