// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Train/RailheadCar.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadCar)

namespace RailheadCar
{
	const FLinearColor Black(0.02f, 0.02f, 0.025f);
	const FLinearColor Iron(0.06f, 0.06f, 0.065f);
}

ARailheadCar::ARailheadCar()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	// Attachment to the train travels with movement replication.
	SetReplicatingMovement(true);
	SetNetUpdateFrequency(2.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/Railhead/Materials/M_Railhead_Paint.M_Railhead_Paint"));
	CubeMesh = Cube.Object;
	CylinderMesh = Cylinder.Object;
	PaintMaterial = Paint.Object;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Lamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(Root);
	Lamp->SetRelativeLocation(FVector(0.0, 0.0, RailheadTrain::DeckTop + 350.0));
	Lamp->SetIntensityUnits(ELightUnits::Candelas);
	Lamp->SetIntensity(60.0f);
	Lamp->SetAttenuationRadius(1200.0f);
	Lamp->SetLightColor(FLinearColor(1.0f, 0.78f, 0.5f));
	Lamp->SetCastShadows(false);
}

void ARailheadCar::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	using namespace RailheadTrain;
	ClearParts();

	const double Half = Length * 0.5;
	AddPart(TEXT("Deck"), CubeMesh, FVector(0.0, 0.0, DeckTop - 20.0), FVector(Length, Width, 40.0), DeckColor);
	AddPart(TEXT("Frame"), CubeMesh, FVector(0.0, 0.0, DeckTop - 65.0), FVector(Length - 100.0, 200.0, 50.0), RailheadCar::Iron);

	// Two axles at each end, wheels on the rails.
	int32 WheelIndex = 0;
	for (const double End : { Half - 220.0, Half - 400.0, -Half + 400.0, -Half + 220.0 })
	{
		for (const double Side : { -75.0, 75.0 })
		{
			UStaticMeshComponent* Wheel = AddPart(FString::Printf(TEXT("Wheel%d"), ++WheelIndex), CylinderMesh,
				FVector(End, Side, WheelRadius), FVector(WheelRadius * 2.0, WheelRadius * 2.0, 15.0), RailheadCar::Black, false, FRotator(0.0, 0.0, 90.0));
			Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// The plate over the gap to the car ahead, so nobody has to jump it.
	AddPart(TEXT("Plate"), CubeMesh, FVector(Half + Gap * 0.5, 0.0, DeckTop - 5.0), FVector(Gap + 20.0, 160.0, 10.0), RailheadCar::Iron);

	BuildBody();
}

void ARailheadCar::BuildBody()
{
	using namespace RailheadTrain;

	switch (Body)
	{
	case ERailheadCarBody::Open:
		AddPart(TEXT("Left"), CubeMesh, FVector(0.0, -Width * 0.5 + 8.0, DeckTop + 50.0), FVector(Length, 15.0, 100.0), BodyColor, true);
		AddPart(TEXT("Right"), CubeMesh, FVector(0.0, Width * 0.5 - 8.0, DeckTop + 50.0), FVector(Length, 15.0, 100.0), BodyColor, true);
		break;
	case ERailheadCarBody::Box:
		AddPart(TEXT("Left"), CubeMesh, FVector(0.0, -Width * 0.5 + 8.0, DeckTop + 130.0), FVector(Length, 15.0, 260.0), BodyColor, true);
		AddPart(TEXT("Right"), CubeMesh, FVector(0.0, Width * 0.5 - 8.0, DeckTop + 130.0), FVector(Length, 15.0, 260.0), BodyColor, true);
		AddPart(TEXT("Roof"), CubeMesh, FVector(0.0, 0.0, DeckTop + 268.0), FVector(Length + 20.0, Width + 30.0, 15.0), BodyColor, true);
		break;
	case ERailheadCarBody::Tank:
		// Along the right of the deck, so the walkway down the left stays clear.
		AddPart(TEXT("Tank"), CylinderMesh, FVector(0.0, 55.0, DeckTop + 100.0), FVector(170.0, 170.0, Length - 120.0), BodyColor, true, FRotator(90.0, 0.0, 0.0));
		break;
	case ERailheadCarBody::Flat:
		break;
	}
}

UStaticMeshComponent* ARailheadCar::AddPart(const FString& Name, UStaticMesh* Mesh, const FVector& Centre, const FVector& SizeCm, const FLinearColor& Color,
	bool bIsBody, const FRotator& Rotation, float Glow)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, *Name, RF_Transient);
	Part->SetupAttachment(Root);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Centre);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(SizeCm / 100.0);	// the basic shapes are a metre across
	Part->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Part->RegisterComponent();

	if (PaintMaterial)
	{
		if (UMaterialInstanceDynamic* Paint = Part->CreateDynamicMaterialInstance(0, PaintMaterial))
		{
			Paint->SetVectorParameterValue(TEXT("Color"), Color);
			Paint->SetScalarParameterValue(TEXT("Glow"), Glow);
			if (bIsBody)
			{
				BodyPaints.Add(Paint);
			}
		}
	}
	Parts.Add(Part);
	return Part;
}

void ARailheadCar::PaintBody(FLinearColor Color)
{
	BodyColor = Color;
	for (UMaterialInstanceDynamic* Paint : BodyPaints)
	{
		Paint->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ARailheadCar::ClearParts()
{
	for (UStaticMeshComponent* Part : Parts)
	{
		if (Part)
		{
			Part->DestroyComponent();
		}
	}
	Parts.Reset();
	BodyPaints.Reset();
}
