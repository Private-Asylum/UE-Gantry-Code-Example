// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "World/RailheadSceneryBuilder.h"

#include "Engine/StaticMesh.h"
#include "Math/RandomStream.h"
#include "Misc/Crc.h"

namespace RailheadScenery
{
	namespace
	{
		/** Ground tiles, tunnel segments and lamp spacing, in metres. */
		constexpr double GroundTile = 50.0;
		constexpr double TunnelSegment = 10.0;
		constexpr double LampSpacing = 25.0;

		/** One stream per slot, so a prop does not move when its neighbours change. */
		FRandomStream SlotStream(uint32 Seed, int32 Layer, int64 Slot, int32 Side)
		{
			uint32 Hash = HashCombine(Seed, ::GetTypeHash(Layer));
			Hash = HashCombine(Hash, ::GetTypeHash(Slot));
			Hash = HashCombine(Hash, ::GetTypeHash(Side));
			return FRandomStream(static_cast<int32>(Hash));
		}

		/**
		 * How far a layer's prop reaches toward the track from its foot, in metres at scale 1: the
		 * furthest any part comes on the track side, or in any direction when the prop turns at random.
		 */
		double TrackReach(const FRailheadSceneryLayer& Layer, bool bLeft)
		{
			double Reach = 0.0;
			for (const FRailheadSceneryPart& Part : Layer.Parts)
			{
				const double HalfX = FMath::Abs(Part.OffsetMeters.X) + Part.SizeMeters.X * 0.5;
				const double HalfY = Part.SizeMeters.Y * 0.5;
				// The track is toward +Y from a prop on the left, toward -Y from one on the right.
				const double TowardTrack = (bLeft ? Part.OffsetMeters.Y : -Part.OffsetMeters.Y) + HalfY;
				const double PartReach = Layer.bRandomYaw
					? FMath::Sqrt(FMath::Square(HalfX) + FMath::Square(FMath::Abs(Part.OffsetMeters.Y) + HalfY))
					: TowardTrack;
				Reach = FMath::Max(Reach, PartReach);
			}
			return Reach;
		}

		/** Adds a box-like piece: centre and size in metres, X along the track. */
		void AddPiece(FPlacement& Result, const FPlacementRequest& Request, const TCHAR* Key, const FLinearColor& Color, double X, const FVector& Centre, const FVector& Size, float Glow = 0.0f, bool bShadows = true)
		{
			FBatch& Batch = Result.Batches.FindOrAdd(Key);
			Batch.Mesh = Request.Cube;
			Batch.Color = Color;
			Batch.Glow = Glow;
			Batch.bShadows = bShadows;
			Batch.Transforms.Emplace(FRotator::ZeroRotator, FVector((X - Request.Start + Centre.X) * Cm, Centre.Y * Cm, Centre.Z * Cm), Size);
		}

		void BuildGround(FPlacement& Result, const FPlacementRequest& Request, const FRange& Range, double Start, double End)
		{
			const FLinearColor Color = Range.bTunnel ? Request.Rock : (Range.Stretch ? Range.Stretch->Ground : FLinearColor(0.2f, 0.2f, 0.2f));
			const FString Key = FString::Printf(TEXT("Ground|%s"), *Color.ToFColor(true).ToHex());
			for (double X = FMath::FloorToDouble(Start / GroundTile) * GroundTile; X < End; X += GroundTile)
			{
				const double TileStart = FMath::Max(X, Start);
				const double TileEnd = FMath::Min(X + GroundTile, End);
				AddPiece(Result, Request, *Key, Color, (TileStart + TileEnd) * 0.5, FVector(0.0, 0.0, -1.1), FVector(TileEnd - TileStart, 1200.0, 1.0), 0.0f, false);
			}
		}

