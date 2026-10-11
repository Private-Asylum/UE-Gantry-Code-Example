// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadMainMenu.h"

#include "UI/RailheadStyle.h"
#include "World/RailheadSessionSubsystem.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadMainMenu"

TSharedRef<SWidget> SRailheadMainMenu::MakeButton(const FText& Label, FReply (SRailheadMainMenu::*Handler)())
{
	using namespace RailheadStyle;
	return SNew(SButton)
		.ButtonStyle(&Button())
		.OnClicked(this, Handler)
		[
			SNew(STextBlock).Font(Font(18, true)).ColorAndOpacity(Paper).Text(Label)
		];
}

void SRailheadMainMenu::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	Context = InArgs._Context;
	OnSettings = InArgs._OnSettings;
	const bool bCanHost = URailheadSessionSubsystem::CanHost();

	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock).Font(Font(64, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Title", "RAILHEAD"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 36.0f)
		[
			SNew(STextBlock).Font(Font(15)).ColorAndOpacity(Dim).Text(LOCTEXT("Subtitle", "Keep the train running. A Gantry example."))
		];

	if (bCanHost)
	{
		Column->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			MakeButton(LOCTEXT("Solo", "Ride alone"), &SRailheadMainMenu::Solo)
		];
		Column->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				MakeButton(LOCTEXT("Host", "Host a crew"), &SRailheadMainMenu::Host)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(110.0f)
				[
					SAssignNew(PortBox, SEditableTextBox).Style(&TextBox()).Text(FText::FromString(TEXT("7777")))
					.ToolTipText(LOCTEXT("PortTip", "The port the crew connect to."))
				]
			]
		];
	}

	Column->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			MakeButton(LOCTEXT("Join", "Join a crew"), &SRailheadMainMenu::Join)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBox).WidthOverride(210.0f)
			[
				SAssignNew(AddressBox, SEditableTextBox).Style(&TextBox()).Text(FText::FromString(TEXT("127.0.0.1:7777")))
				.ToolTipText(LOCTEXT("AddressTip", "host, host:port or [IPv6]:port"))
			]
		]
	];
	Column->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
	[
		MakeButton(LOCTEXT("Settings", "Settings"), &SRailheadMainMenu::Settings)
	];
	Column->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
	[
		MakeButton(LOCTEXT("Quit", "Quit"), &SRailheadMainMenu::Quit)
	];
	Column->AddSlot().AutoHeight()
	[
		SNew(STextBlock).Font(Font(14)).ColorAndOpacity(Ember).AutoWrapText(true).Text_Lambda([this]() { return Status; })
	];

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(Fill())
			.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.012f, 0.72f))
			.Padding(FMargin(72.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(460.0f)
				[
					Column
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(72.0f, 0.0f, 0.0f, 28.0f))
		[
			SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Dim).Text(URailheadSessionSubsystem::DescribeBuild())
		]
	];
}

FReply SRailheadMainMenu::Solo()
{
	if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(Context.Get()))
	{
		Session->PlaySolo(Status);
	}
	return FReply::Handled();
}

FReply SRailheadMainMenu::Host()
{
	const FString Port = PortBox.IsValid() ? PortBox->GetText().ToString().TrimStartAndEnd() : FString();
	if (!Port.IsNumeric())
	{
		Status = LOCTEXT("PortNumber", "The port must be a whole number.");
		return FReply::Handled();
	}
	if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(Context.Get()))
	{
		Session->Host(FCString::Atoi(*Port), Status);
	}
	return FReply::Handled();
}

FReply SRailheadMainMenu::Join()
{
	if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(Context.Get()))
	{
		Session->Join(AddressBox->GetText().ToString(), Status);
	}
	return FReply::Handled();
}

FReply SRailheadMainMenu::Settings()
{
	OnSettings.ExecuteIfBound();
	return FReply::Handled();
}

FReply SRailheadMainMenu::Quit()
{
	if (URailheadSessionSubsystem* Session = URailheadSessionSubsystem::Get(Context.Get()))
	{
		Session->Quit();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
