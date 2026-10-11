// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/SRailheadSettings.h"

#include "UI/RailheadStyle.h"

#include "GantrySetting.h"
#include "GantrySettingKeyBinding.h"
#include "GantrySettingsPage.h"
#include "GantrySettingsRegistry.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SInputKeySelector.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RailheadSettings"

namespace RailheadSettings
{
	TSharedRef<SWidget> TextButton(const FText& Label, FOnClicked OnClicked, int32 Size = 15)
	{
		return SNew(SButton)
			.ButtonStyle(&RailheadStyle::Button())
			.OnClicked(OnClicked)
			[
				SNew(STextBlock).Font(RailheadStyle::Font(Size, true)).ColorAndOpacity(RailheadStyle::Paper).Text(Label)
			];
	}
}

void SRailheadSettings::Construct(const FArguments& InArgs)
{
	using namespace RailheadStyle;

	Registry = InArgs._Registry;
	OnClosed = InArgs._OnClosed;
	TWeakObjectPtr<UGantrySettingsRegistry> WeakRegistry = Registry;

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(Fill())
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(980.0f).HeightOverride(640.0f)
				[
					SNew(SBorder)
					.BorderImage(Fill())
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.025f, 0.95f))
					.Padding(28.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f)
						[
							SNew(STextBlock).Font(Font(30, true)).ColorAndOpacity(Brass).Text(LOCTEXT("Title", "SETTINGS"))
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 24.0f, 0.0f)
							[
								SNew(SBox).WidthOverride(200.0f)
								[
									SAssignNew(Tabs, SVerticalBox)
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.0f)
							[
								SAssignNew(Rows, SScrollBox)
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 18.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								RailheadSettings::TextButton(LOCTEXT("Reset", "Reset page"), FOnClicked::CreateSP(this, &SRailheadSettings::Reset))
							]
							+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(STextBlock).Font(Font(13)).ColorAndOpacity(Dim).Text_Lambda([WeakRegistry]()
								{
									const UGantrySettingsRegistry* Settings = WeakRegistry.Get();
									return Settings && Settings->HasUnappliedChanges() ? LOCTEXT("Unapplied", "Changes not applied yet") : FText::GetEmpty();
								})
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 10.0f, 0.0f)
							[
								RailheadSettings::TextButton(LOCTEXT("Revert", "Revert"), FOnClicked::CreateSP(this, &SRailheadSettings::Revert))
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								RailheadSettings::TextButton(LOCTEXT("Done", "Done"), FOnClicked::CreateSP(this, &SRailheadSettings::Done))
							]
						]
					]
				]
			]
		]

		// Keep the new display? Up only while the registry waits for an answer.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Visibility_Lambda([WeakRegistry]()
			{
				const UGantrySettingsRegistry* Settings = WeakRegistry.Get();
				return Settings && Settings->IsAwaitingVideoConfirm() ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.BorderImage(Fill())
			.BorderBackgroundColor(FLinearColor(0.05f, 0.04f, 0.02f, 0.98f))
			.Padding(32.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 16.0f)
				[
					SNew(STextBlock).Font(Font(18, true)).ColorAndOpacity(Paper).Text_Lambda([WeakRegistry]()
					{
						const UGantrySettingsRegistry* Settings = WeakRegistry.Get();
						const int32 Seconds = Settings ? FMath::CeilToInt(Settings->GetVideoConfirmSecondsLeft()) : 0;
						return FText::Format(LOCTEXT("KeepDisplay", "Keep this display? Going back in {0} s."), Seconds);
					})
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						RailheadSettings::TextButton(LOCTEXT("Keep", "Keep"), FOnClicked::CreateLambda([WeakRegistry]()
						{
							if (UGantrySettingsRegistry* Settings = WeakRegistry.Get())
							{
								Settings->ConfirmVideoChanges();
							}
							return FReply::Handled();
						}))
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						RailheadSettings::TextButton(LOCTEXT("GoBack", "Go back"), FOnClicked::CreateLambda([WeakRegistry]()
						{
							if (UGantrySettingsRegistry* Settings = WeakRegistry.Get())
							{
								Settings->RevertVideoChanges();
							}
							return FReply::Handled();
						}))
					]
				]
			]
		]
	];

	// Pages come and go with experiences: keep the tabs in step.
	if (UGantrySettingsRegistry* Settings = Registry.Get())
	{
		PagesHandle = Settings->OnSettingChangedNative.AddSP(this, &SRailheadSettings::HandleSettingChanged);
	}
	RebuildTabs();
}

SRailheadSettings::~SRailheadSettings()
{
	if (UGantrySettingsRegistry* Settings = Registry.Get())
	{
		Settings->OnSettingChangedNative.Remove(PagesHandle);
	}
}

