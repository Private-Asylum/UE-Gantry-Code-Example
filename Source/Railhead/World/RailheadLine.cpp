// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "World/RailheadLine.h"

#include "RailheadConsoleVariables.h"
#include "RailheadLogChannels.h"
#include "World/RailheadSceneryRegistry.h"
#include "World/RailheadLineMessage.h"
#include "World/RailheadLineRules.h"
#include "RailheadTags.h"
#include "Messages/GantryMessageSubsystem.h"
#include "Train/RailheadTrain.h"


#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Tasks/Task.h"
#include "EngineUtils.h"
#include "Experience/GantryExperienceSubsystem.h"
#include "GantrySetting.h"
#include "GantrySettingsRegistry.h"
#include "GantrySettingsSubsystem.h"
#include "GantryStreamingSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadLine)

CSV_DEFINE_CATEGORY(Railhead, true);

namespace RailheadLine
{
	/** How far behind and ahead of the train anything is drawn, in metres. Haze hides the edge. */
	constexpr double WindowBehind = 300.0;
	constexpr double WindowAhead = 1400.0;

	/** The scenery in view is placed again each time the train covers this many metres. */
	constexpr double PlacementStep = 25.0;

	/** Where spare instances wait, unseen: far below the ground, and too small to draw. */
	const FTransform Parked(FQuat::Identity, FVector(0.0, 0.0, -100000.0), FVector(0.001));

	/** Sleeper pitch in metres. */
	constexpr double SleeperPitch = 0.65;

	/** The first stretch starts just outside a tunnel this long, the way every later one does. */
	constexpr double OpeningTunnelLength = 400.0;
	constexpr double OpeningStretchBehind = 150.0;

	/** Metres of tunnel between the front of the train and the exit, once the exit opens. */
	constexpr double ExitLead = 80.0;

	/** How quickly a client's distance closes on the server's, per second, and when it gives up and jumps. */
	constexpr double FollowRate = 2.0;
	constexpr double FollowSnap = 60.0;

	/** Seconds to ease from one stretch's light to the next. */
	constexpr float LightEaseSeconds = 4.0f;

	/** Centimetres per metre, spelled out where the two meet. */
	constexpr double Cm = 100.0;

	/** What snow whitens paint toward: not quite white, so the shapes still read. */
	const FLinearColor SnowColor(0.86f, 0.88f, 0.92f);

	/** Lerp for the light's colours and numbers. */
	FRailheadStretchLight LerpLight(const FRailheadStretchLight& A, const FRailheadStretchLight& B, float Alpha)
	{
		FRailheadStretchLight Out;
		Out.SunElevation = FMath::Lerp(A.SunElevation, B.SunElevation, Alpha);
		Out.SunHeading = FMath::Lerp(A.SunHeading, B.SunHeading, Alpha);
		Out.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
		Out.SunLux = FMath::Lerp(A.SunLux, B.SunLux, Alpha);
		Out.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
		Out.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
		return Out;
	}
}

