// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "UI/RailheadLoadFeed.h"

#include "GantryLoadingTypes.h"
#include "GantryStreamingSubsystem.h"
#include "Misc/Paths.h"
#include "Mods/GantryModSubsystem.h"
#include "Tags/GantryGaugeTags.h"

FRailheadLoadFeed::~FRailheadLoadFeed()
{
	if (UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get())
	{
		Streaming->StopLoadRecording(Recording);
	}
}

void FRailheadLoadFeed::Begin(double NowSeconds)
{
	OpenedAt = NowSeconds;

	if (UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get())
	{
		Recording = Streaming->StartLoadRecording();
	}

	// The mods first: one line per unit, with its paks ("loose files" when read unpacked).
	const UGantryModSubsystem* Mods = UGantryModSubsystem::Get();
	const TArray<FGantryModUnit> Units = Mods ? Mods->GetUnits() : TArray<FGantryModUnit>();
	Counts.Mods = Units.Num();
	ModLines.Add(FLine{ FString::Printf(TEXT("Mods mounted  %d"), Units.Num()), 0, FGantryHandle() });
	for (const FGantryModUnit& Unit : Units)
	{
		TArray<FString> Paks;
		for (const FString& Container : Unit.Containers)
		{
			Paks.Add(FPaths::GetCleanFilename(Container));
		}
		Counts.Paks += Paks.Num();
		const FString PakText = Paks.IsEmpty() ? TEXT("loose files") : FString::Join(Paks, TEXT(", "));
		ModPaks.Add(Unit.Name, PakText);
		ModLines.Add(FLine{ FString::Printf(TEXT("Mod  %s   %s"), *Unit.Name, *PakText), 1, FGantryHandle() });
	}
	Lines = ModLines;
}

void FRailheadLoadFeed::Collect()
{
	UGantryStreamingSubsystem* Streaming = UGantryStreamingSubsystem::Get();
	TArray<FGantryRecordedScope> Scopes;
	if (!Streaming || !Streaming->GetLoadRecording(Recording, Scopes))
	{
		return;
	}

	// Laid out afresh each time, from the recording's tree order. Lines shown so far stay shown;
	// a line placed above them (a late asset under an early stage) simply appears.
	Lines = ModLines;
	Counts.Experiences = Counts.Stages = Counts.Features = Counts.Assets = 0;

	// Only what the player waits on: user-facing scopes, not startup work. A hidden scope's
	// children move up into its place.
	TArray<bool> HiddenAt;
	for (const FGantryRecordedScope& Each : Scopes)
	{
		const FGantryLoadScopeInfo& Scope = Each.Scope;
		HiddenAt.SetNum(Each.Depth + 1);
		HiddenAt[Each.Depth] = !Scope.bUserFacing || Scope.ReasonTag == GantryGaugeTags::Load_Scope_Bootstrap;
		if (HiddenAt[Each.Depth])
		{
			continue;
		}

		int32 Depth = Each.Depth;
		for (int32 Above = 0; Above < Each.Depth; ++Above)
		{
			Depth -= HiddenAt[Above] ? 1 : 0;
		}

		const bool bFeature = Scope.ReasonTag == GantryGaugeTags::Load_Scope_GameFeature;
		const TCHAR* Label = bFeature ? TEXT("Feature  ") : Depth == 0 ? TEXT("> ") : Depth == 1 ? TEXT("Experience  ") : Depth == 2 ? TEXT("Stage  ") : TEXT("");
		Lines.Add(FLine{ FString(Label) + Scope.DebugName, Depth, Scope.Handle });
		Counts.Features += bFeature ? 1 : 0;
		Counts.Experiences += (Depth == 1 && !bFeature) ? 1 : 0;
		Counts.Stages += (Depth == 2 && !bFeature) ? 1 : 0;

		for (const FGantryTrackedAsset& Asset : Each.Assets)
		{
			Lines.Add(FLine{ FString::Printf(TEXT("- %s   %s"), *Asset.GetDisplayName(), *Origin(Asset.Path.ToString())), Depth + 1, Scope.Handle });
			++Counts.Assets;
		}
	}
}

FString FRailheadLoadFeed::Origin(const FString& Path) const
{
	// The content root is the path's first folder.
	FString Root;
	FString Rest;
	if (!Path.RightChop(1).Split(TEXT("/"), &Root, &Rest))
	{
		Root = Path;
	}
	if (Root == TEXT("Game"))
	{
		return TEXT("game");
	}
	if (Root == TEXT("Engine"))
	{
		return TEXT("engine");
	}
	if (const FString* Paks = ModPaks.Find(Root))
	{
		return FString::Printf(TEXT("mod %s, %s"), *Root, **Paks);
	}
	return FString::Printf(TEXT("plugin %s"), *Root);
}

void FRailheadLoadFeed::Step(double NowSeconds)
{
	// Lines appear in proportion to the time passed, so everything collected is spread over
	// MinimumSeconds, and lines collected late appear as soon as they can.
	const double Fraction = (NowSeconds - OpenedAt) / FMath::Max(0.1, MinimumSeconds);
	const int32 Due = FMath::CeilToInt32(Lines.Num() * Fraction);
	Shown = FMath::Min(Lines.Num(), FMath::Max(Shown, Due));
}

bool FRailheadLoadFeed::IsDone(bool bActivationOver, double NowSeconds) const
{
	return bActivationOver && Shown >= Lines.Num() && NowSeconds - OpenedAt >= MinimumSeconds;
}

FString FRailheadLoadFeed::GetNewestShown() const
{
	const int32 Newest = FMath::Min(Shown, Lines.Num()) - 1;
	return Newest >= 0 ? Lines[Newest].Text.TrimStartAndEnd() : FString();
}

float FRailheadLoadFeed::GetShownFraction() const
{
	return Lines.IsEmpty() ? 0.0f : static_cast<float>(Shown) / static_cast<float>(Lines.Num());
}