void SRailheadSettings::HandleSettingChanged(UGantrySetting* Setting, EGantrySettingChangeReason Reason)
{
	// A dependency can change which options a setting offers (resolutions follow the window mode).
	if (Reason == EGantrySettingChangeReason::Dependency || Reason == EGantrySettingChangeReason::Revert)
	{
		RebuildRows();
	}
}

void SRailheadSettings::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const UGantrySettingsRegistry* Settings = Registry.Get();
	const int32 PageCount = Settings ? Settings->GetPages().Num() : 0;
	if (PageCount != KnownPageCount)
	{
		KnownPageCount = PageCount;
		RebuildTabs();
	}
}

void SRailheadSettings::RebuildTabs()
{
	using namespace RailheadStyle;

	Tabs->ClearChildren();
	UGantrySettingsRegistry* Settings = Registry.Get();
	if (!Settings)
	{
		return;
	}

	const TArray<UGantrySettingsPage*> Pages = Settings->GetPages();
	if (!Shown.IsValid() || !Pages.Contains(Shown.Get()))
	{
		Shown = Pages.IsEmpty() ? nullptr : Pages[0];
	}
	for (UGantrySettingsPage* Page : Pages)
	{
		TWeakObjectPtr<const UGantrySettingsPage> WeakPage(Page);
		Tabs->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SButton)
			.ButtonStyle(&Button())
			.OnClicked_Lambda([this, WeakPage]()
			{
				Shown = WeakPage;
				RebuildRows();
				return FReply::Handled();
			})
			[
				SNew(STextBlock).Font(Font(15, true))
				.ColorAndOpacity_Lambda([this, WeakPage]() { return FSlateColor(Shown == WeakPage ? Brass : Paper); })
				.Text(Page->Title)
			]
		];
	}
	RebuildRows();
}

void SRailheadSettings::RebuildRows()
{
	using namespace RailheadStyle;

	Rows->ClearChildren();
	const UGantrySettingsRegistry* Settings = Registry.Get();
	const UGantrySettingsPage* Page = Shown.Get();
	if (!Settings || !Page)
	{
		return;
	}

	if (!Page->Description.IsEmpty())
	{
		Rows->AddSlot().Padding(0.0f, 0.0f, 0.0f, 14.0f)
		[
			SNew(STextBlock).Font(Font(13)).ColorAndOpacity(Dim).AutoWrapText(true).Text(Page->Description)
		];
	}
	for (UGantrySetting* Setting : Settings->GetSettings(Page))
	{
		if (Setting && Setting->GetEditState().bVisible)
		{
			Rows->AddSlot().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeRow(Setting)
			];
		}
	}
}

TSharedRef<SWidget> SRailheadSettings::MakeRow(UGantrySetting* Setting)
{
	using namespace RailheadStyle;

	TWeakObjectPtr<UGantrySetting> WeakSetting(Setting);
	auto Enabled = [WeakSetting]()
	{
		const UGantrySetting* Each = WeakSetting.Get();
		return Each && Each->GetEditState().bEnabled;
	};

	return SNew(SBorder)
		.BorderImage(Fill())
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.03f))
		.Padding(FMargin(14.0f, 8.0f))
		.ToolTipText(Setting->Description)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Font(Font(15)).Text(Setting->DisplayName)
					.ColorAndOpacity_Lambda([Enabled]() { return FSlateColor(Enabled() ? Paper : Dim); })
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(320.0f).IsEnabled_Lambda(Enabled)
					[
						MakeControl(Setting)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Ember)
				.Visibility_Lambda([Enabled]() { return Enabled() ? EVisibility::Collapsed : EVisibility::Visible; })
				.Text_Lambda([WeakSetting]()
				{
					const UGantrySetting* Each = WeakSetting.Get();
					const TArray<FText> Reasons = Each ? Each->GetEditState().DisabledReasons : TArray<FText>();
					return Reasons.IsEmpty() ? FText::GetEmpty() : FText::Join(FText::FromString(TEXT("  ")), Reasons);
				})
			]
		];
}

