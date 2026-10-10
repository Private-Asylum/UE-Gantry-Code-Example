// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;

/** A client only: joins a server, never hosts. Built from a source or full installed engine. */
public class GantryCodeExampleClientTarget : TargetRules
{
	public GantryCodeExampleClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;

		GantryCodeExampleFullTarget.ApplySharedGantryCodeExampleTargetSettings(this);

		ExtraModuleNames.Add("Railhead");
	}
}
