// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/GameFrameworkComponentManager.h"

#include "RailheadTrain.generated.h"

class ARailheadCar;
class UMaterialInterface;
class USpotLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

// @doc
/**
 * The train: a locomotive, its tender and three open wagons, built from the engine's basic shapes.
 *
 * The game mode builds it at the world's origin, where it stands still while ARailheadLine moves the world past it, so the crew
 * walk on an actor that never moves and nothing about the train needs movement replication. What
 * does replicate is the boiler: its pressure sets the train's speed, it drains as the train runs,
 * and the crew keep it up by carrying coal from the tender to the firebox in the cab.
 *
 * Interactive parts carry a component tag: `CoalPile` on the tender's coal, `Firebox` in the cab.
 *
 * Cars that come and go with experiences (ARailheadCar: the caboose, a mod's van) couple behind
 * the wagons, tail cars last. The train is a Game Framework Component Manager receiver, so a game
 * feature (a mod's included) can add components to it.
 */
UCLASS(meta = (ShortTooltip = "The train the crew ride and stoke. Stands still; the line moves the world."))
class RAILHEAD_API ARailheadTrain : public AActor
{
	GENERATED_BODY()

public:

	ARailheadTrain();

	/**
	 * The world's train.
	 * @param World Any world; may be null.
	 * @return The train placed in it, or null.
	 */
	static ARailheadTrain* Find(const UWorld* World);

	/* AActor interface. */
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/* AActor interface. */

	/** Server: a shovel of coal goes into the firebox. */
	void Stoke();

	/**
	 * Where a crew member stands when they join or fall off.
	 * @param Index Any number; spots repeat.
	 * @return A transform on one of the wagons, facing forward.
	 */
	FTransform GetCrewSpot(int32 Index) const;

	/** @return Boiler pressure, 0 (cold) to 1 (the safety valve lifts). */
	UFUNCTION(BlueprintPure, Category = "Railhead|Train")
	float GetPressure() const { return Pressure; }

	/**
	 * Server: couples a car behind the wagons (or, for a tail car, at the very end) and lays the
	 * train out again. Its attachment replicates.
	 * @param Car A car spawned for this train.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Railhead|Train")
	void CoupleCar(ARailheadCar* Car);

	/**
	 * Server: uncouples a car and closes the gap it leaves. The car is the caller's to destroy.
	 * @param Car A coupled car.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Railhead|Train")
	void UncoupleCar(ARailheadCar* Car);

	/** @return The coupled cars, front to back, on this machine. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Train")
	TArray<ARailheadCar*> GetCoupledCars() const;

	/** @return Metres from the train's centre to its front buffer. */
	float GetFrontMeters() const;

	/** @return Metres from the train's centre to the back of its last car, coupled cars included. */
	float GetRearMeters() const;

	/** @return The speed the boiler can hold right now, in metres per second. */
	UFUNCTION(BlueprintPure, Category = "Railhead|Train")
	float GetBoilerSpeed() const;

	/** Top speed with a full boiler, in metres per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Train", meta = (ClampMin = "1.0", Units = "m/s"))
	float TopSpeed = 22.0f;

	/** Pressure lost per second standing still. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Train", meta = (ClampMin = "0.0"))
	float IdleDrain = 0.004f;

	/** Pressure lost per second at top speed, on top of the idle drain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Train", meta = (ClampMin = "0.0"))
	float RunningDrain = 0.016f;

	/** Pressure one shovel of coal adds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Train", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShovelPressure = 0.12f;

	/** Pressure at the start of a session. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Railhead|Train", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartingPressure = 0.7f;

protected:

	/* AActor interface. */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/* AActor interface. */

private:

	/** Places every coupled car behind the last, tail cars at the end. */
	void LayOutCars();

	/** Server: coupled cars in coupling order. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ARailheadCar>> Cars;

	/** Where the last built-in wagon ends, in centimetres along X: coupled cars go behind it. */
	double TailX = 0.0;

	/** Adds one painted piece; centre and size in centimetres. */
	UStaticMeshComponent* AddPart(const TCHAR* Name, UStaticMesh* Mesh, const FVector& Centre, const FVector& SizeCm, const FLinearColor& Color, const FRotator& Rotation = FRotator::ZeroRotator, float Glow = 0.0f);

	/** Adds a wagon's deck, frame and wheels between two X positions. */
	void AddWagonBase(const TCHAR* Name, double RearX, double FrontX, const FLinearColor& Color);

	UPROPERTY(Replicated)
	float Pressure = 0.7f;

	UPROPERTY(VisibleAnywhere, Category = "Railhead|Train")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Railhead|Train")
	TObjectPtr<USpotLightComponent> Headlamp;

	/** The firebox's glow in the cab, which is most of the light the crew have in a tunnel. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Train")
	TObjectPtr<class UPointLightComponent> FireboxLight;

	/** A lamp over each wagon, so the crew can see their footing in a tunnel. */
	UPROPERTY(VisibleAnywhere, Category = "Railhead|Train")
	TArray<TObjectPtr<class UPointLightComponent>> WagonLamps;

	/** Every painted piece, with the paint each one gets. */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TArray<FLinearColor> PartColors;

	UPROPERTY()
	TArray<float> PartGlows;

	/** Where crew stand, relative to the train. */
	UPROPERTY()
	TArray<FVector> CrewSpots;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> PaintMaterial;
};
