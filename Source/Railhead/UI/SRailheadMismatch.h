// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

/**
 * What a client sees when the server's stretch cannot be ridden here: usually a mod the server has
 * and this machine has not. Gantry's reason says which experiences differ ("Server: ... Here:
 * ..."); the way out is back to the main menu.
 */
class SRailheadMismatch : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadMismatch) {}
		/** Any object in the world, for the session subsystem. */
		SLATE_ARGUMENT(TWeakObjectPtr<UObject>, Context)
		/** Gantry's reason. */
		SLATE_ARGUMENT(FText, Reason)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
