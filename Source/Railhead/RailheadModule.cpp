// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#include "RailheadTags.h"
#include "World/RailheadLineMessage.h"

#include "Messages/GantryChannelRegistry.h"
#include "Modules/ModuleManager.h"

/**
 * Railhead's module. Its one job beyond loading: declaring the channels the game offers, so they
 * are part of its API (the manifest lists them, and with Warn On Undeclared Channels on, a use
 * that does not match its declaration says so).
 */
class FRailheadModule : public FDefaultGameModuleImpl
{
public:

	virtual void StartupModule() override
	{
		FGantryChannelDeclaration Tunnel;
		Tunnel.Tag = RailheadTags::Message_Line_Tunnel;
		Tunnel.PayloadType = FRailheadLineMessage::StaticStruct();
		Tunnel.Description = TEXT("The train has gone into a tunnel; the stretch beyond it is loading. Sent on every machine that shows the line.");
		Declare(Tunnel);

		FGantryChannelDeclaration Daylight;
		Daylight.Tag = RailheadTags::Message_Line_Daylight;
		Daylight.PayloadType = FRailheadLineMessage::StaticStruct();
		Daylight.Description = TEXT("The train has come out of a tunnel into a new stretch. Sent on every machine that shows the line.");
		Declare(Daylight);

		FGantryChannelDeclaration Speed;
		Speed.Tag = RailheadTags::Hook_Line_Speed;
		Speed.Kind = EGantryChannelKind::Hook;
		Speed.PayloadType = FRailheadLineSpeed::StaticStruct();
		Speed.NetScope = EGantryChannelNetScope::ServerOnly;
		Speed.Description = TEXT("Every tick, the speed the train heads for. Change Target Speed to slow or hurry it.");
		Declare(Speed);
	}

	virtual void ShutdownModule() override
	{
		for (FGantryChannelDeclarationHandle& Handle : Declared)
		{
			FGantryChannelRegistry::Get().Undeclare(Handle);
		}
		Declared.Reset();
	}

private:

	void Declare(const FGantryChannelDeclaration& Declaration)
	{
		Declared.Add(FGantryChannelRegistry::Get().Declare(Declaration, TEXT("Railhead")));
	}

	/** What this module declared, taken back when it shuts down. */
	TArray<FGantryChannelDeclarationHandle> Declared;
};

IMPLEMENT_PRIMARY_GAME_MODULE(FRailheadModule, Railhead, "Railhead");