ARailheadLine::ARailheadLine()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(10.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Paint(TEXT("/Game/Railhead/Materials/M_Railhead_Paint.M_Railhead_Paint"));
	CubeMesh = Cube.Object;
	PaintMaterial = Paint.Object;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ScrollRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ScrollRoot"));
	ScrollRoot->SetupAttachment(Root);

	auto MakeTrackPiece = [this](const TCHAR* Name, const FVector& CentreCm, const FVector& Scale)
	{
		UStaticMeshComponent* Piece = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Piece->SetupAttachment(Root);
		Piece->SetStaticMesh(CubeMesh);
		Piece->SetRelativeLocation(CentreCm);
		Piece->SetRelativeScale3D(Scale);
		Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return Piece;
	};

	// Standard gauge: rail centres 1.5 m apart, rail tops at zero, where the train's wheels sit.
	const double Length = RailheadLine::WindowBehind + RailheadLine::WindowAhead;
	const double Middle = (RailheadLine::WindowAhead - RailheadLine::WindowBehind) * 0.5 * RailheadLine::Cm;
	RailLeft = MakeTrackPiece(TEXT("RailLeft"), FVector(Middle, -75.0, -7.5), FVector(Length, 0.07, 0.15));
	RailRight = MakeTrackPiece(TEXT("RailRight"), FVector(Middle, 75.0, -7.5), FVector(Length, 0.07, 0.15));
	Ballast = MakeTrackPiece(TEXT("Ballast"), FVector(Middle, 0.0, -45.0), FVector(Length, 4.0, 0.3));

	Sleepers = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Sleepers"));
	Sleepers->SetupAttachment(Root);
	Sleepers->SetStaticMesh(CubeMesh);
	Sleepers->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

ARailheadLine* ARailheadLine::Find(const UWorld* World)
{
	// Asked several times a frame (HUD, train, crew): remember the last answer per world.
	static TWeakObjectPtr<ARailheadLine> Cached;
	if (ARailheadLine* Line = Cached.Get(); Line && Line->GetWorld() == World)
	{
		return Line;
	}
	if (World)
	{
		// The first one there is: the game mode places exactly one.
		if (TActorIterator<ARailheadLine> It(const_cast<UWorld*>(World)); It)
		{
			Cached = *It;
			return *It;
		}
	}
	return nullptr;
}

void ARailheadLine::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	if (HasAuthority())
	{
		if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
		{
			// The high tier: the line is what everything else in the stretch runs on.
			ReadyHandle = Experiences->OnExperienceReadyNative_HighPriority.AddUObject(this, &ARailheadLine::HandleExperienceReady);
			Experiences->OnExperienceFailed.AddDynamic(this, &ARailheadLine::HandleExperienceFailed);

			// Already up (a stretch that activated before the line began play): start from it.
			if (Experiences->IsExperienceReady())
			{
				HandleExperienceReady(Experiences->GetCurrentExperience());
			}
		}
	}

	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	auto Paint = [this](const FLinearColor& Color)
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(PaintMaterial, this);
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		AddSnowPaint(Mid, Color);
		return Mid;
	};
	RailLeft->SetMaterial(0, Paint(FLinearColor(0.32f, 0.3f, 0.28f)));
	RailRight->SetMaterial(0, RailLeft->GetMaterial(0));
	Ballast->SetMaterial(0, Paint(FLinearColor(0.16f, 0.15f, 0.14f)));
	Sleepers->SetMaterial(0, Paint(FLinearColor(0.12f, 0.08f, 0.05f)));

	TArray<FTransform> SleeperTransforms;
	for (double X = -RailheadLine::WindowBehind; X < RailheadLine::WindowAhead; X += RailheadLine::SleeperPitch)
	{
		SleeperTransforms.Emplace(FRotator::ZeroRotator, FVector(X * RailheadLine::Cm, 0.0, -22.0), FVector(0.25, 2.6, 0.15));
	}
	Sleepers->AddInstances(SleeperTransforms, false);

	// The map's sun and haze: the first of each.
	if (TActorIterator<ADirectionalLight> It(World); It)
	{
		Sun = *It;
	}
	if (TActorIterator<AExponentialHeightFog> It(World); It)
	{
		Fog = *It;
	}
}

void ARailheadLine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// A placement still being worked out is waited for and dropped.
	if (PendingPlacement.IsSet())
	{
		PendingPlacement->Wait();
		PendingPlacement.Reset();
	}

	if (UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		Experiences->OnExperienceReadyNative_HighPriority.Remove(ReadyHandle);
		Experiences->OnExperienceFailed.RemoveDynamic(this, &ARailheadLine::HandleExperienceFailed);
	}

	Super::EndPlay(EndPlayReason);
}

void ARailheadLine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARailheadLine, State);
	DOREPLIFETIME(ARailheadLine, Motion);
}

const URailheadStretchDefinition* ARailheadLine::FindStretch(const FGameplayTag& Tag) const
{
	const URailheadSceneryRegistry* Registry = URailheadSceneryRegistry::Get(GetWorld());
	return Registry ? Registry->FindStretch(Tag) : nullptr;
}

double ARailheadLine::StretchLength(const URailheadStretchDefinition* Stretch)
{
	const double Scale = FMath::Max(0.05f, RailheadCVars::StretchLengthScale.GetValueOnGameThread());
	return FMath::Max(100.0, Stretch->LengthMeters * Scale);
}

