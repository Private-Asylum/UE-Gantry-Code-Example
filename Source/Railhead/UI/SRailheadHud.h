// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Widgets/SCompoundWidget.h"

class APlayerController;

/**
 * The crew's HUD: where the train is, how it is running, and what the crew member can do.
 *
 * Every value is an attribute read each frame from the actors it describes (the line, the train,
 * the crew member), so the widget holds no game state of its own and cannot drift from it.
 */
class SRailheadHud : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRailheadHud) {}
		/** The local player the HUD belongs to. */
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, Owner)
	SLATE_END_ARGS()

	/** Builds the HUD's panels, every value bound to a getter below. */
	void Construct(const FArguments& InArgs);

	/**
	 * Shows a stretch's name, under its route's, across the screen for a few seconds.
	 * @param Route The route's name.
	 * @param Stretch The stretch's name.
	 */
	void ShowTitle(const FText& Route, const FText& Stretch);

	/**
	 * Adds a note to the notes column, highest priority at the top.
	 * @param Id The note's id, for RemoveNote.
	 * @param Title Its heading.
	 * @param Body A line or two under it.
	 * @param Priority Higher sits higher.
	 */
	void AddNote(int32 Id, const FText& Title, const FText& Body, int32 Priority);

	/**
	 * Takes a note out of the column.
	 * @param Id The id it was added with.
	 */
	void RemoveNote(int32 Id);

	/** Keeps the time the title fades by. */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	/** @return The stretch and its route, or the game's name before the line starts. */
	FText GetStretchText() const;

	/** @return What the line is doing: the tunnel ahead, or how much lies loaded beyond it. */
	FText GetStatusText() const;

	/** @return The train's speed in km/h. */
	FText GetSpeedText() const;

	/** @return Boiler pressure, 0 to 1, for the gauge. */
	TOptional<float> GetPressure() const;

	/** @return The gauge's colour: red when low, white when blowing off, brass otherwise. */
	FSlateColor GetPressureColor() const;

	/** @return The pressure as a percentage, or a warning when it is low or full. */
	FText GetPressureText() const;

	/** @return What pressing Interact would do, or nothing. */
	FText GetPromptText() const;

	/** @return How many crew are aboard, and this player's shovels. */
	FText GetCrewText() const;

	/** @return The title's opacity: full while it shows, fading out after. */
	float GetTitleOpacity() const;

	/** @return Visible while the Video page's Show Performance setting is on. */
	EVisibility GetStatsVisibility() const;

	/** @return Gantry's runtime readings: frame timings and open loads. */
	FText GetStatsText() const;

	/** @return The title's brass, at its current opacity. */
	FSlateColor GetTitleColor() const;

	/** @return The title's shadow, fading with it. */
	FLinearColor GetTitleShadow() const;

	/** A stretch's name, and its route's, as Gantry's tree knows them (installed or not); cached. */
	FText DescribeStretch(const FGameplayTag& Stretch, bool bWithRoute) const;

	/** The uncached lookup behind DescribeStretch. */
	FText LookUpStretch(const FGameplayTag& Stretch, bool bWithRoute) const;

	/** DescribeStretch's cache: stretch names alone, and with their routes. */
	mutable TMap<FGameplayTag, FText> StretchNames;
	mutable TMap<FGameplayTag, FText> RouteNames;

	/** The local player the HUD belongs to. */
	TWeakObjectPtr<APlayerController> Owner;

	/** Rebuilds the notes column from Notes. */
	void RebuildNotes();

	/** One note in the column, as AddNote was given it. */
	struct FNote
	{
		int32 Id = 0;
		FText Title;
		FText Body;
		int32 Priority = 0;
	};

	/** The notes, highest priority first. */
	TArray<FNote> Notes;

	/** The notes column. */
	TSharedPtr<class SVerticalBox> NotesBox;

	/** The title showing, and when it was shown, in Slate's time. */
	FText TitleRoute;
	FText TitleStretch;
	double TitleShownAt = -1000.0;

	/** Slate's time at the last tick. */
	double Now = 0.0;
};
