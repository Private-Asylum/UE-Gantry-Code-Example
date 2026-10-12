// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Train/RailheadTrain.h"

#include "Train/RailheadCar.h"

#include "World/RailheadLine.h"
#include "RailheadConsoleVariables.h"

#include "Algo/StableSort.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadTrain)

namespace RailheadTrain
{
	// Deck height, width, wheels and gaps are every car's: RailheadCar.h. Front to back, in
	// centimetres along X. The train is centred on the origin.
	constexpr double Front = 3390.0;
	constexpr double LocoLength = 1500.0;
	constexpr double TenderLength = 900.0;
	constexpr double WagonLength = 1300.0;

	const FLinearColor Black(0.02f, 0.02f, 0.025f);
	const FLinearColor Iron(0.06f, 0.06f, 0.065f);
	const FLinearColor Green(0.03f, 0.16f, 0.09f);
	const FLinearColor Red(0.45f, 0.05f, 0.03f);
	const FLinearColor Brass(0.6f, 0.42f, 0.12f);
	const FLinearColor Timber(0.22f, 0.12f, 0.06f);
	const FLinearColor Oxide(0.32f, 0.1f, 0.06f);
	const FLinearColor Coal(0.015f, 0.015f, 0.015f);
	const FLinearColor Fire(1.0f, 0.35f, 0.05f);
}

