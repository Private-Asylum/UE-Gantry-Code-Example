// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "Settings/RailheadModsProvider.h"

#include "GantrySetting.h"
#include "GantrySettingsPage.h"
#include "GantrySettingsRegistry.h"
#include "Mods/GantryModSubsystem.h"

#include "Containers/Ticker.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RailheadModsProvider)

#define LOCTEXT_NAMESPACE "RailheadMods"

TArray<UGantrySettingsPage*> URailheadModsProvider::CreatePages_Implementation(UGantrySettingsRegistry* Registry)
{
	const ULocalPlayer* Player = Registry ? Registry->GetLocalPlayer() : nullptr;
	UGantryModSubsystem* Mods = UGantryModSubsystem::Get();
	if (!Mods || (Player && !Player->IsPrimaryPlayer()))
	{
		return {};
	}

	BuiltFor = Registry;
	Mods->OnModsReported.AddWeakLambda(this, [this]() { Rebuild(); });

	// Every mod found, refused ones included: a player should see why one did nothing.
	TArray<const FGantryModUnit*> Units;
	for (const FGantryModUnit& Unit : Mods->GetUnits())
	{
		if (!Unit.bAuthoringRoot)
		{
			Units.Add(&Unit);
		}
	}
	if (Units.IsEmpty())
	{
		return {};
	}
	Units.Sort([](const FGantryModUnit& A, const FGantryModUnit& B) { return A.Name < B.Name; });

	UGantrySettingsPage* Page = NewPage(Registry, LOCTEXT("Mods", "Mods"), 20);
	Page->Description = LOCTEXT("ModsDescription", "The mods in the game's Mods folder. Turned off, a mod stays where it is and is not loaded.");

	// What the player picked, until applied: toggling back and forth mounts nothing.
	TSharedRef<TMap<FString, bool>> Pending = MakeShared<TMap<FString, bool>>();
	for (const FGantryModUnit* Unit : Units)
	{
		const FString Name = Unit->Name;
		UGantrySettingToggle* Toggle = Page->AddSetting<UGantrySettingToggle>(FName(*(TEXT("Mods.") + Name)), FText::FromString(Name));
		Toggle->Description = Unit->State == EGantryModUnitState::Registered
			? FText::Format(LOCTEXT("Loaded", "Loaded. Content id {0}."), FText::FromString(Unit->ContentId))
			: Unit->Error.IsEmpty() ? LOCTEXT("NotLoaded", "Not loaded.") : FText::FromString(Unit->Error);
		Toggle->DefaultValue = TEXT("True");
		Toggle->BindValue(
			[Pending, Name]()
			{
				const bool* Picked = Pending->Find(Name);
				const UGantryModSubsystem* Mods = UGantryModSubsystem::Get();
				return LexToString(Picked ? *Picked : (Mods && Mods->IsModEnabled(Name)));
			},
			[Pending, Name](const FString& Value) { Pending->Add(Name, Value.ToBool()); });
		Toggle->BindApply([Pending, Name]()
		{
			bool bEnabled = true;
			if (Pending->RemoveAndCopyValue(Name, bEnabled))
			{
				if (UGantryModSubsystem* Mods = UGantryModSubsystem::Get())
				{
					FString Reason;
					Mods->SetModEnabled(Name, bEnabled, Reason);
				}
			}
		});
		if (Unit->State == EGantryModUnitState::Failed)
		{
			const FText Reason = FText::FromString(Unit->Error);
			Toggle->BindEditState([Reason](const UGantrySetting&, FGantrySettingEditState& State) { State.Disable(Reason); });
		}
	}
	return { Page };
}

void URailheadModsProvider::Rebuild()
{
	if (bRetired)
	{
		return;
	}
	bRetired = true;

	// Not from inside Gantry's report: the registry hears about pages changing, and so may the
	// mod subsystem's other listeners.
	TWeakObjectPtr<UGantrySettingsRegistry> Registry = BuiltFor;
	const TSubclassOf<UGantrySettingsProvider> Class = GetClass();
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Registry, Class](float)
	{
		if (UGantrySettingsRegistry* Live = Registry.Get())
		{
			Live->RemoveProvider(Class);
			Live->AddProvider(Class);
		}
		return false;
	}));
}

#undef LOCTEXT_NAMESPACE