float ARailheadLine::GetMetersToTunnel() const
{
	if (State.Phase != ERailheadLinePhase::Running)
	{
		return 0.0f;
	}
	return static_cast<float>(FMath::Max(0.0, State.StretchEnd - (LocalDistance + GetFrontReach())));
}

double ARailheadLine::GetFrontReach() const
{
	const ARailheadTrain* Train = ARailheadTrain::Find(GetWorld());
	return Train ? Train->GetFrontMeters() : TrainHalfLengthMeters;
}

double ARailheadLine::GetRearReach() const
{
	const ARailheadTrain* Train = ARailheadTrain::Find(GetWorld());
	return Train ? Train->GetRearMeters() : TrainHalfLengthMeters;
}

void ARailheadLine::AddWeather(UObject* Source, const FRailheadWeather& Weather)
{
	if (Source)
	{
		Weathers.Add(Source, Weather);
		bLightSettled = false;
	}
}

void ARailheadLine::RemoveWeather(UObject* Source)
{
	if (Weathers.Remove(Source) > 0)
	{
		bLightSettled = false;
	}
}

void ARailheadLine::AddSnowPaint(UMaterialInstanceDynamic* Paint, const FLinearColor& Color)
{
	SnowPaints.Emplace(Paint, Color);
	if (CurrentSnow > 0.0f)
	{
		Paint->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(Color, RailheadLine::SnowColor, CurrentSnow));
	}
}

bool ARailheadLine::IsTrainInTunnel() const
{
	const double Front = LocalDistance + GetFrontReach();
	const double Rear = LocalDistance - GetRearReach();
	const bool bInBehind = State.Phase != ERailheadLinePhase::Waiting && Rear < State.StretchStart && Front > State.PreviousEnd;
	const bool bInAhead = State.Phase != ERailheadLinePhase::Waiting && Front > State.StretchEnd;
	return bInBehind || bInAhead;
}

void ARailheadLine::HandleExperienceReady(const UGantryExperienceDefinition* Experience)
{
	const URailheadStretchDefinition* Stretch = Cast<URailheadStretchDefinition>(Experience);
	if (!Stretch)
	{
		return;
	}

	// The nearest rules the chain brings: the stretch's, its route's, or the base's.
	if (const UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this))
	{
		if (const URailheadLineRules* Rules = Cast<URailheadLineRules>(Experiences->FindExperienceData(URailheadLineRules::StaticClass())))
		{
			Acceleration = Rules->Acceleration;
			TunnelCrawlFraction = Rules->TunnelCrawlFraction;
		}
	}

	if (State.Phase == ERailheadLinePhase::Waiting)
	{
		// The session opens just outside a tunnel, the way every later stretch does.
		State.Stretch = Stretch->ExperienceTag;
		State.StretchStart = LocalDistance - RailheadLine::OpeningStretchBehind;
		State.StretchEnd = State.StretchStart + StretchLength(Stretch);
		State.PreviousStretch = FGameplayTag();
		State.PreviousEnd = State.StretchStart - RailheadLine::OpeningTunnelLength;
		State.Phase = ERailheadLinePhase::Running;
		UE_LOG(LogRailhead, Log, TEXT("Line: starting in %s (%.0f m to the first tunnel)."), *Stretch->GetDisplayName().ToString(), State.StretchEnd - LocalDistance);
		return;
	}

	if (State.Phase == ERailheadLinePhase::Tunnel && Stretch->ExperienceTag == State.NextStretch)
	{
		bNextReady = true;
		UE_LOG(LogRailhead, Log, TEXT("Line: %s is ready after %.1f s in the tunnel; the train had slowed to %.0f km/h."), *Stretch->GetDisplayName().ToString(),
			GetWorld()->GetTimeSeconds() - TunnelEnteredSeconds, LocalSpeed * 3.6f);
	}
}

void ARailheadLine::HandleExperienceFailed(FGameplayTag ExperienceTag, const FString& Reason)
{
	if (State.Phase != ERailheadLinePhase::Tunnel || ExperienceTag != State.NextStretch)
	{
		return;
	}

	// Gantry keeps the running stretch when a swap is refused, so the tunnel leads back into it.
	UE_LOG(LogRailhead, Warning, TEXT("Line: %s could not be activated (%s); the tunnel comes out where it went in."), *ExperienceTag.ToString(), *Reason);
	State.NextStretch = State.Stretch;
	bNextReady = true;
}