ARailheadTrain::ARailheadTrain()
{
	using namespace RailheadTrain;

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(4.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/Railhead/Materials/M_Railhead_Paint.M_Railhead_Paint"));
	CubeMesh = Cube.Object;
	CylinderMesh = Cylinder.Object;
	SphereMesh = Sphere.Object;
	PaintMaterial = Paint.Object;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	const FRotator AlongTrack(90.0, 0.0, 0.0);	// a cylinder's axis turned from Z to X
	const FRotator Across(0.0, 0.0, 90.0);		// a cylinder's axis turned from Z to Y

	// The locomotive: frame, boiler, smokebox, chimney, dome, cab, firebox and buffer beam.
	const double LocoRear = Front - LocoLength;
	AddWagonBase(TEXT("Loco"), LocoRear, Front, Iron);
	AddPart(TEXT("Boiler"), CylinderMesh, FVector(Front - 600.0, 0.0, DeckTop + 110.0), FVector(200.0, 200.0, 850.0), Green, AlongTrack);
	AddPart(TEXT("Smokebox"), CylinderMesh, FVector(Front - 120.0, 0.0, DeckTop + 110.0), FVector(210.0, 210.0, 120.0), Black, AlongTrack);
	AddPart(TEXT("Chimney"), CylinderMesh, FVector(Front - 220.0, 0.0, DeckTop + 260.0), FVector(55.0, 55.0, 120.0), Black);
	AddPart(TEXT("Dome"), SphereMesh, FVector(Front - 620.0, 0.0, DeckTop + 210.0), FVector(80.0, 80.0, 70.0), Brass);
	AddPart(TEXT("BufferBeam"), CubeMesh, FVector(Front + 20.0, 0.0, DeckTop - 40.0), FVector(40.0, Width, 50.0), Red);
	AddPart(TEXT("Lamp"), CubeMesh, FVector(Front - 40.0, 0.0, DeckTop + 230.0), FVector(30.0, 40.0, 40.0), FLinearColor(1.0f, 0.9f, 0.7f), FRotator::ZeroRotator, 10.0f);

	const double CabFront = LocoRear + 450.0;
	AddPart(TEXT("CabLeft"), CubeMesh, FVector(LocoRear + 225.0, -Width * 0.5 + 8.0, DeckTop + 130.0), FVector(450.0, 15.0, 260.0), Green);
	AddPart(TEXT("CabRight"), CubeMesh, FVector(LocoRear + 225.0, Width * 0.5 - 8.0, DeckTop + 130.0), FVector(450.0, 15.0, 260.0), Green);
	AddPart(TEXT("CabFront"), CubeMesh, FVector(CabFront, 0.0, DeckTop + 130.0), FVector(15.0, Width, 260.0), Green);
	AddPart(TEXT("CabRoof"), CubeMesh, FVector(LocoRear + 225.0, 0.0, DeckTop + 268.0), FVector(500.0, Width + 30.0, 15.0), Black);
	UStaticMeshComponent* Firebox = AddPart(TEXT("Firebox"), CubeMesh, FVector(CabFront - 40.0, 0.0, DeckTop + 70.0), FVector(60.0, 120.0, 90.0), Fire, FRotator::ZeroRotator, 4.0f);
	Firebox->ComponentTags.Add(TEXT("Firebox"));

	// The tender: low walls round a heap of coal.
	const double TenderFront = LocoRear - Gap;
	const double TenderRear = TenderFront - TenderLength;
	AddWagonBase(TEXT("Tender"), TenderRear, TenderFront, Iron);
	AddPart(TEXT("TenderLeft"), CubeMesh, FVector((TenderFront + TenderRear) * 0.5, -Width * 0.5 + 8.0, DeckTop + 60.0), FVector(TenderLength, 15.0, 120.0), Green);
	AddPart(TEXT("TenderRight"), CubeMesh, FVector((TenderFront + TenderRear) * 0.5, Width * 0.5 - 8.0, DeckTop + 60.0), FVector(TenderLength, 15.0, 120.0), Green);
	// The coal is heaped against the right wall, leaving a walkway down the left to the cab: the crew
	// spawn on the wagons behind, and the heap is taller than anyone can jump.
	UStaticMeshComponent* CoalPile = AddPart(TEXT("CoalPile"), CubeMesh, FVector((TenderFront + TenderRear) * 0.5, Width * 0.5 - 15.0 - 70.0, DeckTop + 45.0), FVector(TenderLength - 60.0, 140.0, 90.0), Coal);
	CoalPile->ComponentTags.Add(TEXT("CoalPile"));

	// Three open wagons, ends open so the crew can walk the length of the train.
	double WagonFront = TenderRear - Gap;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const double WagonRear = WagonFront - WagonLength;
		const double Middle = (WagonFront + WagonRear) * 0.5;
		const FString Name = FString::Printf(TEXT("Wagon%d"), Index + 1);
		AddWagonBase(*Name, WagonRear, WagonFront, Oxide);
		AddPart(*(Name + TEXT("Left")), CubeMesh, FVector(Middle, -Width * 0.5 + 8.0, DeckTop + 50.0), FVector(WagonLength, 15.0, 100.0), Timber);
		AddPart(*(Name + TEXT("Right")), CubeMesh, FVector(Middle, Width * 0.5 - 8.0, DeckTop + 50.0), FVector(WagonLength, 15.0, 100.0), Timber);
		UPointLightComponent* Lamp = CreateDefaultSubobject<UPointLightComponent>(*(Name + TEXT("Lamp")));
		Lamp->SetupAttachment(Root);
		Lamp->SetRelativeLocation(FVector(Middle, 0.0, DeckTop + 350.0));
		Lamp->SetIntensityUnits(ELightUnits::Candelas);
		Lamp->SetIntensity(60.0f);
		Lamp->SetAttenuationRadius(1200.0f);
		Lamp->SetLightColor(FLinearColor(1.0f, 0.78f, 0.5f));
		Lamp->SetCastShadows(false);
		WagonLamps.Add(Lamp);
		CrewSpots.Add(FVector(Middle + 300.0, -60.0, DeckTop + 100.0));
		CrewSpots.Add(FVector(Middle - 300.0, 60.0, DeckTop + 100.0));
		WagonFront = WagonRear - Gap;
	}
	TailX = WagonFront + Gap;

	// Plates over the gaps between cars, so nobody has to jump them.
	double PlateX = LocoRear - Gap * 0.5;
	for (int32 Index = 0; Index < 4; ++Index)
	{
		AddPart(*FString::Printf(TEXT("Plate%d"), Index + 1), CubeMesh, FVector(PlateX, 0.0, DeckTop - 5.0), FVector(Gap + 20.0, 160.0, 10.0), Iron);
		PlateX -= (Index == 0 ? TenderLength : WagonLength) + Gap;
	}

	FireboxLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireboxLight"));
	FireboxLight->SetupAttachment(Root);
	FireboxLight->SetRelativeLocation(FVector(CabFront - 120.0, 0.0, DeckTop + 90.0));
	FireboxLight->SetIntensityUnits(ELightUnits::Candelas);
	FireboxLight->SetIntensity(120.0f);
	FireboxLight->SetAttenuationRadius(900.0f);
	FireboxLight->SetLightColor(FLinearColor(1.0f, 0.45f, 0.15f));

	Headlamp = CreateDefaultSubobject<USpotLightComponent>(TEXT("Headlamp"));
	Headlamp->SetupAttachment(Root);
	Headlamp->SetRelativeLocation(FVector(Front, 0.0, DeckTop + 230.0));
	Headlamp->SetRelativeRotation(FRotator(-6.0, 0.0, 0.0));
	Headlamp->SetIntensityUnits(ELightUnits::Candelas);
	Headlamp->SetIntensity(40000.0f);
	Headlamp->SetAttenuationRadius(12000.0f);
	Headlamp->SetOuterConeAngle(28.0f);
	Headlamp->SetInnerConeAngle(12.0f);
	Headlamp->SetLightColor(FLinearColor(1.0f, 0.88f, 0.7f));
}

