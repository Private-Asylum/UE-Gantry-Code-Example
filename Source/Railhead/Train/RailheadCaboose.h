// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Train/RailheadCar.h"

#include "RailheadCaboose.generated.h"

class UGantryExperienceDefinition;

// @doc
/**
 * The guard's van at the end of the train: a short van with a lookout cupola and a red tail lamp,
 * always last. Painted in the livery's caboose colour (URailheadLiverySettings), read again every
 * time a stretch is ready, so a mod's livery shows from the stretch it arrives on.
 */
UCLASS(meta = (ShortTooltip = "The guard's van, always last, painted in the livery."))
class RAILHEAD_API ARailheadCaboose : public ARailheadCar
{
	GENERATED_BODY()

public:

	ARailheadCaboose();

protected:

	/* AActor interface. */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/* AActor interface. */

	/** The van's body, then the cupola and the tail lamp. */
	virtual void BuildBody() override;

private:

	/**
	 * Paints the body from the livery, read again from the config.
	 * @param Experience The experience that became ready; unused, since the livery is the same
	 *        whichever it is.
	 */
	void ApplyLivery(const UGantryExperienceDefinition* Experience = nullptr);

	/** The caboose's listener on Gantry's low ready tier. */
	FDelegateHandle ReadyHandle;
};
