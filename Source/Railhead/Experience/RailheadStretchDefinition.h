// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/GantryExperienceDefinition.h"

#include "RailheadStretchDefinition.generated.h"

class UMaterialInterface;
class UStaticMesh;

/** Which side of the track a scenery layer is placed on, looking the way the train travels. */
UENUM(BlueprintType)
enum class ERailheadTrackSide : uint8
{
	/** Left of the track only. */
	Left,
	/** Right of the track only. */
	Right,
	/** Both sides, each placed independently. */
	Both
};

/**
 * One mesh of a scenery prop. A lighthouse is three parts (tower, gallery, lamp) placed together;
 * a pine is two (trunk, crown).
 */
USTRUCT(BlueprintType)
struct RAILHEAD_API FRailheadSceneryPart
{
	GENERATED_BODY()

	/** The mesh. Railhead's own props use the engine's basic shapes, which are 1 m across. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Paint colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FLinearColor Color = FLinearColor::White;

	/** How strongly the paint glows: 0 for none, around 5 for a lamp. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part", meta = (ClampMin = "0.0", UIMax = "20.0"))
	float Glow = 0.0f;

	/** Size in metres, for a 1 m mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FVector SizeMeters = FVector(1.0);

	/** Where the part's centre sits relative to the prop's foot, in metres. Z is up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FVector OffsetMeters = FVector::ZeroVector;
};

/**
 * One kind of prop beside the track, repeated along a stretch.
 *
 * Placement is a pure function of the stretch, the layer and the slot number, so every machine
 * places the same prop in the same spot without anything being replicated.
 */
USTRUCT(BlueprintType)
struct RAILHEAD_API FRailheadSceneryLayer
{
	GENERATED_BODY()

	/** What a person would call it ("Warehouses"); shown in logs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	FName Name;

	/** The meshes one prop is made of. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	TArray<FRailheadSceneryPart> Parts;

	/** Distance between slots along the track, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "1.0", Units = "m"))
	float SpacingMeters = 40.0f;

	/** How far a prop may wander from its slot, as a fraction of the spacing. 0 for a regular row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Jitter = 0.5f;

	/** Chance that a slot has a prop at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Chance = 1.0f;

	/** Which side of the track. Both places each side independently. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	ERailheadTrackSide Side = ERailheadTrackSide::Both;

	/** Nearest a prop's foot comes to the track's centre line, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.0", Units = "m"))
	float LateralMinMeters = 15.0f;

	/** Furthest a prop's foot goes from the track's centre line, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.0", Units = "m"))
	float LateralMaxMeters = 60.0f;

	/** Smallest random scale applied to the whole prop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.01"))
	float ScaleMin = 1.0f;

	/** Largest random scale applied to the whole prop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (ClampMin = "0.01"))
	float ScaleMax = 1.0f;

	/** Turn each prop to a random heading. Off for things that line up with the track (fences, sea). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	bool bRandomYaw = true;
};

/** Sun and air for a stretch. The line eases from one stretch's light to the next. */
USTRUCT(BlueprintType)
struct RAILHEAD_API FRailheadStretchLight
{
	GENERATED_BODY()

	/** Sun elevation in degrees: 90 is overhead, 5 is a long sunset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "-10.0", ClampMax = "90.0", Units = "deg"))
	float SunElevation = 40.0f;

	/** Sun heading in degrees, relative to the direction of travel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (Units = "deg"))
	float SunHeading = 30.0f;

	/** Sun colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	FLinearColor SunColor = FLinearColor::White;

	/** Sun brightness in lux. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0.0", UIMax = "120000.0"))
	float SunLux = 10.0f;

	/** Haze colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	FLinearColor FogColor = FLinearColor(0.45f, 0.55f, 0.7f);

	/** Haze density; 0.02 is a clear day. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0.0", UIMax = "0.2"))
	float FogDensity = 0.02f;
};

// @doc
/**
 * A stretch of track: Railhead's own kind of Gantry experience.
 *
 * Gantry experiences are data assets, and a game adds the fields it needs by subclassing the
 * definition in C++ (or Blueprint). A stretch says how long it is, what grows beside it, what the
 * light is like and which stretch comes next. Everything that turns that data into a world is
 * code elsewhere: the route's scenery stage loads the meshes, and the line places them.
 *
 * Stretches are children of a route experience (Coast Line), so their activation chain is the base
 * experience, the route, then the stretch, and the route's stages serve every stretch on it.
 */
UCLASS(BlueprintType, meta = (GantryAPI, GantryAPIBlueprint, ShortTooltip = "A stretch of track: its length, scenery, light and the stretch after it."))
class RAILHEAD_API URailheadStretchDefinition : public UGantryExperienceDefinition
{
	GENERATED_BODY()

public:

	/**
	 * Every asset the stretch's scenery uses, for the scenery stage to load.
	 * @param OutPaths Appended to, without duplicates.
	 */
	void GetSceneryAssets(TArray<FSoftObjectPath>& OutPaths) const;

	/** Track length from one tunnel to the next, in metres. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stretch", meta = (ClampMin = "100.0", Units = "m"))
	float LengthMeters = 1500.0f;

	/**
	 * The shortest time the train spends in the tunnel at the end of this stretch, even when the
	 * next stretch loads at once. Long enough to read the tunnel as a tunnel.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stretch", meta = (ClampMin = "0.0", Units = "s"))
	float MinimumTunnelSeconds = 6.0f;

	/** The stretch the tunnel at the end of this one leads to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stretch", meta = (Categories = "Gantry.ExperienceId"))
	FGameplayTag NextStretch;

	/** Colour of the land beside the track. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	FLinearColor GroundColor = FLinearColor(0.25f, 0.3f, 0.18f);

	/** Sun and haze. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look")
	FRailheadStretchLight Light;

	/** Everything beside the track. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Look", meta = (TitleProperty = "Name"))
	TArray<FRailheadSceneryLayer> Scenery;

#if WITH_EDITOR
	/** Adds stretch checks to Gantry's own: a next stretch, and layers that can place something. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
