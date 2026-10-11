// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"
#include "GameplayTagContainer.h"
#include "Messages/GantryMessageTypes.h"
#include "UIExtension/GantryUIExtensionTypes.h"

#include "RailheadHUD.generated.h"

class SRailheadHud;

// @doc
/**
 * Puts the crew's Slate HUD on screen for the local player: the stretch, the tunnel, speed, the
 * boiler, and what the crew member could do with what they are looking at. No widget assets: the
 * whole thing is code (SRailheadHud), styled in one place.
 *
 * Two things reach it without anyone naming it: the line's announcements (Gantry messages on
 * Railhead.Message.Line.Daylight bring up each stretch's title), and notes in the
 * Railhead.HUD.Notes UI extension slot (a route's experience puts its note there).
 *
 * On a client, a stretch the server runs that Gantry cannot activate here (a mod the host has and
 * this machine has not) brings up SRailheadMismatch with Gantry's reason and the way out.
 */
UCLASS()
class RAILHEAD_API ARailheadHUD : public AHUD
{
	GENERATED_BODY()

public:

	ARailheadHUD();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	/**
	 * Low tier: the HUD comes up once everything else in the stretch is standing, which is what
	 * Gantry's low ready tier is for. It stays hidden until then.
	 */
	void HandleFirstReady(const class UGantryExperienceDefinition* Experience);

	/** The line has come out of a tunnel into a stretch: show its title. */
	void HandleDaylight(FGameplayTag Stretch);

	/** Client: the server's experience cannot be activated here. */
	UFUNCTION()
	void HandleExperienceFailed(FGameplayTag ExperienceTag, const FString& Reason);

	/** A note arrived in, or left, the notes slot. */
	void HandleNote(EGantryUIExtensionAction Action, const FGantryUIExtensionEntry& Entry);

	TSharedPtr<SRailheadHud> Widget;

	/** The mismatch screen, once shown. */
	TSharedPtr<class SWidget> Mismatch;

	FGantryMessageListenerHandle DaylightListener;
	FGantryUIExtensionSlotHandle NotesSlot;
};
