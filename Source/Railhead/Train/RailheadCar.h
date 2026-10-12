// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "RailheadCar.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

/** Measures every car shares with the train, in centimetres. */
namespace RailheadTrain
{
	/** Deck top, where the crew stand, above the rails. */
	inline constexpr double DeckTop = 150.0;

	/** Width of decks and bodies. */
	inline constexpr double Width = 300.0;

	/** Radius of every wheel; the wheels stand on the rails. */
	inline constexpr double WheelRadius = 45.0;

	/** Space between two cars, bridged by a plate. */
	inline constexpr double Gap = 120.0;
}

/** What a car carries on its deck. */
UENUM(BlueprintType)
enum class ERailheadCarBody : uint8
{
	/** Low side walls, open ends: a wagon the crew walk through. */
	Open,

	/** Walls and a roof, ends open: a van the crew walk through. */
	Box,

	/** A tank along the deck, with a walkway on the left. */
	Tank,

	/** A bare deck. */
	Flat
};

// @doc
/**
 * A car the train can couple: deck, frame, wheels, a plate over the gap ahead, a lamp, and a body,
 * built from the engine's basic shapes and painted.
 *
 * The train's own cars are part of the train; these are the cars that come and go with
 * experiences. The Couple Cars stage (URailheadStage_CoupleCars) spawns them on the server and the
 * train couples them behind its wagons (tail cars, such as the caboose, stay last); they
 * replicate, attached to the train, so every machine sees the same consist.
 *
 * A Blueprint subclass is a new car: set its length, body and paint, and add components for
 * anything more. Nothing in the game needs to know about it.
 */
UCLASS(Blueprintable, meta = (GantryAPI, GantryAPIBlueprint, ShortTooltip = "A car the train can couple; subclass it for a new one."))
class RAILHEAD_API ARailheadCar : public AActor
{
	GENERATED_BODY()

public:

	ARailheadCar();

	/** Builds the car from its properties. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * Paints the body (walls, roof, tank) a new colour; deck, frame and wheels keep theirs.
	 * @param Color The body's new colour.
	 */
	UFUNCTION(BlueprintCallable, Category = "Railhead|Car")
	void PaintBody(FLinearColor Color);

	/** @return The car's length, buffer to buffer, in centimetres. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Car")
	float GetLength() const { return Length; }

	/** Length buffer to buffer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Car", meta = (ClampMin = "400.0", UIMax = "2000.0", Units = "cm"))
	float Length = 1300.0f;

	/** What it carries. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Car")
	ERailheadCarBody Body = ERailheadCarBody::Open;

	/** Paint of the walls, roof or tank. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Car")
	FLinearColor BodyColor = FLinearColor(0.22f, 0.12f, 0.06f);

	/** Paint of the deck. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Car")
	FLinearColor DeckColor = FLinearColor(0.32f, 0.1f, 0.06f);

	/** Stays at the very end of the train, behind every car coupled after it (a caboose). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Car")
	bool bTailCar = false;

protected:

	/**
	 * Adds the body for Body. Override to add to it (call the parent) or replace it; parts added
	 * with bIsBody are the ones PaintBody repaints.
	 */
	virtual void BuildBody();

	/**
	 * Adds one painted piece; centre and size in centimetres, relative to the car's centre at rail
	 * height.
	 * @param Name The component's name, unique on this car.
	 * @param Mesh One of the basic shapes, 1 m across (CubeMesh, CylinderMesh).
	 * @param Centre Where the piece's centre sits.
	 * @param SizeCm The piece's size.
	 * @param Color Its paint.
	 * @param bIsBody Whether PaintBody repaints it.
	 * @param Rotation Its turn, for cylinders laid along or across the track.
	 * @param Glow How strongly the paint glows; 0 for none.
	 * @return The piece.
	 */
	UStaticMeshComponent* AddPart(const FString& Name, UStaticMesh* Mesh, const FVector& Centre, const FVector& SizeCm, const FLinearColor& Color,
		bool bIsBody = false, const FRotator& Rotation = FRotator::ZeroRotator, float Glow = 0.0f);

	/** The engine's basic cube, for AddPart. */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** The engine's basic cylinder, for AddPart: wheels, tanks. */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

private:

	/** Removes what an earlier construction built (the editor reconstructs on every change). */
	void ClearParts();

	/** The car's centre at rail height; every part hangs from it. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Car")
	TObjectPtr<USceneComponent> Root;

	/** A lamp over the deck, so the crew can see their footing in a tunnel. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Car")
	TObjectPtr<UPointLightComponent> Lamp;

	/** Every piece the last construction built, removed by the next. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	/** The body's paints, which PaintBody changes. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> BodyPaints;

	/** The flat paint every piece is drawn with, tinted per piece (M_Railhead_Paint). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> PaintMaterial;
};
