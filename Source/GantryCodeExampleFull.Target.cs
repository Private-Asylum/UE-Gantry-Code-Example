// Copyright Private Asylum LLC, 2026, All Rights Reserved.

using UnrealBuildTool;
using Microsoft.Extensions.Logging;

/**
 * The full game: standalone, listen server or client, in one executable. There is no plain
 * GantryCodeExample target on purpose; the targets are Full, Client, Server and Editor, and this
 * one owns the settings they share.
 */
public class GantryCodeExampleFullTarget : TargetRules
{
	public GantryCodeExampleFullTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ApplySharedGantryCodeExampleTargetSettings(this);

		ExtraModuleNames.Add("Railhead");
	}

	/**
	 * Settings shared by every GantryCodeExample target.
	 *
	 * Keeping these in one place means the Game, Client, Server and Editor targets can never
	 * drift apart on build settings, which is the usual source of "works in editor, breaks in
	 * a cooked build" surprises.
	 */
	internal static void ApplySharedGantryCodeExampleTargetSettings(TargetRules Target)
	{
		ILogger Logger = Target.Logger;

		Target.DefaultBuildSettings = BuildSettingsVersion.Latest;
		Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
		bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
		bool bIsDedicatedServer = Target.Type == TargetType.Server;

		if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
		{
			Logger.LogInformation("GantryCodeExample: applying Unique build environment settings.");

			// Shadowed variables are almost always a bug waiting to happen. Treat them as one.
			Target.CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;

			// Keep logging available in Shipping so field builds remain diagnosable.
			Target.bUseLoggingInShipping = true;

			if (bIsShipping && !bIsDedicatedServer)
			{
				Target.bDisableUnverifiedCertificates = true;
			}

			if (bIsShipping || bIsTest)
			{
				// Don't read generated / non-UFS ini files out of a cooked build.
				Target.bAllowGeneratedIniWhenCooked = false;
				Target.bAllowNonUFSIniWhenCooked = false;
			}

			if (Target.Type != TargetType.Editor)
			{
				// The path tracer is a beauty-shot tool only, and OpenImageDenoise is a very large library.
				Target.DisablePlugins.Add("OpenImageDenoise");
			}
		}
		else
		{
			// Shared build environment (the usual case on an installed engine). Nothing set here may
			// change how the shared engine binaries or PCHs were built; a setting that would needs
			// the target to use TargetBuildEnvironment.Unique.
		}
	}
}