void ARailheadLine::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		TickServer(DeltaSeconds);
	}
	else
	{
		TickFollow(DeltaSeconds);
	}

	if (!IsNetMode(NM_DedicatedServer))
	{
		TickPresentation(DeltaSeconds);
	}
}

void ARailheadLine::TickServer(float DeltaSeconds)
{
	if (State.Phase == ERailheadLinePhase::Waiting)
	{
		return;
	}

	UWorld* World = GetWorld();
	const double Now = World->GetTimeSeconds();
	ARailheadTrain* Train = ARailheadTrain::Find(World);
	UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
	if (!Experiences)
	{
		return;
	}

	// The boiler sets the pace; a tunnel whose far side is still loading slows the train to match.
	float Target = Train ? Train->GetBoilerSpeed() : 0.0f;
	if (State.Phase == ERailheadLinePhase::Tunnel && !bNextReady)
	{
		UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
		const float Progress = Streaming ? Streaming->GetOverallProgress() : 1.0f;
		Target *= FMath::Lerp(TunnelCrawlFraction, 1.0f, Progress);
	}

	// Then anything listening (the weather, a mod) has its say.
	FRailheadLineSpeed Speed;
	Speed.TargetSpeed = Target;
	Speed.Stretch = State.Stretch;
	Speed.bInTunnel = State.Phase == ERailheadLinePhase::Tunnel;
	Gantry::RunHook(this, RailheadTags::Hook_Line_Speed, Speed);
	Target = FMath::Max(0.0f, Speed.TargetSpeed) * FMath::Max(0.0f, RailheadCVars::SpeedScale.GetValueOnGameThread());

	LocalSpeed = FMath::FInterpConstantTo(LocalSpeed, Target, DeltaSeconds, Acceleration * FMath::Max(1.0f, RailheadCVars::SpeedScale.GetValueOnGameThread()));
	LocalDistance += LocalSpeed * DeltaSeconds;

	const double Front = LocalDistance + GetFrontReach();
	const double Rear = LocalDistance - GetRearReach();

	switch (State.Phase)
	{
	case ERailheadLinePhase::Running:
		// The whole train is inside: ask Gantry for the next stretch.
		if (Rear > State.StretchEnd)
		{
			const URailheadStretchDefinition* Leaving = Cast<URailheadStretchDefinition>(Experiences->GetCurrentExperience());
			State.NextStretch = Leaving && Leaving->NextStretch.IsValid() ? Leaving->NextStretch : State.Stretch;
			State.Phase = ERailheadLinePhase::Tunnel;
			bNextReady = false;
			TunnelEnteredSeconds = Now;
			TunnelMinimumSeconds = Leaving ? Leaving->MinimumTunnelSeconds : 0.0;
			UE_LOG(LogRailhead, Log, TEXT("Line: into the tunnel after %s; asking for %s."), *State.Stretch.ToString(), *State.NextStretch.ToString());
			Experiences->SetExperienceByTag(State.NextStretch);
		}
		break;

	case ERailheadLinePhase::Tunnel:
	{
		// The experience being activated is already the current one, ready or not.
		const URailheadStretchDefinition* Current = Cast<URailheadStretchDefinition>(Experiences->GetCurrentExperience());
		if (bNextReady && Current && Now - TunnelEnteredSeconds >= TunnelMinimumSeconds)
		{
			// Open the exit a little way ahead, and the next stretch begins there.
			State.PreviousStretch = State.Stretch;
			State.PreviousEnd = State.StretchEnd;
			State.Stretch = State.NextStretch;
			State.NextStretch = FGameplayTag();
			State.StretchStart = Front + RailheadLine::ExitLead;
			State.StretchEnd = State.StretchStart + StretchLength(Current);
			State.Phase = ERailheadLinePhase::Leaving;
			UE_LOG(LogRailhead, Log, TEXT("Line: tunnel exit opens into %s after %.1f s."), *Current->GetDisplayName().ToString(), Now - TunnelEnteredSeconds);
		}
		break;
	}

	case ERailheadLinePhase::Leaving:
		if (Rear > State.StretchStart)
		{
			State.Phase = ERailheadLinePhase::Running;
		}
		break;

	default:
		break;
	}

	Motion.Distance = LocalDistance;
	Motion.Speed = LocalSpeed;
}

