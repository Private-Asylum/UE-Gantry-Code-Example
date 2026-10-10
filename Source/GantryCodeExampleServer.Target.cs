// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;

/** A dedicated server: no rendering, no audio, no local players. */
public class GantryCodeExampleServerTarget : TargetRules
{
	public GantryCodeExampleServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		GantryCodeExampleFullTarget.ApplySharedGantryCodeExampleTargetSettings(this);

		ExtraModuleNames.Add("Railhead");
	}
}
