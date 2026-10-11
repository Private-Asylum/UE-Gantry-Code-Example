// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Train/RailheadCaboose.h"

#include "RailheadLogChannels.h"
#include "Settings/RailheadLiverySettings.h"

#include "Experience/GantryExperienceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadCaboose)

ARailheadCaboose::ARailheadCaboose()
{
	Length = 900.0f;
	Body = ERailheadCarBody::Box;
	bTailCar = true;
	DeckColor = FLinearColor(0.06f, 0.06f, 0.065f);
	BodyColor = FLinearColor(0.45f, 0.05f, 0.03f);	// until BeginPlay reads the livery
}

void ARailheadCaboose::BuildBody()
{
	using namespace RailheadTrain;
	Super::BuildBody();

	const double Half = Length * 0.5;
	AddPart(TEXT("Cupola"), CubeMesh, FVector(-Half * 0.2, 0.0, DeckTop + 320.0), FVector(260.0, 200.0, 90.0), BodyColor, true);
	AddPart(TEXT("CupolaRoof"), CubeMesh, FVector(-Half * 0.2, 0.0, DeckTop + 370.0), FVector(290.0, 230.0, 12.0), FLinearColor(0.02f, 0.02f, 0.025f));
	AddPart(TEXT("TailLamp"), CubeMesh, FVector(-Half - 10.0, 0.0, DeckTop + 200.0), FVector(20.0, 30.0, 30.0), FLinearColor(1.0f, 0.05f, 0.02f), false, FRotator::ZeroRotator, 8.0f);
}

void ARailheadCaboose::BeginPlay()
{
	Super::BeginPlay();

	ApplyLivery();
	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		ReadyHandle = Experiences->OnExperienceReadyNative_LowPriority.AddUObject(this, &ARailheadCaboose::ApplyLivery);
	}
}

void ARailheadCaboose::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		Experiences->OnExperienceReadyNative_LowPriority.Remove(ReadyHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ARailheadCaboose::ApplyLivery(const UGantryExperienceDefinition* /*Experience*/)
{
	const FLinearColor Livery = URailheadLiverySettings::GetCurrent()->Caboose;
	if (!Livery.Equals(BodyColor))
	{
		PaintBody(Livery);
		UE_LOG(LogRailhead, Log, TEXT("Caboose: painted in the livery, %s."), *Livery.ToString());
	}
}