TSharedRef<SWidget> SRailheadSettings::MakeControl(UGantrySetting* Setting)
{
	using namespace RailheadStyle;

	// A toggle: a box.
	if (UGantrySettingToggle* Toggle = Cast<UGantrySettingToggle>(Setting))
	{
		TWeakObjectPtr<UGantrySettingToggle> Weak(Toggle);
		return SNew(SCheckBox)
			.IsChecked_Lambda([Weak]() { return Weak.IsValid() && Weak->IsOn() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([Weak](ECheckBoxState State) { if (Weak.IsValid()) { Weak->SetOn(State == ECheckBoxState::Checked); } });
	}

	// A choice: arrows either side of the chosen option.
	if (UGantrySettingChoice* Choice = Cast<UGantrySettingChoice>(Setting))
	{
		TWeakObjectPtr<UGantrySettingChoice> Weak(Choice);
		auto Step = [Weak](int32 Direction)
		{
			if (UGantrySettingChoice* Each = Weak.Get())
			{
				const int32 Count = Each->GetOptions().Num();
				if (Count > 0)
				{
					const int32 Current = FMath::Max(0, Each->GetSelectedIndex());
					Each->SetSelectedIndex((Current + Direction + Count) % Count);
				}
			}
			return FReply::Handled();
		};
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				RailheadSettings::TextButton(FText::FromString(TEXT("<")), FOnClicked::CreateLambda([Step]() { return Step(-1); }), 14)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Font(Font(15, true)).ColorAndOpacity(Brass)
				.Text_Lambda([Weak]() { return Weak.IsValid() ? Weak->GetSelectedText() : FText::GetEmpty(); })
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				RailheadSettings::TextButton(FText::FromString(TEXT(">")), FOnClicked::CreateLambda([Step]() { return Step(1); }), 14)
			];
	}

	// A number: a slider and its value.
	if (UGantrySettingNumber* Number = Cast<UGantrySettingNumber>(Setting))
	{
		TWeakObjectPtr<UGantrySettingNumber> Weak(Number);
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(SSlider)
				.Value_Lambda([Weak]() { return Weak.IsValid() ? static_cast<float>(Weak->GetNormalized()) : 0.0f; })
				.StepSize(static_cast<float>(Number->GetNormalizedStep()))
				.SliderBarColor(Dim)
				.SliderHandleColor(Brass)
				.OnValueChanged_Lambda([Weak](float Value) { if (Weak.IsValid()) { Weak->SetNormalized(Value); } })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(64.0f)
				[
					SNew(STextBlock).Font(Font(15, true)).ColorAndOpacity(Brass)
					.Text_Lambda([Weak]() { return Weak.IsValid() ? Weak->GetFormattedValue() : FText::GetEmpty(); })
				]
			];
	}

	// A key: click it, then press the new key; any other mapping already on that key is named.
	if (UGantrySettingKeyBinding* Binding = Cast<UGantrySettingKeyBinding>(Setting))
	{
		TWeakObjectPtr<UGantrySettingKeyBinding> Weak(Binding);
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(180.0f)
				[
					SNew(SInputKeySelector)
					.Font(Font(15, true))
					.SelectedKey_Lambda([Weak]() { return FInputChord(Weak.IsValid() ? Weak->GetKey() : FKey()); })
					.OnKeySelected_Lambda([Weak](const FInputChord& Chord) { if (Weak.IsValid()) { Weak->SetKey(Chord.Key); } })
					.AllowModifierKeys(false)
					.EscapeCancelsSelection(true)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(12.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock).Font(Font(12)).ColorAndOpacity(Dim)
				.Text_Lambda([Weak]()
				{
					const TArray<FText> Conflicts = Weak.IsValid() ? Weak->GetConflicts() : TArray<FText>();
					return Conflicts.IsEmpty() ? FText::GetEmpty() : FText::Format(LOCTEXT("AlsoOn", "Also: {0}"), FText::Join(FText::FromString(TEXT(", ")), Conflicts));
				})
			];
	}

	// An action: a button.
	if (UGantrySettingAction* Action = Cast<UGantrySettingAction>(Setting))
	{
		TWeakObjectPtr<UGantrySettingAction> Weak(Action);
		return RailheadSettings::TextButton(Action->ActionText, FOnClicked::CreateLambda([Weak]()
		{
			if (Weak.IsValid())
			{
				Weak->Execute();
			}
			return FReply::Handled();
		}), 14);
	}

	return SNullWidget::NullWidget;
}

FReply SRailheadSettings::Done()
{
	if (UGantrySettingsRegistry* Settings = Registry.Get())
	{
		Settings->ApplyChanges();
		// A display change still waiting for an answer keeps the screen up.
		if (Settings->IsAwaitingVideoConfirm())
		{
			return FReply::Handled();
		}
	}
	OnClosed.ExecuteIfBound();
	return FReply::Handled();
}

FReply SRailheadSettings::Revert()
{
	if (UGantrySettingsRegistry* Settings = Registry.Get())
	{
		Settings->RevertChanges();
	}
	return FReply::Handled();
}

FReply SRailheadSettings::Reset()
{
	if (UGantrySettingsRegistry* Settings = Registry.Get())
	{
		Settings->ResetToDefaults(Shown.Get());
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