		void BuildTunnel(FPlacement& Result, const FPlacementRequest& Request, const FRange& Range, double Start, double End)
		{
			const FLinearColor Rock = Request.Rock;
			const FLinearColor Hill = Rock * 1.6f;

			// Walls, roof and the hill above, segment by segment.
			for (double X = FMath::FloorToDouble(Start / TunnelSegment) * TunnelSegment; X < End; X += TunnelSegment)
			{
				const double SegStart = FMath::Max(X, Start);
				const double SegEnd = FMath::Min(X + TunnelSegment, End);
				const double Mid = (SegStart + SegEnd) * 0.5;
				const double Len = SegEnd - SegStart;
				AddPiece(Result, Request, TEXT("Tunnel|Rock"), Rock, Mid, FVector(0.0, -5.0, 3.5), FVector(Len, 1.0, 8.0));
				AddPiece(Result, Request, TEXT("Tunnel|Rock"), Rock, Mid, FVector(0.0, 5.0, 3.5), FVector(Len, 1.0, 8.0));
				AddPiece(Result, Request, TEXT("Tunnel|Rock"), Rock, Mid, FVector(0.0, 0.0, 8.0), FVector(Len, 11.0, 1.0));
				AddPiece(Result, Request, TEXT("Tunnel|Hill"), Hill, Mid, FVector(0.0, 0.0, 22.0), FVector(Len, 90.0, 28.0));
				// The hill's lower slopes stop at the tunnel walls: one block either side, never across the bore.
				AddPiece(Result, Request, TEXT("Tunnel|Hill"), Hill, Mid, FVector(0.0, -40.25, 6.0), FVector(Len, 69.5, 12.0));
				AddPiece(Result, Request, TEXT("Tunnel|Hill"), Hill, Mid, FVector(0.0, 40.25, 6.0), FVector(Len, 69.5, 12.0));
			}

			// Lamps along both walls.
			const FLinearColor Lamp(1.0f, 0.75f, 0.4f);
			for (double X = FMath::CeilToDouble(Start / LampSpacing) * LampSpacing; X < End; X += LampSpacing)
			{
				AddPiece(Result, Request, TEXT("Tunnel|Lamp"), Lamp, X, FVector(0.0, -4.4, 5.0), FVector(0.6, 0.2, 0.3), 25.0f, false);
				AddPiece(Result, Request, TEXT("Tunnel|Lamp"), Lamp, X, FVector(0.0, 4.4, 5.0), FVector(0.6, 0.2, 0.3), 25.0f, false);
			}

			// A stone face at each end that falls in the placement: two piers and a lintel round the bore.
			const FLinearColor Face(0.32f, 0.3f, 0.27f);
			for (const double Portal : { Range.Start, Range.End })
			{
				if (Portal >= Request.Start && Portal < Request.End)
				{
					AddPiece(Result, Request, TEXT("Tunnel|Face"), Face, Portal, FVector(0.0, -25.5, 18.0), FVector(3.0, 41.0, 40.0));
					AddPiece(Result, Request, TEXT("Tunnel|Face"), Face, Portal, FVector(0.0, 25.5, 18.0), FVector(3.0, 41.0, 40.0));
					AddPiece(Result, Request, TEXT("Tunnel|Face"), Face, Portal, FVector(0.0, 0.0, 23.0), FVector(3.0, 10.0, 30.0));
				}
			}
		}

