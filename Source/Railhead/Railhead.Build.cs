// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;

/** Railhead's game module: the train, its crew and the world it crosses. */
public class Railhead : ModuleRules
{
	public Railhead(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// No Public/Private split: each header sits beside its source, in a folder named for its
		// role (World/, Train/, Player/, UI/...). The module root is the include root, so headers
		// are addressed by that folder, e.g. "World/RailheadLine.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"GameplayTags",
			"EnhancedInput",
			// Gantry: experiences (stretches are experiences), the load ledger the tunnels and
			// loading screen read, and the shared core types both of those use.
			"GantryCore",
			"GantryGauge",
			"GantryExperience",
			// Players' settings: pages, values, saving. Railhead draws them (UI/SRailheadSettings).
			"GantrySettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			// The crew's readiness component is a Gantry init-state component: the engine's interface.
			"ModularGameplay",
			// The HUD and the loading screen are Slate, written by hand: no widget assets.
			"Slate",
			"SlateCore"
		});
	}
}