void ARailheadTrain::AddWagonBase(const TCHAR* Name, double RearX, double FrontX, const FLinearColor& Color)
{
	using namespace RailheadTrain;

	const double Length = FrontX - RearX;
	const double Middle = (FrontX + RearX) * 0.5;
	const FString Base(Name);

	AddPart(*(Base + TEXT("Deck")), CubeMesh, FVector(Middle, 0.0, DeckTop - 20.0), FVector(Length, Width, 40.0), Color);
	AddPart(*(Base + TEXT("Frame")), CubeMesh, FVector(Middle, 0.0, DeckTop - 65.0), FVector(Length - 100.0, 200.0, 50.0), Iron);

	// Two axles at each end, wheels on the rails.
	int32 WheelIndex = 0;
	for (const double End : { FrontX - 220.0, FrontX - 400.0, RearX + 400.0, RearX + 220.0 })
	{
		for (const double Side : { -75.0, 75.0 })
		{
			UStaticMeshComponent* Wheel = AddPart(*FString::Printf(TEXT("%sWheel%d"), Name, ++WheelIndex), CylinderMesh,
				FVector(End, Side, WheelRadius), FVector(WheelRadius * 2.0, WheelRadius * 2.0, 15.0), Black, FRotator(0.0, 0.0, 90.0));
			Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

UStaticMeshComponent* ARailheadTrain::AddPart(const TCHAR* Name, UStaticMesh* Mesh, const FVector& Centre, const FVector& SizeCm, const FLinearColor& Color, const FRotator& Rotation, float Glow)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(Root);
	Part->SetStaticMesh(Mesh);
	Part->SetMaterial(0, PaintMaterial);
	Part->SetRelativeLocation(Centre);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(SizeCm / 100.0);	// the basic shapes are a metre across
	Part->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Parts.Add(Part);
	PartColors.Add(Color);
	PartGlows.Add(Glow);
	return Part;
}

ARailheadTrain* ARailheadTrain::Find(const UWorld* World)
{
	// Asked several times a frame (HUD, line, crew): remember the last answer per world.
	static TWeakObjectPtr<ARailheadTrain> Cached;
	if (ARailheadTrain* Train = Cached.Get(); Train && Train->GetWorld() == World)
	{
		return Train;
	}
	if (World)
	{
		// The first one there is: the game mode places exactly one.
		if (TActorIterator<ARailheadTrain> It(const_cast<UWorld*>(World)); It)
		{
			Cached = *It;
			return *It;
		}
	}
	return nullptr;
}

void ARailheadTrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!PaintMaterial)
	{
		return;
	}

	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		if (UMaterialInstanceDynamic* Paint = Parts[Index]->CreateDynamicMaterialInstance(0, PaintMaterial))
		{
			Paint->SetVectorParameterValue(TEXT("Color"), PartColors[Index]);
			Paint->SetScalarParameterValue(TEXT("Glow"), PartGlows[Index]);
		}
	}
}