		void BuildStretch(FPlacement& Result, const FPlacementRequest& Request, const FStretchSpec& Stretch, double Start, double End)
		{
			for (int32 LayerIndex = 0; LayerIndex < Stretch.Layers.Num(); ++LayerIndex)
			{
				const FLayerSpec& Layer = Stretch.Layers[LayerIndex];
				const double Spacing = FMath::Max(1.0, Layer.Spacing);
				const int64 FirstSlot = FMath::FloorToInt64(Start / Spacing) - 1;
				const int64 LastSlot = FMath::CeilToInt64(End / Spacing) + 1;

				for (int64 Slot = FirstSlot; Slot <= LastSlot; ++Slot)
				{
					for (int32 Side = 0; Side < 2; ++Side)
					{
						const bool bLeft = Side == 0;
						if ((Layer.Side == ERailheadTrackSide::Left && !bLeft) || (Layer.Side == ERailheadTrackSide::Right && bLeft))
						{
							continue;
						}

						FRandomStream Stream = SlotStream(Stretch.Seed, LayerIndex, Slot, Side);
						if (Stream.FRand() > Layer.Chance)
						{
							continue;
						}
						const double X = (static_cast<double>(Slot) + (Stream.FRand() - 0.5) * Layer.Jitter) * Spacing;
						if (X < Start || X >= End)
						{
							continue;
						}
						const double Distance = FMath::Lerp(Layer.LateralMin, Layer.LateralMax, static_cast<double>(Stream.FRand()));
						const double Scale = FMath::Lerp(Layer.ScaleMin, Layer.ScaleMax, static_cast<double>(Stream.FRand()));
						const double Yaw = Layer.bRandomYaw ? Stream.FRand() * 360.0 : 0.0;

						// A big prop, scaled up or turned across the track, could reach the train: push it out
						// until its nearest edge is clear. Done after every draw, so placement stays the same.
						const double Clear = (bLeft ? Layer.ReachLeft : Layer.ReachRight) * Scale + Request.Clearance;
						const double Lateral = FMath::Max(Distance, Clear) * (bLeft ? -1.0 : 1.0);
						const FTransform Prop(FRotator(0.0, Yaw, 0.0), FVector((X - Request.Start) * Cm, Lateral * Cm, -60.0), FVector(Scale));

						for (int32 PartIndex = 0; PartIndex < Layer.Parts.Num(); ++PartIndex)
						{
							const FPartSpec& Part = Layer.Parts[PartIndex];
							const FString Key = FString::Printf(TEXT("%s|%d|%d"), *Stretch.Name, LayerIndex, PartIndex);
							FBatch& Batch = Result.Batches.FindOrAdd(Key);
							Batch.Mesh = Part.Mesh;
							Batch.Color = Part.Color;
							Batch.Glow = Part.Glow;
							Batch.Transforms.Add(FTransform(FRotator::ZeroRotator, Part.OffsetMeters * Cm, Part.SizeMeters) * Prop);
						}
					}
				}
			}
		}

#if !UE_BUILD_SHIPPING
		/** The bounds of a batch's mesh, from the request's copies (workers cannot ask the mesh). */
		FBox BoundsOf(const FPlacementRequest& Request, const FString& Key, const FBatch& Batch)
		{
			if (Key.StartsWith(TEXT("Ground|")) || Key.StartsWith(TEXT("Tunnel|")))
			{
				return Request.CubeBounds;
			}
			for (const FRange& Range : Request.Ranges)
			{
				if (!Range.Stretch.IsValid())
				{
					continue;
				}
				for (const FLayerSpec& Layer : Range.Stretch->Layers)
				{
					for (const FPartSpec& Part : Layer.Parts)
					{
						if (Part.Mesh == Batch.Mesh)
						{
							return Part.MeshBounds;
						}
					}
				}
			}
			return FBox(ForceInit);
		}
#endif
	}

