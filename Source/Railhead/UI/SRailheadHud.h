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

	void Construct(const FArguments& InArgs);

	/** Shows a stretch's name across the screen for a few seconds. */
	void ShowTitle(const FText& Route, const FText& Stretch);

	/** Adds a note to the notes column, highest priority at the top. */
	void AddNote(int32 Id, const FText& Title, const FText& Body, int32 Priority);

	/** Takes a note out of the column. */
	void RemoveNote(int32 Id);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	FText GetStretchText() const;
	FText GetStatusText() const;
	FText GetSpeedText() const;
	TOptional<float> GetPressure() const;
	FSlateColor GetPressureColor() const;
	FText GetPressureText() const;
	FText GetPromptText() const;
	FText GetCrewText() const;
	float GetTitleOpacity() const;
	EVisibility GetStatsVisibility() const;
	FText GetStatsText() const;
	FSlateColor GetTitleColor() const;
	FLinearColor GetTitleShadow() const;

	/** A stretch's name, and its route's, as Gantry's tree knows them (installed or not); cached. */
	FText DescribeStretch(const FGameplayTag& Stretch, bool bWithRoute) const;

	/** The uncached lookup behind DescribeStretch. */
	FText LookUpStretch(const FGameplayTag& Stretch, bool bWithRoute) const;

	mutable TMap<FGameplayTag, FText> StretchNames;
	mutable TMap<FGameplayTag, FText> RouteNames;

	TWeakObjectPtr<APlayerController> Owner;

	/** Rebuilds the notes column from Notes. */
	void RebuildNotes();

	struct FNote
	{
		int32 Id = 0;
		FText Title;
		FText Body;
		int32 Priority = 0;
	};
	TArray<FNote> Notes;
	TSharedPtr<class SVerticalBox> NotesBox;

	FText TitleRoute;
	FText TitleStretch;
	double TitleShownAt = -1000.0;
	double Now = 0.0;
};
