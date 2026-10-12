// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Core/GantryCoreTypes.h"

class UWorld;
struct FGantryLoadScopeInfo;

// @doc
/**
 * Everything Gantry reports while an experience activates, laid out as a feed of lines.
 *
 * Gantry draws nothing; it reports. This reads what it reports and orders it:
 *  - the mod units Gantry mounted, and their paks (the mod subsystem's units);
 *  - the activation's load scopes, nested as Gantry nests them: the activation, each experience in
 *    the chain, each stage, and under a stage its own work (each game feature plugin, each set of
 *    assets), from a load recording the feed starts when it begins;
 *  - each scope's assets, with where each comes from: the game, the engine, a plugin, or a mod
 *    unit and its paks.
 *
 * Loads here take a few frames, too quick to read, so the feed paces itself: lines are shown in
 * proportion to the time passed, spread over MinimumSeconds, and the feed is done once the
 * activation is over, every line has been shown and MinimumSeconds have passed.
 *
 * Plain C++ on the game thread; the loading screen's widget only reads it.
 */
class RAILHEAD_API FRailheadLoadFeed
{
public:

	/** One line of the feed. */
	struct FLine
	{
		/** What the line says. */
		FString Text;
		/** How deep it is indented: 0 the activation, 1 an experience, 2 a stage, 3 a stage's work. */
		int32 Depth = 0;
		/** The scope the line belongs to; invalid for lines about mods. */
		FGantryHandle Scope;
	};

	/** What the feed has seen, for the summary line. */
	struct FCounts
	{
		/** Experiences in the chain, stages, game feature plugins and assets loaded. */
		int32 Experiences = 0;
		int32 Stages = 0;
		int32 Features = 0;
		int32 Assets = 0;
		/** Mod units mounted, and their paks. */
		int32 Mods = 0;
		int32 Paks = 0;
	};

	/** Lines are spread over this long, and the feed lasts at least this long. */
	double MinimumSeconds = 5.0;

	/** Stops the feed's load recording. */
	~FRailheadLoadFeed();

	/**
	 * Starts a feed: starts a load recording (so earlier activations' scopes are left out) and
	 * lists the mounted mod units.
	 * @param NowSeconds The time now, in any steadily rising seconds; Step and IsDone use the same.
	 */
	void Begin(double NowSeconds);

	/** Reads the recording and lays out its scopes and assets as lines, below the mod lines. */
	void Collect();

	/**
	 * Advances how many lines are shown, in proportion to the time since Begin.
	 * @param NowSeconds The time now, in the same seconds as Begin was given.
	 */
	void Step(double NowSeconds);

	/**
	 * Whether the feed has nothing left to show.
	 * @param bActivationOver Whether the activation it follows is ready, failed or gone.
	 * @param NowSeconds The time now, in the same seconds as Begin was given.
	 * @return True once the activation is over, every line has been shown and MinimumSeconds have
	 *         passed.
	 */
	bool IsDone(bool bActivationOver, double NowSeconds) const;

	/** @return Every line collected so far, mod lines first. */
	const TArray<FLine>& GetLines() const { return Lines; }

	/** @return How many of the lines are shown, from the first. */
	int32 GetShown() const { return Shown; }

	/** @return What the feed has seen, for the summary line. */
	const FCounts& GetCounts() const { return Counts; }

	/** @return The newest line shown, without its indentation; empty before the first. */
	FString GetNewestShown() const;

	/** @return Shown over all collected, 0 to 1. */
	float GetShownFraction() const;

private:

	/** Where an asset comes from, by its content root. */
	FString Origin(const FString& Path) const;

	/** Every line collected, mod lines first. */
	TArray<FLine> Lines;

	/** How many lines are shown. */
	int32 Shown = 0;

	/** When the feed began, in the seconds Begin was given. */
	double OpenedAt = 0.0;

	/** What the feed has seen. */
	FCounts Counts;

	/** The load recording the feed reads: every scope since Begin, in tree order. */
	FGantryHandle Recording;

	/** The mod lines, which head the feed. */
	TArray<FLine> ModLines;

	/** Mounted mod units by content root, with their paks as one line. */
	TMap<FString, FString> ModPaks;
};
