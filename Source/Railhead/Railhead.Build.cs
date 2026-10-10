// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;

/** Railhead's game module: the train, its crew and the world it crosses. */
public class Railhead : ModuleRules
{
	public Railhead(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"GantryCore",
			"GantryGauge",
			"GantryExperience"
		});
	}
}
