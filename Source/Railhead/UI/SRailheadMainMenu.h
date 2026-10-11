// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;

/**
 * The main menu: ride alone, host a crew on a port, join one at an address, settings, quit.
 *
 * What it offers follows the build (URailheadSessionSubsystem::CanPlaySolo): the Client target
 * only joins. Actions go through the session subsystem; a refusal (a port out of range, an address
 * that cannot be read) is shown under the buttons.
 */
class SRailheadMainMenu : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadMainMenu) {}
		/** Any object in the menu's world, for reaching the session subsystem. */
		SLATE_ARGUMENT(TWeakObjectPtr<UObject>, Context)
		/** Settings was pressed. */
		SLATE_EVENT(FSimpleDelegate, OnSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	FReply Solo();
	FReply Host();
	FReply Join();
	FReply Settings();
	FReply Quit();

	/** A full-width menu button. */
	TSharedRef<SWidget> MakeButton(const FText& Label, FReply (SRailheadMainMenu::*Handler)());

	TWeakObjectPtr<UObject> Context;
	FSimpleDelegate OnSettings;
	TSharedPtr<SEditableTextBox> PortBox;
	TSharedPtr<SEditableTextBox> AddressBox;
	FText Status;
};