void ARailheadTrain::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Pressure = StartingPressure;
	}
}

void ARailheadTrain::CoupleCar(ARailheadCar* Car)
{
	if (!Car || Cars.Contains(Car))
	{
		return;
	}
	Cars.Add(Car);
	Car->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	LayOutCars();
}

void ARailheadTrain::UncoupleCar(ARailheadCar* Car)
{
	if (Cars.Remove(Car) > 0)
	{
		Car->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		LayOutCars();
	}
}

void ARailheadTrain::LayOutCars()
{
	Cars.RemoveAll([](const ARailheadCar* Car) { return !IsValid(Car); });

	// Coupling order, tail cars last (a stable sort keeps both groups in the order they came).
	TArray<ARailheadCar*> Order(Cars);
	Algo::StableSort(Order, [](const ARailheadCar* A, const ARailheadCar* B) { return !A->bTailCar && B->bTailCar; });

	double Cursor = TailX - RailheadTrain::Gap;
	for (ARailheadCar* Car : Order)
	{
		Car->SetActorRelativeLocation(FVector(Cursor - Car->GetLength() * 0.5, 0.0, 0.0));
		Car->SetActorRelativeRotation(FRotator::ZeroRotator);
		Cursor -= Car->GetLength() + RailheadTrain::Gap;
	}
}

TArray<ARailheadCar*> ARailheadTrain::GetCoupledCars() const
{
	// Attached cars, which every machine has (the server's list does not replicate), front to back.
	TArray<AActor*> Attached;
	GetAttachedActors(Attached);
	TArray<ARailheadCar*> Out;
	for (AActor* Actor : Attached)
	{
		if (ARailheadCar* Car = Cast<ARailheadCar>(Actor))
		{
			Out.Add(Car);
		}
	}
	Out.Sort([](const ARailheadCar& A, const ARailheadCar& B) { return A.GetRootComponent()->GetRelativeLocation().X > B.GetRootComponent()->GetRelativeLocation().X; });
	return Out;
}

float ARailheadTrain::GetFrontMeters() const
{
	return static_cast<float>(RailheadTrain::Front / 100.0);
}

float ARailheadTrain::GetRearMeters() const
{
	double Rear = TailX;
	for (const ARailheadCar* Car : GetCoupledCars())
	{
		Rear = FMath::Min(Rear, Car->GetRootComponent()->GetRelativeLocation().X - Car->GetLength() * 0.5);
	}
	return static_cast<float>(-Rear / 100.0);
}

void ARailheadTrain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARailheadTrain, Pressure);
}

float ARailheadTrain::GetBoilerSpeed() const
{
	// Little happens below a tenth of full pressure; most of the speed comes by three quarters.
	return TopSpeed * FMath::SmoothStep(0.08f, 0.75f, Pressure);
}

void ARailheadTrain::Stoke()
{
	Pressure = FMath::Min(1.0f, Pressure + ShovelPressure);
}

FTransform ARailheadTrain::GetCrewSpot(int32 Index) const
{
	const FVector Spot = CrewSpots.IsEmpty() ? FVector(0.0, 0.0, 300.0) : CrewSpots[FMath::Abs(Index) % CrewSpots.Num()];
	return FTransform(FRotator::ZeroRotator, GetActorTransform().TransformPosition(Spot));
}

void ARailheadTrain::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		const ARailheadLine* Line = ARailheadLine::Find(GetWorld());
		const float Speed = Line ? Line->GetSpeed() : 0.0f;

		const float Drain = IdleDrain + RunningDrain * FMath::Clamp(Speed / TopSpeed, 0.0f, 1.0f);
		Pressure = FMath::Max(0.0f, Pressure - Drain * DeltaSeconds);

		if (RailheadCVars::AutoStoke.GetValueOnGameThread() && Pressure < 0.6f)
		{
			Stoke();
		}
	}
}