void ARailheadLine::TickFollow(float DeltaSeconds)
{
	const double Now = GetWorld()->GetTimeSeconds();
	const double Predicted = ReceivedMotion.Distance + ReceivedMotion.Speed * (Now - ReceivedAtSeconds);

	LocalSpeed = ReceivedMotion.Speed;
	LocalDistance += LocalSpeed * DeltaSeconds;

	const double Error = Predicted - LocalDistance;
	if (FMath::Abs(Error) > RailheadLine::FollowSnap)
	{
		LocalDistance = Predicted;
	}
	else
	{
		LocalDistance += Error * FMath::Min(1.0, DeltaSeconds * RailheadLine::FollowRate);
	}
}

void ARailheadLine::OnRep_State()
{
	// Nothing to do here: every frame compares each chunk's key with what the state now says.
}

void ARailheadLine::OnRep_Motion()
{
	ReceivedMotion = Motion;
	ReceivedAtSeconds = GetWorld()->GetTimeSeconds();
}


void ARailheadLine::TickPresentation(float DeltaSeconds)
{
	using namespace RailheadLine;
	CSV_SCOPED_TIMING_STAT(Railhead, LinePresentation);

	// Sleepers repeat every pitch, so they only ever move by less than one.
	Sleepers->SetRelativeLocation(FVector(-FMath::Fmod(LocalDistance, SleeperPitch) * Cm, 0.0, 0.0));

	UpdateScenery();
	ScrollRoot->SetRelativeLocation(FVector((ScrollAnchor - LocalDistance) * Cm, 0.0, 0.0));

	Announce();

	// Tunnel lamps waver while this machine is still loading what lies beyond. The paint is only
	// touched when its glow actually changes.
	if (LampPaint)
	{
		UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
		// The Coast Line's own settings page (added by its experience) can keep the lamps steady.
		const UGantrySettingsRegistry* Registry = UGantrySettingsSubsystem::GetRegistry(GetWorld()->GetFirstLocalPlayerFromController());
		const UGantrySettingToggle* FlickerSetting = Registry ? Cast<UGantrySettingToggle>(Registry->FindSetting(TEXT("Coast.LampsFlicker"))) : nullptr;
		const bool bLoading = Streaming && Streaming->IsLoading() && (!FlickerSetting || FlickerSetting->IsOn());
		const double Time = GetWorld()->GetTimeSeconds();
		const float Flicker = bLoading ? FMath::Clamp(0.35f + 1.3f * FMath::Abs(FMath::PerlinNoise1D(static_cast<float>(Time * 7.0))), 0.1f, 1.0f) : 1.0f;
		const float Glow = FMath::RoundToFloat(25.0f * Flicker * 4.0f) / 4.0f;
		if (Glow != LampGlow)
		{
			LampGlow = Glow;
			LampPaint->SetScalarParameterValue(TEXT("Glow"), Glow);
		}
	}

	// Ease toward the light of the stretch the train is in, then leave the sun and haze alone:
	// every change to them is work for the renderer, and most frames there is none to do.
	const URailheadStretchDefinition* Stretch = FindStretch(State.Stretch);
	if (!Stretch)
	{
		return;
	}
	if (State.Stretch != LightStretch)
	{
		LightStretch = State.Stretch;
		bLightSettled = false;
	}
	if (bLightSettled)
	{
		return;
	}

	// The stretch's light with the weather laid over it.
	FRailheadStretchLight Target = Stretch->Light;
	float TargetSnow = 0.0f;
	for (auto It = Weathers.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
			continue;
		}
		const FRailheadWeather& Weather = It.Value();
		Target.SunLux *= Weather.SunScale;
		Target.SunColor *= Weather.SunTint;
		Target.FogColor = FMath::Lerp(Target.FogColor, Weather.FogColor, Weather.FogBlend);
		Target.FogDensity += Weather.FogDensityAdd;
		TargetSnow = FMath::Max(TargetSnow, Weather.SnowCover);
	}

	const float Alpha = bHasLight ? FMath::Clamp(DeltaSeconds / LightEaseSeconds, 0.0f, 1.0f) : 1.0f;
	CurrentLight = LerpLight(CurrentLight, Target, Alpha);
	const float Snow = FMath::Lerp(CurrentSnow, TargetSnow, Alpha);
	bHasLight = true;
	if (FMath::IsNearlyEqual(CurrentLight.SunElevation, Target.SunElevation, 0.05f) && FMath::IsNearlyEqual(CurrentLight.SunHeading, Target.SunHeading, 0.05f)
		&& CurrentLight.SunColor.Equals(Target.SunColor, 0.002f) && FMath::IsNearlyEqual(CurrentLight.SunLux, Target.SunLux, 0.01f)
		&& CurrentLight.FogColor.Equals(Target.FogColor, 0.002f) && FMath::IsNearlyEqual(CurrentLight.FogDensity, Target.FogDensity, 0.0001f)
		&& FMath::IsNearlyEqual(Snow, TargetSnow, 0.002f))
	{
		CurrentLight = Target;
		bLightSettled = true;
	}

	// Snow whitens every paint it may, from its own colour.
	if (!FMath::IsNearlyEqual(Snow, CurrentSnow, 0.0005f) || (bLightSettled && CurrentSnow != TargetSnow))
	{
		CurrentSnow = bLightSettled ? TargetSnow : Snow;
		for (const TPair<TWeakObjectPtr<UMaterialInstanceDynamic>, FLinearColor>& Each : SnowPaints)
		{
			if (UMaterialInstanceDynamic* Paint = Each.Key.Get())
			{
				Paint->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(Each.Value, RailheadLine::SnowColor, CurrentSnow));
			}
		}
	}

	if (ADirectionalLight* SunActor = Sun.Get())
	{
		SunActor->SetActorRotation(FRotator(-CurrentLight.SunElevation, CurrentLight.SunHeading, 0.0f));
		if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(SunActor->GetLightComponent()))
		{
			Light->SetLightColor(CurrentLight.SunColor);
			Light->SetIntensity(CurrentLight.SunLux);
		}
	}
	if (AExponentialHeightFog* FogActor = Fog.Get())
	{
		UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
		FogComponent->SetFogInscatteringColor(CurrentLight.FogColor);
		FogComponent->SetFogDensity(CurrentLight.FogDensity);
	}
}

