// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "RailheadInputConfig.generated.h"

class UInputAction;

/**
 * The crew's input actions, so code can bind them without naming assets. EXP_Base carries it in
 * its Data, beside the Add Input Mappings stage that puts IMC_Crew's keys on every player; the
 * crew finds it with Find Experience Data once the experience has loaded it.
 */
UCLASS(BlueprintType)
class URailheadInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Walk (2D). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Move;

	/** Look (2D). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Look;

	/** Jump. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Jump;

	/** Pick up or put down coal, stoke the firebox. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Interact;

	/** The pause menu. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Menu;
};
