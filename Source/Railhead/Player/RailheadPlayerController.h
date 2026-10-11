// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"

#include "RailheadPlayerController.generated.h"

class SRailheadPauseMenu;
class SRailheadSettings;
class SWidget;
class UGantryExperienceDefinition;

// @doc
/**
 * A crew member's controller. Answers Menu with the pause menu and, from it, the settings screen.
 *
 * Its keys come from the experience, not from here: EXP_Base's Add Input Mappings stage puts
 * IMC_Crew on every local player, and its Data carries the input actions (URailheadInputConfig),
 * which the controller binds Menu from once the experience is ready. The character binds the rest.
 */
UCLASS()
class RAILHEAD_API ARailheadPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ARailheadPlayerController();

	/** Opens the settings screen over whatever is showing. */
	void OpenSettings();

	/** Opens or closes the pause menu (or closes the settings screen, when that is up). */
	void ToggleMenu();

protected:

	/** Binds Menu once the experience (and its input config) is ready. */
	virtual void BeginPlay() override;

	/** Takes any menus off the screen. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Puts a widget on this player's screen. */
	void ShowWidget(const TSharedRef<SWidget>& Widget, int32 ZOrder);

	/** Takes a widget off this player's screen. */
	void HideWidget(const TSharedPtr<SWidget>& Widget);

	/** Input to the game, or to menus with a cursor, by whether a menu is up. */
	virtual void UpdateInputMode();

	/** Whether this controller always shows a menu (the main menu's controller). */
	virtual bool IsMenuController() const { return false; }

	TSharedPtr<SRailheadPauseMenu> PauseMenu;
	TSharedPtr<SRailheadSettings> Settings;

private:

	/** Binds Menu from the experience's input config, once. */
	void HandleExperienceReady(const UGantryExperienceDefinition* Experience);

	bool bMenuBound = false;
};