TSharedPtr<const RailheadScenery::FStretchSpec> ARailheadLine::GetSpec(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return nullptr;
	}
	if (const TSharedPtr<const RailheadScenery::FStretchSpec>* Found = Specs.Find(Tag))
	{
		return *Found;
	}
	const URailheadStretchDefinition* Stretch = FindStretch(Tag);
	if (!Stretch)
	{
		return nullptr;
	}
	TSharedPtr<const RailheadScenery::FStretchSpec> Spec = RailheadScenery::MakeStretchSpec(*Stretch);
	Specs.Add(Tag, Spec);
	return Spec;
}

TArray<RailheadScenery::FRange> ARailheadLine::BuildRanges()
{
	// What lies behind, the tunnel the train came through, the stretch, and the tunnel at its end
	// (open-ended until its exit is decided).
	TArray<RailheadScenery::FRange> Ranges;
	if (State.Phase == ERailheadLinePhase::Waiting)
	{
		return Ranges;
	}
	Ranges.Add({ -DBL_MAX, State.PreviousEnd, false, GetSpec(State.PreviousStretch) });
	Ranges.Add({ State.PreviousEnd, State.StretchStart, true, nullptr });
	Ranges.Add({ State.StretchStart, State.StretchEnd, false, GetSpec(State.Stretch) });
	Ranges.Add({ State.StretchEnd, DBL_MAX, true, nullptr });
	return Ranges;
}

