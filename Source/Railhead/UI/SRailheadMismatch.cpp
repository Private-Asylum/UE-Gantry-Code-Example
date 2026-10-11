// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadMismatch.h"

#include "UI/RailheadStyle.h"
#include "World/RailheadSessionSubsystem.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadMismatch"

void SRailheadMismatch::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	TWeakObjectPtr<UObject> WeakContext = InArgs._Context;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(Fill())
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(640.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(32, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Title", "CAN'T RIDE THIS STRETCH"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock).Font(Font(16)).ColorAndOpacity(Paper).AutoWrapText(true)
					.Text(LOCTEXT("Explain", "The train's next stretch needs content this machine does not have, or has differently. Install the same mods as the host."))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 24.0f)
				[
					SNew(STextBlock).Font(Font(13)).ColorAndOpacity(Dim).AutoWrapText(true).Text(InArgs._Reason)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SButton)
					.ButtonStyle(&Button())
					.OnClicked_Lambda([WeakContext]()
					{
						if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(WeakContext.Get()))
						{
							Session->LeaveToMenu();
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock).Font(Font(18, true)).ColorAndOpacity(Paper).Text(LOCTEXT("Leave", "Leave the train"))
					]
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
