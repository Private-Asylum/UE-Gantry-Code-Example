// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class FRailheadLoadFeed;

/**
 * Railhead's loading screen: what Gantry reports while an experience activates.
 *
 * Two layers. The curtain covers the whole screen, opaque, with the game's name, a throbber and
 * the newest line; it is only up for the first activation in a world, so nothing of the world
 * shows until it is ready. The panel, top right, is up for every activation: the feed of mods,
 * scopes and assets (FRailheadLoadFeed), a summary of the counts and a bar. Swapping stretches
 * happens in a tunnel with the train still running, so there it is the panel alone.
 *
 * The widget keeps nothing: it reads the feed every frame.
 */
class SRailheadLoadingScreen : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadLoadingScreen) : _bBlocking(false) {}
		/** The feed to show. */
		SLATE_ARGUMENT(TSharedPtr<const FRailheadLoadFeed>, Feed)
		/** Cover the whole screen: the first activation in a world. */
		SLATE_ARGUMENT(bool, bBlocking)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** How many feed lines the panel shows, newest at the bottom. */
	static constexpr int32 FeedLines = 18;

private:

	FText GetFeedLine(int32 Slot) const;
	FText GetSummary() const;
	FText GetCurtainStatus() const;
	TOptional<float> GetProgress() const;

	TSharedPtr<const FRailheadLoadFeed> Feed;
};