void ARailheadLine::UpdateScenery()
{
	using namespace RailheadLine;

	// New scenery arrived: keep only what the track can still show, and copy it afresh.
	if (URailheadSceneryRegistry* Registry = URailheadSceneryRegistry::Get(GetWorld()); Registry && Registry->GetVersion() != SpecsVersion)
	{
		FGameplayTagContainer Keep;
		for (const FGameplayTag& Tag : { State.Stretch, State.PreviousStretch, State.NextStretch })
		{
			if (Tag.IsValid())
			{
				Keep.AddTag(Tag);
			}
		}
		UGantryExperienceSubsystem* Experiences = UGantryExperienceSubsystem::Get(this);
		if (const UGantryExperienceDefinition* Current = Experiences ? Experiences->GetCurrentExperience() : nullptr)
		{
			Keep.AddTag(Current->ExperienceTag);
		}
		Registry->Prune(Keep);
		SpecsVersion = Registry->GetVersion();
		Specs.Reset();
	}

	// A finished placement goes in first, so the anchor and the transforms change in the same frame.
	if (PendingPlacement.IsSet() && PendingPlacement->IsCompleted())
	{
		ApplyPlacement(MoveTemp(PendingPlacement->GetResult()));
		PendingPlacement.Reset();
	}
	if (PendingPlacement.IsSet())
	{
		return;
	}

	// The next placement: when the train has covered PlacementStep, or the track has changed.
	const TArray<RailheadScenery::FRange> Ranges = BuildRanges();
	if (Ranges.IsEmpty())
	{
		return;
	}
	const uint32 TrackKey = HashCombine(RailheadScenery::TrackKey(Ranges), ::GetTypeHash(SpecsVersion));
	if (TrackKey == RequestedTrackKey && FMath::Abs(LocalDistance - RequestedAt) < PlacementStep)
	{
		return;
	}

	RailheadScenery::FPlacementRequest Request;
	Request.Start = FMath::RoundToDouble(LocalDistance - WindowBehind);
	Request.End = Request.Start + WindowBehind + WindowAhead;
	Request.Ranges = Ranges;
	Request.Cube = CubeMesh;
	Request.CubeBounds = CubeMesh ? CubeMesh->GetBoundingBox() : FBox(ForceInit);
	Request.Rock = RockColor;
	Request.Clearance = TrackClearanceMeters;
	Request.TrainHalfWidth = TrainHalfWidthMeters;
	Request.TrainHeight = TrainHeightMeters;
	RequestedAt = LocalDistance;
	RequestedTrackKey = TrackKey;
	// Background priority: the renderer's own parallel work shares these workers, and must never
	// queue behind scenery that can just as well land a frame later.
	PendingPlacement.Emplace(UE::Tasks::Launch(UE_SOURCE_LOCATION, [Request = MoveTemp(Request)]() { return RailheadScenery::Place(Request); },
		UE::Tasks::ETaskPriority::BackgroundNormal));
}

UMaterialInstanceDynamic* ARailheadLine::GetPaint(const RailheadScenery::FBatch& Batch)
{
	const FString Key = RailheadScenery::PaintKey(Batch);
	if (TObjectPtr<UMaterialInstanceDynamic>* Found = Paints.Find(Key))
	{
		return *Found;
	}
	UMaterialInstanceDynamic* Paint = UMaterialInstanceDynamic::Create(PaintMaterial, this);
	Paint->SetVectorParameterValue(TEXT("Color"), Batch.Color);
	Paint->SetScalarParameterValue(TEXT("Glow"), Batch.Glow);
	if (Batch.Glow <= 0.0f)
	{
		AddSnowPaint(Paint, Batch.Color);
	}
	Paints.Add(Key, Paint);
	return Paint;
}

UInstancedStaticMeshComponent* ARailheadLine::GetInstancer(const FString& Key, const RailheadScenery::FBatch& Batch)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* Found = Instancers.Find(Key))
	{
		return *Found;
	}

	UInstancedStaticMeshComponent* Instancer = NewObject<UInstancedStaticMeshComponent>(this, NAME_None, RF_Transient);
	Instancer->SetStaticMesh(Batch.Mesh.Get());
	Instancer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Scenery is not ground anyone walks on: keep every update away from the navigation system.
	Instancer->SetCanEverAffectNavigation(false);
	Instancer->SetCastShadow(Batch.bShadows);
	Instancer->SetMobility(EComponentMobility::Movable);
	Instancer->SetupAttachment(ScrollRoot);
	UMaterialInstanceDynamic* Paint = GetPaint(Batch);
	Instancer->SetMaterial(0, Paint);
	Instancer->RegisterComponent();
	if (Key == TEXT("Tunnel|Lamp"))
	{
		LampPaint = Paint;
		LampGlow = -1.0f;
	}
	Instancers.Add(Key, Instancer);
	return Instancer;
}

