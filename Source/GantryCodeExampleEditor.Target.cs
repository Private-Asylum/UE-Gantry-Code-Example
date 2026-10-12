// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;

/** The editor, with the game module. */
public class GantryCodeExampleEditorTarget : TargetRules
{
	public GantryCodeExampleEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		GantryCodeExampleFullTarget.ApplySharedGantryCodeExampleTargetSettings(this);

		ExtraModuleNames.Add("Railhead");
	}
}
