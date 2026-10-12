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

	/** Builds the buttons this build can use, the port and address boxes, and the status line. */
	void Construct(const FArguments& InArgs);

private:

	/** Ride alone. */
	FReply Solo();

	/** Host a crew on the port in the port box. */
	FReply Host();

	/** Join the crew at the address in the address box. */
	FReply Join();

	/** Open the settings (OnSettings). */
	FReply Settings();

	/** Close the game. */
	FReply Quit();

	/** A full-width menu button. */
	TSharedRef<SWidget> MakeButton(const FText& Label, FReply (SRailheadMainMenu::*Handler)());

	/** Any object in the menu's world, for reaching the session subsystem. */
	TWeakObjectPtr<UObject> Context;

	/** Called when Settings is pressed. */
	FSimpleDelegate OnSettings;

	/** The port to host on. */
	TSharedPtr<SEditableTextBox> PortBox;

	/** The address to join. */
	TSharedPtr<SEditableTextBox> AddressBox;

	/** Why the last action was refused, shown under the buttons; empty otherwise. */
	FText Status;
};