void ARailheadLine::ApplyPlacement(RailheadScenery::FPlacement&& Placement)
{
	using namespace RailheadLine;
	CSV_SCOPED_TIMING_STAT(Railhead, ApplyPlacement);
	CSV_EVENT(Railhead, TEXT("Placement"));

	ScrollAnchor = Placement.Start;

	// Every instancer showing something: write the new transforms over the old ones, grow only when
	// there are more than ever before, and park whatever is left over. The renderer hears about
	// changed transforms, nothing more.
	TSet<FString> Placed;
	for (TPair<FString, RailheadScenery::FBatch>& Pair : Placement.Batches)
	{
		if (!Pair.Value.Mesh.IsValid())
		{
			continue;
		}
		Placed.Add(Pair.Key);
		UInstancedStaticMeshComponent* Instancer = GetInstancer(Pair.Key, Pair.Value);
		const TArray<FTransform>& Transforms = Pair.Value.Transforms;
		const int32 Count = Transforms.Num();
		const int32 Capacity = Instancer->GetInstanceCount();
		if (const int32 Overwrite = FMath::Min(Count, Capacity); Overwrite > 0)
		{
			Instancer->BatchUpdateInstancesTransforms(0, TArrayView<const FTransform>(Transforms.GetData(), Overwrite), false, false, true);
		}
		if (Count > Capacity)
		{
			Instancer->AddInstances(TArray<FTransform>(Transforms.GetData() + Capacity, Count - Capacity), false, false, false);
		}
		int32& Shown = Showing.FindOrAdd(Pair.Key);
		if (Shown > Count)
		{
			TArray<FTransform> Park;
			Park.Init(Parked, Shown - Count);
			Instancer->BatchUpdateInstancesTransforms(Count, Park, false, false, true);
		}
		Shown = Count;
	}
	for (TPair<FString, int32>& Pair : Showing)
	{
		if (Pair.Value > 0 && !Placed.Contains(Pair.Key))
		{
			TArray<FTransform> Park;
			Park.Init(Parked, Pair.Value);
			Instancers[Pair.Key]->BatchUpdateInstancesTransforms(0, Park, false, false, true);
			Pair.Value = 0;
		}
	}

#if !UE_BUILD_SHIPPING
	for (const FString& Key : Placement.Crossings)
	{
		if (!ReportedOverlaps.Contains(Key))
		{
			ReportedOverlaps.Add(Key);
			UE_LOG(LogRailhead, Warning, TEXT("Line: %s crosses the track; the train will pass through it."), *Key);
		}
	}
#endif
}

void ARailheadLine::Announce()
{
	const bool bInTunnel = IsTrainInTunnel();
	const bool bNewDaylight = !bInTunnel && State.Phase != ERailheadLinePhase::Waiting && State.Stretch.IsValid() && State.Stretch != AnnouncedStretch;
	const bool bNewTunnel = bInTunnel && !bAnnouncedTunnel;
	bAnnouncedTunnel = bInTunnel;
	if (!bNewDaylight && !bNewTunnel)
	{
		return;
	}

	FRailheadLineMessage Message;
	Message.Stretch = State.Stretch;
	Message.NextStretch = State.NextStretch;
	UGantryMessageSubsystem* Messages = UGantryMessageSubsystem::Get(this);
	if (bNewDaylight)
	{
		AnnouncedStretch = State.Stretch;
		if (Messages)
		{
			Messages->BroadcastMessage(RailheadTags::Message_Line_Daylight, Message);
		}
	}
	else if (Messages)
	{
		Messages->BroadcastMessage(RailheadTags::Message_Line_Tunnel, Message);
	}
}