	TSharedRef<const FStretchSpec> MakeStretchSpec(const URailheadStretchDefinition& Stretch)
	{
		TSharedRef<FStretchSpec> Spec = MakeShared<FStretchSpec>();
		Spec->Tag = Stretch.ExperienceTag;
		Spec->Name = Stretch.ExperienceTag.ToString();
		// The same on every machine, unlike an FName's hash.
		Spec->Seed = FCrc::StrCrc32(*Spec->Name);
		Spec->Ground = Stretch.GroundColor;
		for (const FRailheadSceneryLayer& Layer : Stretch.Scenery)
		{
			FLayerSpec& Out = Spec->Layers.AddDefaulted_GetRef();
			Out.Spacing = Layer.SpacingMeters;
			Out.Jitter = Layer.Jitter;
			Out.Chance = Layer.Chance;
			Out.Side = Layer.Side;
			Out.LateralMin = Layer.LateralMinMeters;
			Out.LateralMax = Layer.LateralMaxMeters;
			Out.ScaleMin = Layer.ScaleMin;
			Out.ScaleMax = Layer.ScaleMax;
			Out.bRandomYaw = Layer.bRandomYaw;
			Out.ReachLeft = TrackReach(Layer, true);
			Out.ReachRight = TrackReach(Layer, false);
			for (const FRailheadSceneryPart& Part : Layer.Parts)
			{
				UStaticMesh* Mesh = Part.Mesh.Get();
				if (!Mesh)
				{
					continue;
				}
				FPartSpec& PartSpec = Out.Parts.AddDefaulted_GetRef();
				PartSpec.Mesh = Mesh;
				PartSpec.MeshBounds = Mesh->GetBoundingBox();
				PartSpec.Color = Part.Color;
				PartSpec.Glow = Part.Glow;
				PartSpec.SizeMeters = Part.SizeMeters;
				PartSpec.OffsetMeters = Part.OffsetMeters;
			}
		}
		return Spec;
	}

	uint32 TrackKey(const TArray<FRange>& Ranges)
	{
		uint32 Key = 0;
		for (const FRange& Range : Ranges)
		{
			Key = HashCombine(Key, ::GetTypeHash(FMath::RoundToInt64(FMath::Clamp(Range.Start, -1.0e12, 1.0e12) * 10.0)));
			Key = HashCombine(Key, ::GetTypeHash(FMath::RoundToInt64(FMath::Clamp(Range.End, -1.0e12, 1.0e12) * 10.0)));
			Key = HashCombine(Key, ::GetTypeHash(Range.bTunnel));
			Key = HashCombine(Key, ::GetTypeHash(Range.Stretch.Get()));
		}
		return Key;
	}

	FPlacement Place(const FPlacementRequest& Request)
	{
		FPlacement Result;
		Result.Start = Request.Start;

		for (const FRange& Range : Request.Ranges)
		{
			const double Start = FMath::Max(Range.Start, Request.Start);
			const double End = FMath::Min(Range.End, Request.End);
			if (Start >= End && !(Range.bTunnel && Range.End >= Request.Start && Range.End < Request.End))
			{
				continue;
			}
			if (Start < End)
			{
				BuildGround(Result, Request, Range, Start, End);
			}
			if (Range.bTunnel)
			{
				BuildTunnel(Result, Request, Range, Start, End);
			}
			else if (Range.Stretch.IsValid() && Start < End)
			{
				BuildStretch(Result, Request, *Range.Stretch, Start, End);
			}
		}

#if !UE_BUILD_SHIPPING
		// Everything scrolls past the train, so anything whose cross-section overlaps the train's will
		// pass through it sooner or later, wherever it is along the track.
		const FBox Section(FVector(-DBL_MAX, -Request.TrainHalfWidth * Cm, 10.0), FVector(DBL_MAX, Request.TrainHalfWidth * Cm, Request.TrainHeight * Cm));
		for (const TPair<FString, FBatch>& Pair : Result.Batches)
		{
			const FBox Bounds = BoundsOf(Request, Pair.Key, Pair.Value);
			if (!Bounds.IsValid)
			{
				continue;
			}
			for (const FTransform& Transform : Pair.Value.Transforms)
			{
				if (Bounds.TransformBy(Transform).Intersect(Section))
				{
					Result.Crossings.Add(Pair.Key);
					break;
				}
			}
		}
#endif

		return Result;
	}

	FString PaintKey(const FBatch& Batch)
	{
		return FString::Printf(TEXT("%s|%.2f"), *Batch.Color.ToFColor(true).ToHex(), Batch.Glow);
	}
}
