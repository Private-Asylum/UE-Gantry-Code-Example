// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Experience/RailheadStretchDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadStretchDefinition)

#define LOCTEXT_NAMESPACE "RailheadStretchDefinition"

void URailheadStretchDefinition::GetSceneryAssets(TArray<FSoftObjectPath>& OutPaths) const
{
	for (const FRailheadSceneryLayer& Layer : Scenery)
	{
		for (const FRailheadSceneryPart& Part : Layer.Parts)
		{
			if (!Part.Mesh.IsNull())
			{
				OutPaths.AddUnique(Part.Mesh.ToSoftObjectPath());
			}
		}
	}
}

#if WITH_EDITOR
EDataValidationResult URailheadStretchDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!NextStretch.IsValid())
	{
		Context.AddError(LOCTEXT("NoNextStretch", "Next Stretch is empty: the tunnel at the end of this stretch would lead nowhere."));
		Result = EDataValidationResult::Invalid;
	}

	for (const FRailheadSceneryLayer& Layer : Scenery)
	{
		if (Layer.Parts.IsEmpty())
		{
			Context.AddWarning(FText::Format(LOCTEXT("EmptyLayer", "Scenery layer {0} has no parts, so it places nothing."), FText::FromName(Layer.Name)));
		}
		if (Layer.LateralMaxMeters < Layer.LateralMinMeters)
		{
			Context.AddError(FText::Format(LOCTEXT("LateralRange", "Scenery layer {0}: Lateral Max is less than Lateral Min."), FText::FromName(Layer.Name)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
