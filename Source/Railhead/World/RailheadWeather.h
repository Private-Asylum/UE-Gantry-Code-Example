// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RailheadWeather.generated.h"

// @doc
/**
 * Weather laid over a stretch's own light: what ARailheadLine::AddWeather takes. The defaults
 * change nothing, so a source sets only what it means. Several sources combine: scales and tints
 * multiply, fog blends and density add, snow takes the most any source asks for. The line eases
 * into and out of it as it does between stretches.
 */
USTRUCT(BlueprintType, meta = (DocumentationPolicy = "Strict"))
struct RAILHEAD_API FRailheadWeather
{
	GENERATED_BODY()

	/** The sun's brightness is multiplied by this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float SunScale = 1.0f;

	/** The sun's colour is multiplied by this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FLinearColor SunTint = FLinearColor::White;

	/** The haze's colour moves this far toward FogColor (0 not at all, 1 all the way). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float FogBlend = 0.0f;

	/** The haze colour FogBlend moves toward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FLinearColor FogColor = FLinearColor::White;

	/** Added to the haze's density. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.2"))
	float FogDensityAdd = 0.0f;

	/** How far the ground, track and scenery whiten under snow (0 none, 1 white over). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SnowCover = 0.0f;
};
