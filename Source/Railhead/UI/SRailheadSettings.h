// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GantrySettingTypes.h"
#include "Widgets/SCompoundWidget.h"

class SScrollBox;
class SVerticalBox;
class UGantrySetting;
class UGantrySettingsPage;
class UGantrySettingsRegistry;

/**
 * Railhead's settings screen, drawn over whatever a player's Gantry settings registry holds.
 *
 * Nothing on it is specific to a setting: pages become tabs, and each setting becomes a row whose
 * control follows its kind (a slider for a number, arrows for a choice, a box for a toggle, a
 * button for an action). Pages that arrive with an experience (the Coast Line's own) appear while
 * it is active. Disabled settings say why; hidden ones are left out.
 *
 * Done applies and closes; Revert puts the page back; Reset sets the page to its defaults. A new
 * display mode or resolution asks to be kept, and reverts by itself if nobody answers.
 */
class SRailheadSettings : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadSettings) {}
		/** The player's registry. */
		SLATE_ARGUMENT(TWeakObjectPtr<UGantrySettingsRegistry>, Registry)
		/** Done was pressed (after applying). */
		SLATE_EVENT(FSimpleDelegate, OnClosed)
	SLATE_END_ARGS()

	/** Builds the tabs, the rows for the first page, and the buttons, and listens for changes. */
	void Construct(const FArguments& InArgs);

	/** Stops listening to the registry. */
	virtual ~SRailheadSettings() override;

	/** Rebuilds the tabs when pages arrive or leave (an experience starting or ending). */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	/** A dependency or a revert can change a setting's options: the rows are rebuilt. */
	void HandleSettingChanged(UGantrySetting* Setting, EGantrySettingChangeReason Reason);

	/** Rebuilds the tabs, and the rows when the page shown is gone. */
	void RebuildTabs();

	/** Rebuilds the rows for the page shown. */
	void RebuildRows();

	/** One row: the setting's name, its control, and why it is disabled when it is. */
	TSharedRef<SWidget> MakeRow(UGantrySetting* Setting);

	/** The control for a setting, by its kind. */
	TSharedRef<SWidget> MakeControl(UGantrySetting* Setting);

	/** Applies the changes and closes, unless a display change still waits to be kept. */
	FReply Done();

	/** Puts back every change not yet applied. */
	FReply Revert();

	/** Sets the page shown to its defaults. */
	FReply Reset();

	/** The player's registry. */
	TWeakObjectPtr<UGantrySettingsRegistry> Registry;

	/** The page whose rows are showing. */
	TWeakObjectPtr<const UGantrySettingsPage> Shown;

	/** Called by Done, after applying. */
	FSimpleDelegate OnClosed;

	/** The tab column, one button per page. */
	TSharedPtr<SVerticalBox> Tabs;

	/** The rows of the page shown. */
	TSharedPtr<SScrollBox> Rows;

	/** How many pages the tabs were built for. */
	int32 KnownPageCount = -1;

	/** The listener on the registry's setting changes. */
	FDelegateHandle PagesHandle;
};
