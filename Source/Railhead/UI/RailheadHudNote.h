// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "RailheadHudNote.generated.h"

/**
 * A note for the HUD's notes column (the Railhead.HUD.Notes UI extension slot). The HUD draws
 * whatever notes are in the slot; a route's experience puts its own in with an Add UI Extensions
 * stage, so the HUD never names a route, and a mod could add one the same way.
 */
UCLASS(BlueprintType)
class URailheadHudNote : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The note's heading. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Note")
	FText Title;

	/** A line or two under it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Note", meta = (MultiLine = true))
	FText Body;
};
