// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Character/RailheadCharacter.h"

#include "Character/RailheadCrewReadiness.h"
#include "Player/RailheadInputConfig.h"
#include "Player/RailheadPlayerController.h"
#include "Player/RailheadPlayerState.h"
#include "RailheadLogChannels.h"
#include "Train/RailheadTrain.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GantrySetting.h"
#include "GantrySettingsRegistry.h"
#include "GantrySettingsSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadCharacter)

ARailheadCharacter::ARailheadCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/Railhead/Materials/M_Railhead_Paint.M_Railhead_Paint"));

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 90.0f);

	Readiness = CreateDefaultSubobject<URailheadCrewReadiness>(TEXT("Readiness"));

	Eyes = CreateDefaultSubobject<UCameraComponent>(TEXT("Eyes"));
	Eyes->SetupAttachment(GetCapsuleComponent());
	Eyes->SetRelativeLocation(FVector(0.0, 0.0, 64.0));
	Eyes->bUsePawnControlRotation = true;
	Eyes->SetFieldOfView(95.0f);

	auto MakePiece = [&](const TCHAR* Name, USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
	{
		UStaticMeshComponent* Piece = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Piece->SetupAttachment(Parent);
		Piece->SetStaticMesh(Mesh);
		Piece->SetMaterial(0, Paint.Object);
		Piece->SetRelativeLocation(Location);
		Piece->SetRelativeScale3D(Scale);
		Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return Piece;
	};
	Body = MakePiece(TEXT("Body"), GetCapsuleComponent(), Cylinder.Object, FVector(0.0, 0.0, -10.0), FVector(0.6, 0.6, 1.5));
	Head = MakePiece(TEXT("Head"), GetCapsuleComponent(), Sphere.Object, FVector(0.0, 0.0, 70.0), FVector(0.4));
	Body->SetOwnerNoSee(true);
	Head->SetOwnerNoSee(true);

	CoalInView = MakePiece(TEXT("CoalInView"), Eyes, Cube.Object, FVector(45.0, 18.0, -25.0), FVector(0.22, 0.18, 0.12));
	CoalInView->SetOnlyOwnerSee(true);
	CoalInView->SetVisibility(false);
	CoalHeld = MakePiece(TEXT("CoalHeld"), GetCapsuleComponent(), Cube.Object, FVector(40.0, 0.0, 20.0), FVector(0.22, 0.18, 0.12));
	CoalHeld->SetOwnerNoSee(true);
	CoalHeld->SetVisibility(false);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 420.0f;
	Movement->JumpZVelocity = 380.0f;
	Movement->AirControl = 0.3f;

	bUseControllerRotationYaw = true;
}

void ARailheadCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARailheadCharacter, bCarryingCoal);
}

void ARailheadCharacter::BindCrewInput(const URailheadInputConfig& Config)
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		UE_LOG(LogRailhead, Warning, TEXT("Crew: no Enhanced Input component; this crew member cannot move."));
		return;
	}
	Input->BindAction(Config.Move, ETriggerEvent::Triggered, this, &ARailheadCharacter::Move);
	Input->BindAction(Config.Look, ETriggerEvent::Triggered, this, &ARailheadCharacter::Look);
	Input->BindAction(Config.Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
	Input->BindAction(Config.Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	Input->BindAction(Config.Interact, ETriggerEvent::Started, this, &ARailheadCharacter::Interact);
}

void ARailheadCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	Readiness->CheckInitState();
}

void ARailheadCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	Readiness->CheckInitState();
}

void ARailheadCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator Yaw(0.0, GetControlRotation().Yaw, 0.0);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

double ARailheadCharacter::CrewSetting(FName Id, double Fallback) const
{
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const UGantrySettingsRegistry* Registry = PC ? UGantrySettingsSubsystem::GetRegistry(PC->GetLocalPlayer()) : nullptr;
	const UGantrySetting* Setting = Registry ? Registry->FindSetting(Id) : nullptr;
	if (const UGantrySettingNumber* Number = Cast<UGantrySettingNumber>(Setting))
	{
		return Number->GetValue();
	}
	if (const UGantrySettingToggle* Toggle = Cast<UGantrySettingToggle>(Setting))
	{
		return Toggle->IsOn() ? 1.0 : 0.0;
	}
	return Fallback;
}

void ARailheadCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const double Sensitivity = CrewSetting(TEXT("Crew.LookSensitivity"), 1.0);
	const double Invert = CrewSetting(TEXT("Crew.InvertLook"), 0.0) > 0.5 ? -1.0 : 1.0;
	AddControllerYawInput(Axis.X * Sensitivity);
	AddControllerPitchInput(Axis.Y * Sensitivity * Invert);
}

void ARailheadCharacter::Interact()
{
	if (TraceForInteraction() != ERailheadInteraction::None)
	{
		ServerInteract();
	}
}

ERailheadInteraction ARailheadCharacter::TraceForInteraction() const
{
	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RailheadInteract), false, this);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, EyeLocation + EyeRotation.Vector() * Reach, ECC_Visibility, Params))
	{
		return ERailheadInteraction::None;
	}

	const UPrimitiveComponent* Component = Hit.GetComponent();
	if (!Component)
	{
		return ERailheadInteraction::None;
	}
	if (!bCarryingCoal && Component->ComponentHasTag(TEXT("CoalPile")))
	{
		return ERailheadInteraction::TakeCoal;
	}
	if (bCarryingCoal && Component->ComponentHasTag(TEXT("Firebox")))
	{
		return ERailheadInteraction::Stoke;
	}
	return ERailheadInteraction::None;
}

void ARailheadCharacter::ServerInteract_Implementation()
{
	switch (TraceForInteraction())
	{
	case ERailheadInteraction::TakeCoal:
		bCarryingCoal = true;
		OnRep_CarryingCoal();
		break;

	case ERailheadInteraction::Stoke:
		if (ARailheadTrain* Train = ARailheadTrain::Find(GetWorld()))
		{
			Train->Stoke();
		}
		if (ARailheadPlayerState* Crew = GetPlayerState<ARailheadPlayerState>())
		{
			++Crew->Shovels;
		}
		bCarryingCoal = false;
		OnRep_CarryingCoal();
		break;

	default:
		break;
	}
}

void ARailheadCharacter::OnRep_CarryingCoal()
{
	CoalInView->SetVisibility(bCarryingCoal);
	CoalHeld->SetVisibility(bCarryingCoal);
}

void ARailheadCharacter::ApplyCrewColor()
{
	const ARailheadPlayerState* Crew = GetPlayerState<ARailheadPlayerState>();
	if (!Crew)
	{
		return;
	}

	const auto Paint = [](UStaticMeshComponent* Piece, const FLinearColor& Color)
	{
		if (UMaterialInstanceDynamic* Mid = Piece->CreateDynamicMaterialInstance(0))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Color);
		}
	};
	Paint(Body, Crew->CrewColor);
	Paint(Head, FLinearColor(0.75f, 0.55f, 0.42f));
	Paint(CoalInView, FLinearColor(0.015f, 0.015f, 0.015f));
	Paint(CoalHeld, FLinearColor(0.015f, 0.015f, 0.015f));
	bColorApplied = true;
}

void ARailheadCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bColorApplied && !IsNetMode(NM_DedicatedServer))
	{
		ApplyCrewColor();
	}

	if (IsLocallyControlled())
	{
		Offer = TraceForInteraction();
		Eyes->SetFieldOfView(static_cast<float>(CrewSetting(TEXT("Crew.FieldOfView"), 95.0)));
	}

	// Off the train: the ground beside the track is scenery, not floor. Back aboard.
	if (HasAuthority() && GetActorLocation().Z < FallenHeight)
	{
		if (const ARailheadTrain* Train = ARailheadTrain::Find(GetWorld()))
		{
			UE_LOG(LogRailhead, Log, TEXT("Crew: %s fell off and is back aboard."), *GetNameSafe(GetPlayerState()));
			TeleportTo(Train->GetCrewSpot(FMath::Rand()).GetLocation(), GetActorRotation());
			GetCharacterMovement()->StopMovementImmediately();
		}
	}
}
