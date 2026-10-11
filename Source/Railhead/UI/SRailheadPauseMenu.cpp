// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadPauseMenu.h"

#include "UI/RailheadStyle.h"
#include "World/RailheadSessionSubsystem.h"

#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadPauseMenu"

void SRailheadPauseMenu::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	Context = InArgs._Context;
	const FSimpleDelegate OnResume = InArgs._OnResume;
	const FSimpleDelegate OnSettings = InArgs._OnSettings;
	TWeakObjectPtr<UObject> WeakContext = Context;

	auto Button = [](const FText& Label, TFunction<void()> Action)
	{
		return SNew(SButton)
			.ButtonStyle(&RailheadStyle::Button())
			.OnClicked_Lambda([Action]() { Action(); return FReply::Handled(); })
			[
				SNew(STextBlock).Font(RailheadStyle::Font(18, true)).ColorAndOpacity(RailheadStyle::Paper).Text(Label)
			];
	};

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(Fill())
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(380.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(36, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Title", "ABOARD"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 24.0f)
				[
					SNew(STextBlock).Font(Font(14)).ColorAndOpacity(Dim).Text_Lambda([WeakContext]()
					{
						const UObject* Object = WeakContext.Get();
						return URailheadSessionSubsystem::DescribeWorld(Object ? Object->GetWorld() : nullptr);
					})
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					Button(LOCTEXT("Resume", "Back to work"), [OnResume]() { OnResume.ExecuteIfBound(); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					Button(LOCTEXT("Settings", "Settings"), [OnSettings]() { OnSettings.ExecuteIfBound(); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					Button(LOCTEXT("Leave", "Leave the train"), [WeakContext]()
					{
						if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(WeakContext.Get()))
						{
							Session->LeaveToMenu();
						}
					})
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					Button(LOCTEXT("Quit", "Quit"), [WeakContext]()
					{
						if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(WeakContext.Get()))
						{
							Session->Quit();
						}
					})
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
