// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Experience/RailheadStretchDefinition.h"

#include "GameplayTagContainer.h"
#include "Math/Box.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UStaticMesh;

// @doc
/**
 * Where everything beside the track goes, worked out away from the game thread.
 *
 * The line asks for everything in view each time the train has covered a few metres, or the track
 * changes (a tunnel's exit opening, a stretch's scenery arriving). Placing it is a pure function of
 * plain data copied on the game thread: no UObject is touched while it runs, and meshes travel as
 * weak pointers the game thread resolves when it applies the result. So placement runs on a worker,
 * and the game thread only writes finished transforms into instanced components it already has,
 * in place: no component is created, cleared or rebuilt as the train moves.
 *
 * Placement is the same on every machine: each prop is a function of its stretch, its layer and
 * its slot along the track, never of when or where it was built.
 */
namespace RailheadScenery
{
	/** Centimetres per metre, where the two meet. */
	inline constexpr double Cm = 100.0;

	/** One mesh of a prop, copied from the stretch definition. */
	struct FPartSpec
	{
		/** The mesh, resolved on the game thread when the placement is applied. */
		TWeakObjectPtr<UStaticMesh> Mesh;
		/** The mesh's bounds, read on the game thread, for the clearance and crossing checks. */
		FBox MeshBounds = FBox(ForceInit);
		/** Paint colour, glow, size and offset: FRailheadSceneryPart's. */
		FLinearColor Color = FLinearColor::White;
		float Glow = 0.0f;
		FVector SizeMeters = FVector(1.0);
		FVector OffsetMeters = FVector::ZeroVector;
	};

	/**
	 * One layer of props, copied from the stretch definition, with how far a prop reaches toward the
	 * track.
	 */
	struct FLayerSpec
	{
		/** The meshes one prop is made of. */
		TArray<FPartSpec> Parts;
		/**
		 * FRailheadSceneryLayer's spacing, jitter, chance, side, lateral range (metres), scale range
		 * and random yaw.
		 */
		double Spacing = 40.0;
		double Jitter = 0.5;
		double Chance = 1.0;
		ERailheadTrackSide Side = ERailheadTrackSide::Both;
		double LateralMin = 15.0;
		double LateralMax = 60.0;
		double ScaleMin = 1.0;
		double ScaleMax = 1.0;
		bool bRandomYaw = true;
		/** Metres a prop reaches toward the track from its foot at scale 1, on the left and on the right. */
		double ReachLeft = 0.0;
		double ReachRight = 0.0;
	};

	/** A stretch's look, copied once from its definition so workers never read the asset. */
	struct FStretchSpec
	{
		/** The stretch's experience tag. */
		FGameplayTag Tag;
		/** The tag as text, for logs. */
		FString Name;
		/** Seeds every slot's random choices; from the name, so every machine has the same. */
		uint32 Seed = 0;
		/** Colour of the land beside the track. */
		FLinearColor Ground = FLinearColor::Gray;
		/** The stretch's scenery layers. */
		TArray<FLayerSpec> Layers;
	};

	/** A stretch of the track: a stretch's scenery, or a tunnel. Distances in metres. */
	struct FRange
	{
		/** Where the range begins and ends along the track. */
		double Start = 0.0;
		double End = 0.0;
		/** Whether the range is a tunnel. */
		bool bTunnel = false;
		/** The stretch's scenery; null for a tunnel, or a stretch this machine has no scenery for yet. */
		TSharedPtr<const FStretchSpec> Stretch;
	};

	/** Everything needed to place the scenery between two distances. */
	struct FPlacementRequest
	{
		/** The stretch of track to place, in metres; transforms come back relative to Start. */
		double Start = 0.0;
		double End = 0.0;
		/** The track between them, as ranges. */
		TArray<FRange> Ranges;
		/** The engine's cube and its bounds, for the ground and the tunnels. */
		TWeakObjectPtr<UStaticMesh> Cube;
		FBox CubeBounds = FBox(ForceInit);
		/** Colour of the tunnels and the rock above them. */
		FLinearColor Rock = FLinearColor::Black;
		/** ARailheadLine's TrackClearanceMeters, TrainHalfWidthMeters and TrainHeightMeters. */
		double Clearance = 4.0;
		double TrainHalfWidth = 1.7;
		double TrainHeight = 4.8;
	};

	/** One mesh in one paint: the instances of a single instanced component. */
	struct FBatch
	{
		/** The mesh every instance shares. */
		TWeakObjectPtr<UStaticMesh> Mesh;
		/** The paint every instance shares. */
		FLinearColor Color = FLinearColor::White;
		float Glow = 0.0f;
		/** Whether the instances cast shadows. */
		bool bShadows = true;
		/** Relative to the placement's start, in centimetres. */
		TArray<FTransform> Transforms;
	};

	/** Placed scenery, by batch key (one mesh in one paint). */
	struct FPlacement
	{
		/** Where the placement starts, in metres: the request's Start. */
		double Start = 0.0;
		/** The batches, by key. */
		TMap<FString, FBatch> Batches;
		/** Development builds: batches with an instance whose cross-section overlaps the train's. */
		TArray<FString> Crossings;
	};

	/** Copies what workers need from a stretch definition. Game thread. */
	RAILHEAD_API TSharedRef<const FStretchSpec> MakeStretchSpec(const URailheadStretchDefinition& Stretch);

	/**
	 * A number that changes whenever the ranges do: where they meet and whose scenery they use. Any
	 * thread.
	 */
	RAILHEAD_API uint32 TrackKey(const TArray<FRange>& Ranges);

	/** Places everything between the request's distances. Any thread; touches no UObject. */
	RAILHEAD_API FPlacement Place(const FPlacementRequest& Request);

	/** The paint a batch key wants, as one string: batches that share it share a material instance. */
	RAILHEAD_API FString PaintKey(const FBatch& Batch);
}
